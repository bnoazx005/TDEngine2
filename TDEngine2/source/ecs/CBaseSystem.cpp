#include "../../include/ecs/CBaseSystem.h"


namespace TDEngine2
{
	void CBaseSystem::OnInit(TPtr<IJobManager> pJobManager)
	{
		mpJobManager = pJobManager;
	}

	E_RESULT_CODE CBaseSystem::AddDefferedCommand(const TCommandFunctor& action)
	{
		if (!action)
		{
			return RC_INVALID_ARGS;
		}

		mDefferedCommandsBuffer.push_back(action);

		return RC_OK;
	}

	void CBaseSystem::ExecuteDefferedCommands()
	{
		for (auto&& currCommand : mDefferedCommandsBuffer)
		{
			currCommand();
		}

		mDefferedCommandsBuffer.clear();
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
}