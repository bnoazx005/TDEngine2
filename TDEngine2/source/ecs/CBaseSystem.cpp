#include "../../include/ecs/CBaseSystem.h"


namespace TDEngine2
{
	void CBaseSystem::OnInit()
	{
	}

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