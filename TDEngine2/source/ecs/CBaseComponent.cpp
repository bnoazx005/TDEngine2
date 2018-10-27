#include "./../../include/ecs/CBaseComponent.h"


namespace TDEngine2
{
	CBaseComponent::CBaseComponent() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CBaseComponent::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CBaseComponent::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}
}