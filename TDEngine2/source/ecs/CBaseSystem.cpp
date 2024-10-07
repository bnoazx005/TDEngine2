#include "../../include/ecs/CBaseSystem.h"
#include "../../include/ecs/CWorld.h"
#include "../../include/editor/CPerfProfiler.h"
#include "../../include/core/IJobManager.h"
#include "../../include/core/Event.h"
#include "../../include/core/IEventManager.h"
#include "../../include/editor/CPerfProfiler.h"


namespace TDEngine2
{
	void CBaseSystem::OnInit(TPtr<IJobManager> pJobManager)
	{
		mpJobManager = pJobManager;
	}

	E_RESULT_CODE CBaseSystem::AddDeferredCommand(const TCommandFunctor& action)
	{
		if (!action)
		{
			return RC_INVALID_ARGS;
		}

		mDeferredCommandsBuffer.push_back(action);

		return RC_OK;
	}

	void CBaseSystem::ExecuteDeferredCommands()
	{
		TDE2_PROFILER_SCOPE("CBaseSystem::ExecuteDeferredCommands");

		for (auto&& currCommand : mDeferredCommandsBuffer)
		{
			currCommand();
		}

		mDeferredCommandsBuffer.clear();
	}

#if TDE2_EDITORS_ENABLED
	
	void CBaseSystem::DebugOutput(IDebugUtility* pDebugUtility, F32 dt) const
	{
	}

#endif

	void CBaseSystem::OnDestroy()
	{
	}

	void CBaseSystem::OnActivated()
	{
		mIsActive = true;
	}

	void CBaseSystem::OnDeactivated()
	{
		mIsActive = false;
	}

	bool CBaseSystem::IsActive() const
	{
		return mIsActive;
	}


	/*!
		\brief CAsyncSystemsGroup's definition
	*/

	class CAsyncSystemsGroup : public CBaseSystem, public IEventHandler
	{
		public:
			friend TDE2_API ISystem* CreateAsyncSystemsGroup(IEventManager*, const std::vector<ISystem*>&, E_RESULT_CODE&);
		public:
			TDE2_SYSTEM(CAsyncSystemsGroup);

			E_RESULT_CODE Init(IEventManager* pEventManager, const std::vector<ISystem*>& systems);
			void InjectBindings(IWorld* pWorld) override;

			void Update(IWorld* pWorld, F32 dt) override;

			E_RESULT_CODE OnEvent(const TBaseEvent* pEvent) override;
			TEventListenerId GetListenerId() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CAsyncSystemsGroup)

			E_RESULT_CODE _onFreeInternal() override;
		protected:
			mutable std::mutex    mMutex;
			IEventManager*        mpEventManager = nullptr;
			std::vector<ISystem*> mpGroupedSystems;

			std::atomic_bool      mIsDirty = true;
	};

	CAsyncSystemsGroup::CAsyncSystemsGroup() :
		CBaseSystem()
	{
	}

	E_RESULT_CODE CAsyncSystemsGroup::Init(IEventManager* pEventManager, const std::vector<ISystem*>& systems)
	{
		if (!pEventManager)
		{
			return RC_INVALID_ARGS;
		}

		mpEventManager = pEventManager;

		/// subscribe the manager onto events of ECS
		mpEventManager->Subscribe(TOnEntityCreatedEvent::GetTypeId(), this);
		mpEventManager->Subscribe(TOnEntityRemovedEvent::GetTypeId(), this);
		mpEventManager->Subscribe(TOnComponentCreatedEvent::GetTypeId(), this);
		mpEventManager->Subscribe(TOnComponentRemovedEvent::GetTypeId(), this);
		mpEventManager->Subscribe(TOnHierarchyChangedEvent::GetTypeId(), this);

		std::copy(systems.cbegin(), systems.cend(), std::back_inserter(mpGroupedSystems));

		mIsInitialized = true;

		return RC_OK;
	}

	void CAsyncSystemsGroup::InjectBindings(IWorld* pWorld)
	{
	}

	void CAsyncSystemsGroup::Update(IWorld* pWorld, F32 dt)
	{
		TDE2_PROFILER_SCOPE("CAsyncSystemsGroup::Update");

		mpJobManager->SubmitJob(nullptr, [this, pWorld, dt](auto)
			{
				TDE2_PROFILER_SCOPE("CBaseSystem::ExecuteDeferredCommands");
				std::lock_guard<std::mutex> lock(mMutex);

				for (ISystem* pCurrSystem : mpGroupedSystems)
				{
					if (mIsDirty)
					{
						pCurrSystem->InjectBindings(pWorld);
					}

					pCurrSystem->Update(pWorld, dt);
				}

				mIsDirty = false;

				/// \note Execute all deferred commands after all updates
				for (ISystem* pCurrSystem : mpGroupedSystems)
				{
					pCurrSystem->ExecuteDeferredCommands();
				}
			});
	}

	E_RESULT_CODE CAsyncSystemsGroup::OnEvent(const TBaseEvent* pEvent)
	{
		mIsDirty = true;
		return RC_OK;
	}

	TEventListenerId CAsyncSystemsGroup::GetListenerId() const
	{
		return static_cast<TEventListenerId>(GetTypeId());
	}

	E_RESULT_CODE CAsyncSystemsGroup::_onFreeInternal()
	{
		std::lock_guard<std::mutex> lock(mMutex);

		mpEventManager->Unsubscribe(TOnEntityCreatedEvent::GetTypeId(), this);
		mpEventManager->Unsubscribe(TOnEntityRemovedEvent::GetTypeId(), this);
		mpEventManager->Unsubscribe(TOnComponentCreatedEvent::GetTypeId(), this);
		mpEventManager->Unsubscribe(TOnComponentRemovedEvent::GetTypeId(), this);
		mpEventManager->Unsubscribe(TOnHierarchyChangedEvent::GetTypeId(), this);

		for (ISystem* pCurrSystem : mpGroupedSystems)
		{
			pCurrSystem->Free();
		}

		mpGroupedSystems.clear();

		return RC_OK;
	}


	TDE2_API ISystem* CreateAsyncSystemsGroup(IEventManager* pEventManager, const std::vector<ISystem*>& systems, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem ,CAsyncSystemsGroup, result, pEventManager, systems);
	}
}