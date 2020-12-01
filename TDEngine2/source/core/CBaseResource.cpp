#include "./../../include/core/CBaseResource.h"


namespace TDEngine2
{
	CBaseResource::CBaseResource() :
		CBaseObject(), mId(TResourceId::Invalid), mState(E_RESOURCE_STATE_TYPE::RST_PENDING)
	{
	}

	E_RESULT_CODE CBaseResource::Load()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		const IResourceLoader* pResourceLoader = _getResourceLoader();

		if (!pResourceLoader)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = pResourceLoader->LoadResource(this);

		if (result != RC_OK)
		{
			mState = E_RESOURCE_STATE_TYPE::RST_PENDING;

			return result;
		}

		mState = E_RESOURCE_STATE_TYPE::RST_LOADED;

		return result;
	}

	E_RESULT_CODE CBaseResource::Unload()
	{
		return Reset();
	}

	TDE2_API E_RESULT_CODE CBaseResource::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = Unload();

		mIsInitialized = false;
		delete this;

		return result;
	}

	TDE2_API TResourceId CBaseResource::GetId() const
	{
		return mId;
	}

	TDE2_API const std::string& CBaseResource::GetName() const
	{
		return mName;
	}

	TDE2_API E_RESOURCE_STATE_TYPE CBaseResource::GetState() const
	{
		return mState;
	}

	TDE2_API E_RESULT_CODE CBaseResource::_init(IResourceManager* pResourceManager, const std::string& name)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pResourceManager || name.empty())
		{
			return RC_INVALID_ARGS;
		}

		mpResourceManager = pResourceManager;

		mName = name;

		mId = mpResourceManager->GetResourceId(mName); /// \todo Should a resource store mId at all?

		mIsInitialized = true;

		return RC_OK;
	}
}