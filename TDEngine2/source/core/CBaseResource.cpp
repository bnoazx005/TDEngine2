#include "./../../include/core/CBaseResource.h"


namespace TDEngine2
{
	CBaseResource::CBaseResource() :
		CBaseObject(), mId(mInvalidResourceId), mState(RST_PENDING)
	{
	}

	TDE2_API E_RESULT_CODE CBaseResource::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		delete this;

		mIsInitialized = false;

		return RC_OK;
	}

	TDE2_API TResourceId CBaseResource::GetId() const
	{
		return mId;
	}

	TDE2_API std::string CBaseResource::GetName() const
	{
		return mName;
	}

	TDE2_API E_RESOURCE_STATE_TYPE CBaseResource::GetState() const
	{
		return mState;
	}

	TDE2_API E_RESULT_CODE CBaseResource::_init(IResourceManager* pResourceManager, const std::string& name, TResourceId id)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pResourceManager || name.empty() || id == InvalidResourceId)
		{
			return RC_INVALID_ARGS;
		}

		mpResourceManager = pResourceManager;

		mName = name;

		mId = id;

		mIsInitialized = true;

		return RC_OK;
	}

	const TResourceId CBaseResource::mInvalidResourceId { 0 };
}