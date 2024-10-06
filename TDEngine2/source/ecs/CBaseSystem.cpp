#include "../../include/ecs/CBaseSystem.h"
#include "../../include/editor/CPerfProfiler.h"
#include "../../include/core/IJobManager.h"
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

	class CAsyncSystemsGroup : public CBaseSystem
	{
		public:
			friend TDE2_API ISystem* CreateAsyncSystemsGroup(const std::vector<ISystem*>&, E_RESULT_CODE&);
		public:
			TDE2_SYSTEM(CAsyncSystemsGroup);

			E_RESULT_CODE Init(const std::vector<ISystem*>& systems);
			void InjectBindings(IWorld* pWorld) override;

			void Update(IWorld* pWorld, F32 dt) override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CAsyncSystemsGroup)

			E_RESULT_CODE _onFreeInternal() override;
		protected:
			mutable std::mutex    mMutex;
			std::vector<ISystem*> mpGroupedSystems;
	};

	CAsyncSystemsGroup::CAsyncSystemsGroup() :
		CBaseSystem()
	{
	}

	E_RESULT_CODE CAsyncSystemsGroup::Init(const std::vector<ISystem*>& systems)
	{
		std::copy(systems.cbegin(), systems.cend(), std::back_inserter(mpGroupedSystems));

		mIsInitialized = true;

		return RC_OK;
	}

	void CAsyncSystemsGroup::InjectBindings(IWorld* pWorld)
	{
		TDE2_PROFILER_SCOPE("CAsyncSystemsGroup::InjectBindings");

		for (ISystem* pCurrSystem : mpGroupedSystems)
		{
			pCurrSystem->InjectBindings(pWorld);
		}
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
					pCurrSystem->Update(pWorld, dt);
				}

				/// \note Execute all deferred commands after all updates
				for (ISystem* pCurrSystem : mpGroupedSystems)
				{
					pCurrSystem->ExecuteDeferredCommands();
				}
			});
	}

	E_RESULT_CODE CAsyncSystemsGroup::_onFreeInternal()
	{
		std::lock_guard<std::mutex> lock(mMutex);

		for (ISystem* pCurrSystem : mpGroupedSystems)
		{
			pCurrSystem->Free();
		}

		mpGroupedSystems.clear();

		return RC_OK;
	}


	TDE2_API ISystem* CreateAsyncSystemsGroup(const std::vector<ISystem*>& systems, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem ,CAsyncSystemsGroup, result, systems);
	}
}