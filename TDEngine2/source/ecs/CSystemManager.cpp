#include "../../include/ecs/CSystemManager.h"
#include "../../include/ecs/ISystem.h"
#include "../../include/ecs/IWorld.h"
#include "../../include/ecs/CTransformSystem.h"
#include "../../include/ecs/CSpriteRendererSystem.h"
#include "../../include/utils/Utils.h"
#include "../../include/core/IEventManager.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/ecs/CBaseComponent.h"
#include "../../include/editor/CPerfProfiler.h"
#include "../../include/core/CGameUserSettings.h"
#include <algorithm>
#include <unordered_set>
#if TDE2_EDITORS_ENABLED
	#include "../../include/platform/CBaseFile.h"
	#include "../../include/platform/IOStreams.h"
	#include "../../include/platform/CTextFileWriter.h"
#endif


namespace TDEngine2
{
	CInt32ConsoleVarDecl TriggerDumpSystemsDepsGraph("ecs.dump_systems_graph", "", 0); // \todo For now triggers are not supported so mimic them using usual cvars


#if TDE2_EDITORS_ENABLED
	static inline void DumpSystemsDepsGraph(const CSystemManager::TSystemsArray& systems)
	{
		E_RESULT_CODE result = RC_OK;

		auto pFileStream = TPtr<TDEngine2::IStream>(CreateFileOutputStream("SystemsDependenciesGraphDump.dot", result));
		ITextFileWriter* pFileWriter = dynamic_cast<ITextFileWriter*>(CreateTextFileWriter(nullptr, pFileStream, result));

		// \note Writes file in DOT format
		pFileWriter->WriteLine("digraph SystemsGraph {");
		pFileWriter->WriteLine("rankdir = LR\nsplines=spline\nnode [shape=rectangle, fontname=\"helvetica\", fontsize=12]\n");

		// \note Write passes nodes
		for (auto&& currSystemDesc : systems)
		{
			pFileWriter->WriteLine(Wrench::StringUtils::Format("\"{0}\" [label=\"{0}\", style=\"rounded, filled\", fillcolor=darkorange]", currSystemDesc.mpSystem->GetName()));
		}

		pFileWriter->WriteLine(Wrench::StringUtils::GetEmptyStr());

		// \note Create edges between systems
		for (auto&& currSystemDesc : systems)
		{
			if (currSystemDesc.mpSystem->GetContinuations().empty())
			{
				continue;
			}

			pFileWriter->WriteLine(Wrench::StringUtils::Format("\"{0}\" -> {", currSystemDesc.mpSystem->GetName()));

			for (TPtr<ISystem> continuation: currSystemDesc.mpSystem->GetContinuations())
			{
				pFileWriter->WriteLine("\"" + continuation->GetName() + "\" ");
			}

			pFileWriter->WriteLine("} [color = seagreen]");
		}

		pFileWriter->WriteLine("}");

		pFileWriter->Close();

		TriggerDumpSystemsDepsGraph.Set(0);
	}

#endif


	TSystemAutoInitializer::~TSystemAutoInitializer()
	{
	}

	E_RESULT_CODE TSystemAutoInitializer::ManageDependencies(IWorld* pWorld)
	{
		return RC_OK;
	}



	CSystemManager::TSystemsInitializersArray CSystemManager::mSystemsInitializersRegistry{};


	CSystemManager::CSystemManager() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CSystemManager::Init(IWorld* pWorld, IEventManager* pEventManager, TPtr<IJobManager> pJobManager)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pEventManager || !pWorld || !pJobManager)
		{
			return RC_INVALID_ARGS;
		}
		
		mpWorld = pWorld;

		mpEventManager = pEventManager;
		mpJobManager = pJobManager;

		/// subscribe the manager onto events of ECS
		mpEventManager->Subscribe(TOnEntityCreatedEvent::GetTypeId(), this);
		mpEventManager->Subscribe(TOnEntityRemovedEvent::GetTypeId(), this);
		mpEventManager->Subscribe(TOnComponentCreatedEvent::GetTypeId(), this);
		mpEventManager->Subscribe(TOnComponentRemovedEvent::GetTypeId(), this);
		mpEventManager->Subscribe(TOnHierarchyChangedEvent::GetTypeId(), this);
		
		mIsDirty = true;

		mIsInitialized = true;
		
		return RC_OK;
	}

	E_RESULT_CODE CSystemManager::_onFreeInternal()
	{
		return ClearSystemsRegistry();
	}

	TResult<TSystemId> CSystemManager::RegisterSystem(TPtr<ISystem> pSystem, E_SYSTEM_PRIORITY priority)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (!pSystem)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}

		/// inject bindings for the first time when the system is registered
		pSystem->OnInit(mpJobManager);
		pSystem->InjectBindings(mpWorld);

		auto duplicateIter = std::find_if(mpActiveSystems.begin(), mpActiveSystems.end(), [&pSystem](const TSystemDesc& sysDesc)
		{
			return sysDesc.mpSystem == pSystem;
		});

		if (duplicateIter != mpActiveSystems.end()) /// if there is a duplicate, just interrupt registration process
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		U32 lastUsedSystemId = mSystemsIdentifiersTable[priority];

		++mSystemsIdentifiersTable[priority];

		/// low bytes contains id, high bytes - priority
		TSystemId internalSystemPriority = TSystemId(static_cast<U32>(priority) << 16 | lastUsedSystemId);

		mpActiveSystems.push_back({ internalSystemPriority, pSystem });
		
		if (auto pRenderSystem = DynamicPtrCast<IRenderSystem>(pSystem))
		{
			mpRenderSystems.emplace_back(pRenderSystem);
		}

		return Wrench::TOkValue<TSystemId>(TSystemId(internalSystemPriority));
	}
	
	E_RESULT_CODE CSystemManager::UnregisterSystem(TSystemId systemId)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		return _internalUnregisterSystem(systemId);
	}

	E_RESULT_CODE CSystemManager::RegisterPendingSystems(IWorld* pWorld, const TRequestSubsystemCallback& subsystemsProviderCallback)
	{
		TDE2_PROFILER_SCOPE("CSystemManager::RegisterPendingSystems");

		if (!pWorld)
		{
			return RC_INVALID_ARGS;
		}

		E_RESULT_CODE result = RC_OK;

		for (auto&& pSystemRegistrar : mSystemsInitializersRegistry)
		{
			TResult<TSystemId> registerSystemResult = RegisterSystem(TPtr<ISystem>(pSystemRegistrar->GetSystem(pWorld, subsystemsProviderCallback)));
			if (registerSystemResult.HasError())
			{
				result = result | registerSystemResult.GetError();
			}
		}

		for (auto&& pSystemRegistrar : mSystemsInitializersRegistry)
		{
			result = result | pSystemRegistrar->ManageDependencies(pWorld);
		}

		mSystemsInitializersRegistry.clear();

		return result;
	}

	E_RESULT_CODE CSystemManager::ActivateSystem(TSystemId systemId)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		auto targetSystemIter = _findSystemDesc(mpDeactivatedSystems.begin(), mpDeactivatedSystems.end(), systemId);

		if (targetSystemIter == mpDeactivatedSystems.end())
		{
			return RC_FAIL;
		}

		if (TPtr<ISystem> pSystem = targetSystemIter->mpSystem)
		{
			pSystem->OnActivated();
			pSystem->InjectBindings(mpWorld);
		}

		mpActiveSystems.emplace_back(*targetSystemIter);

		if (auto pRenderSystem = DynamicPtrCast<IRenderSystem>(targetSystemIter->mpSystem))
		{
			mpRenderSystems.emplace_back(pRenderSystem);
		}

		mpDeactivatedSystems.erase(targetSystemIter);

		return RC_OK;
	}

	E_RESULT_CODE CSystemManager::DeactivateSystem(TSystemId systemId)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		auto targetSystemIter = _findSystemDesc(mpActiveSystems.begin(), mpActiveSystems.end(), systemId);

		if (targetSystemIter == mpActiveSystems.end())
		{
			return RC_FAIL;
		}

		if (TPtr<ISystem> pSystem = targetSystemIter->mpSystem)
		{
			pSystem->OnDeactivated();
		}

		if (auto pRenderSystem = DynamicPtrCast<IRenderSystem>(targetSystemIter->mpSystem))
		{
			mpRenderSystems.erase(std::find(mpRenderSystems.cbegin(), mpRenderSystems.cend(), pRenderSystem));
		}

		mpDeactivatedSystems.emplace_back(*targetSystemIter);
		mpActiveSystems.erase(targetSystemIter);

		return RC_OK;
	}

	E_RESULT_CODE CSystemManager::InitSystems()
	{
		std::lock_guard<std::mutex> lock(mMutex);

		TPtr<ISystem> pCurrSystem = nullptr;

		for (auto currSystemDesc : mpActiveSystems)
		{
			pCurrSystem = currSystemDesc.mpSystem;

			pCurrSystem->OnInit(mpJobManager);
			pCurrSystem->OnActivated();
		}

		return RC_OK;
	}

	E_RESULT_CODE CSystemManager::ClearSystemsRegistry()
	{
		E_RESULT_CODE result = DestroySystems();

		std::lock_guard<std::mutex> lock(mMutex);

		mpActiveSystems.clear();
		mpDeactivatedSystems.clear();

		return result;
	}

	void CSystemManager::Update(IWorld* pWorld, float dt)
	{
		TDE2_PROFILER_SCOPE("CSystemManager::Update");
		std::lock_guard<std::mutex> lock(mMutex);

#if TDE2_EDITORS_ENABLED
		if (TriggerDumpSystemsDepsGraph.Get())
		{
			DumpSystemsDepsGraph(mpActiveSystems);
		}
#endif

#if TDE2_DEBUG_MODE
		mIsUpdateSystemsStageExecuted.store(true);
#endif

		std::unordered_map<TypeId, U32> systemsAncestors{};

		for (const TSystemDesc& currSystemEntry : mpActiveSystems)
		{
			systemsAncestors[currSystemEntry.mpSystem->GetSystemType()] = 0;
		}

		for (const TSystemDesc& currSystemEntry : mpActiveSystems)
		{
			for (TPtr<ISystem> pCurrSystem : currSystemEntry.mpSystem->GetContinuations())
			{
				++systemsAncestors[pCurrSystem->GetSystemType()];
			}
		}

		std::unordered_set<TPtr<ISystem>> remainingSystems{};
		std::transform(mpActiveSystems.cbegin(), mpActiveSystems.cend(), 
			std::inserter(remainingSystems, remainingSystems.end()), 
			[](const TSystemDesc& currSystemEntry) { return currSystemEntry.mpSystem; });

#if TDE2_DEBUG_MODE
		U32 currIterationsCount = 0;
#endif

		while (!remainingSystems.empty())
		{
			CFixedVector<TPtr<ISystem>, 64> executionGroup{};

#if TDE2_DEBUG_MODE
			TDE2_ASSERT_MSG(currIterationsCount++ <= MAX_SYSTEMS_PER_FRAME, "[CSystemManager] Maximum iterations count exceeded. Possibly circlular dependencies between systems exists");
#endif

			for (TPtr<ISystem> pSystem : remainingSystems)
			{
				if (systemsAncestors[pSystem->GetSystemType()])
				{
					continue;
				}

				executionGroup.push_back(pSystem);
			}

			// Execute all systems that have no dependencies
			{
				{
					TDE2_PROFILER_SCOPE("InjectBindings");

					for (TPtr<ISystem> pSystem : executionGroup)
					{
						if (mIsDirty)
						{
							pSystem->InjectBindings(mpWorld);
						}
					}
				}

				TJobCounter executionCounterGroup{};

				mpJobManager->SubmitMultipleJobs(&executionCounterGroup, static_cast<U32>(executionGroup.size()), 1, [this, &executionGroup, pWorld, dt](const TJobArgs& args)
					{
						executionGroup[args.mJobIndex]->Update(pWorld, dt);
					});

				mpJobManager->WaitForJobCounter(executionCounterGroup);
			}

			for (TPtr<ISystem> pSystem : executionGroup)
			{
				remainingSystems.erase(pSystem);
			}

			for (TPtr<ISystem> pSystem : executionGroup)
			{
				for (TPtr<ISystem> pSubsystem : pSystem->GetContinuations())
				{
					--systemsAncestors[pSubsystem->GetSystemType()];
				}
			}
		}

#if TDE2_DEBUG_MODE
		mIsUpdateSystemsStageExecuted.store(false);
#endif

		mIsDirty = false;

		/// \note Execute all deferred commands after all updates
		for (auto currSystemDesc : mpActiveSystems)
		{
			currSystemDesc.mpSystem->ExecuteDeferredCommands();
		}
	}

	void CSystemManager::SyncSystemsExecution()
	{
		TDE2_PROFILER_SCOPE("CSystemManager::SyncSystemsExecution");
		std::lock_guard<std::mutex> lock(mMutex);

		for (auto currSystemDesc : mpActiveSystems)
		{
			TPtr<ISystem> pCurrSystem = currSystemDesc.mpSystem;
			pCurrSystem->OnSyncRequested();
		}
	}

	void CSystemManager::FillFramePacket(TFramePacket& framePacket)
	{
		TDE2_PROFILER_SCOPE("CSystemManager::FillFramePacket");
		std::lock_guard<std::mutex> lock(mMutex);

		for (TPtr<IRenderSystem> pRenderSystem : mpRenderSystems)
		{
			pRenderSystem->FillFramePacket(framePacket);
		}
	}

	E_RESULT_CODE CSystemManager::DestroySystems()
	{
		std::lock_guard<std::mutex> lock(mMutex);

		auto&& preDestroySystem = [this](auto&& container)
		{
			TPtr<ISystem> pCurrSystem = nullptr;

			for (auto currSystemDesc : container)
			{
				pCurrSystem = currSystemDesc.mpSystem;

				pCurrSystem->OnDeactivated();
				pCurrSystem->OnDestroy();
			}
		};

		preDestroySystem(mpActiveSystems);
		preDestroySystem(mpDeactivatedSystems);

		return RC_OK;
	}

	E_RESULT_CODE CSystemManager::OnEvent(const TBaseEvent& event)
	{
		mIsDirty = true;
		return RC_OK;
	}

	TEventListenerId CSystemManager::GetListenerId() const
	{
		return TEventListenerId(GetTypeId());
	}

	void CSystemManager::ForEachSystem(const std::function<void(TSystemId systemId, const ISystem* const pSystem)> action) const
	{
		TDE2_ASSERT(action);
		if (!action)
		{
			return;
		}

		for (auto&& currSystem : mpActiveSystems)
		{
			action(currSystem.mSystemId, currSystem.mpSystem.Get());
		}

		for (auto&& currSystem : mpDeactivatedSystems)
		{
			action(currSystem.mSystemId, currSystem.mpSystem.Get());
		}
	}


	E_RESULT_CODE CSystemManager::RegisterSystemInitializer(std::unique_ptr<TSystemAutoInitializer> registrar)
	{
		if (!registrar)
		{
			return RC_INVALID_ARGS;
		}

		mSystemsInitializersRegistry.emplace_back(std::move(registrar));

		return RC_OK;
	}


	bool CSystemManager::IsSystemActive(TSystemId systemId) const
	{
		return std::find_if(mpActiveSystems.cbegin(), mpActiveSystems.cend(), [systemId](auto&& systemInfoEntity) { return systemId == systemInfoEntity.mSystemId; }) != mpActiveSystems.cend();
	}


#if TDE2_DEBUG_MODE

	bool CSystemManager::IsUpdateSystemsStageExecuted() const
	{
		return mIsUpdateSystemsStageExecuted;
	}

#endif // TDE2_DEBUG_MODE

	E_RESULT_CODE CSystemManager::_internalUnregisterSystem(TSystemId systemId)
	{
		if (systemId == TSystemId::Invalid)
		{
			return RC_INVALID_ARGS;
		}

		auto targetSystemIter = _findSystemDesc(mpActiveSystems.begin(), mpActiveSystems.end(), systemId);

		if (targetSystemIter == mpActiveSystems.end()) /// specified system is not registred yet
		{
			return RC_FAIL;
		}

		if (auto pRenderSystem = DynamicPtrCast<IRenderSystem>(targetSystemIter->mpSystem))
		{
			mpRenderSystems.erase(std::find(mpRenderSystems.cbegin(), mpRenderSystems.cend(), pRenderSystem));
		}

		mpActiveSystems.erase(targetSystemIter);

		return RC_OK;
	}

	TSystemId CSystemManager::FindSystem(TypeId typeId)
	{
		auto iter = std::find_if(mpActiveSystems.cbegin(), mpActiveSystems.cend(), [typeId](const TSystemDesc& systemDesc)
		{
			return systemDesc.mpSystem->GetSystemType() == typeId;
		});

		return (iter == mpActiveSystems.cend()) ? TSystemId::Invalid : iter->mSystemId;
	}


	TPtr<ISystem> CSystemManager::GetSystem(TSystemId handle)
	{
		return _findSystemDesc(mpActiveSystems.begin(), mpActiveSystems.end(), handle)->mpSystem;
	}


	ISystemManager* CreateSystemManager(IWorld* pWorld, IEventManager* pEventManager, TPtr<IJobManager> pJobManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystemManager, CSystemManager, result, pWorld, pEventManager, pJobManager);
	}
}