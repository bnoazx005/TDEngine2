#include "./../../include/core/CBaseResource.h"


namespace TDEngine2
{
	CBaseResource::CBaseResource() :
		CBaseObject(), mId(TResourceId::Invalid), mState(E_RESOURCE_STATE_TYPE::RST_PENDING), mLoadingPolicy(E_RESOURCE_LOADING_POLICY::SYNCED)
	{
	}

	E_RESULT_CODE CBaseResource::Load()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		std::lock_guard<std::mutex> lock(mMutex);

		const IResourceLoader* pResourceLoader = _getResourceLoader();

		if (!pResourceLoader)
		{
			return RC_FAIL;
		}

		mId = mpResourceManager->GetResourceId(mName); /// \todo Should a resource store mId at all?

		mState = E_RESOURCE_STATE_TYPE::RST_PENDING;

		E_RESULT_CODE result = pResourceLoader->LoadResource(this);

		if (result != RC_OK)
		{
			mState = E_RESOURCE_STATE_TYPE::RST_PENDING;

			return result;
		}

		if (E_RESOURCE_LOADING_POLICY::SYNCED == mLoadingPolicy)
		{
			mState = E_RESOURCE_STATE_TYPE::RST_LOADED;
		}

		return result;
	}

	E_RESULT_CODE CBaseResource::Unload()
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return Reset();
	}

	TDE2_API E_RESULT_CODE CBaseResource::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		--mRefCounter;

		E_RESULT_CODE result = RC_OK;

		if (!mRefCounter)
		{
			result = Unload();

			mIsInitialized = false;
			delete this;
		}

		return result;
	}

	void CBaseResource::SetState(E_RESOURCE_STATE_TYPE state)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		mState = state;
	}

	TResourceId CBaseResource::GetId() const
	{
		return mId;
	}

	const std::string& CBaseResource::GetName() const
	{
		return mName;
	}

	E_RESOURCE_STATE_TYPE CBaseResource::GetState() const
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return mState;
	}

	E_RESOURCE_LOADING_POLICY CBaseResource::GetLoadingPolicy() const
	{
		//std::lock_guard<std::mutex> lock(mMutex);
		return mLoadingPolicy;
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

		mState = E_RESOURCE_STATE_TYPE::RST_LOADED;

		mIsInitialized = true;

		return RC_OK;
	}
}