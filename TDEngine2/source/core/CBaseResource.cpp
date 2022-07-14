#include "./../../include/core/CBaseResource.h"
#include "../../include/core/IResourceFactory.h"


namespace TDEngine2
{
	/*!
		\brief TBaseResourceParameters's definition
	*/

	E_RESULT_CODE TBaseResourceParameters::Load(IArchiveReader* pReader)
	{
		return RC_NOT_IMPLEMENTED_YET;
	}

	E_RESULT_CODE TBaseResourceParameters::Save(IArchiveWriter* pWriter)
	{
		return RC_NOT_IMPLEMENTED_YET;
	}


	/*!
		\brief CBaseResource's definition 
	*/


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

		auto pResourceLoader = _getResourceLoader();

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

		switch (mLoadingPolicy)
		{
			case E_RESOURCE_LOADING_POLICY::SYNCED:
				mState = E_RESOURCE_STATE_TYPE::RST_LOADED;
				break;
			case E_RESOURCE_LOADING_POLICY::STREAMING:
				mState = E_RESOURCE_STATE_TYPE::RST_LOADING;
				break;
		}

		return result;
	}

	E_RESULT_CODE CBaseResource::Unload()
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return mpResourceManager->ReleaseResource(mId);
	}

	E_RESULT_CODE CBaseResource::_onFreeInternal()
	{
		return Reset();
	}

	void CBaseResource::SetState(E_RESOURCE_STATE_TYPE state)
	{
		mState = state;
	}

	void CBaseResource::OnCreated(IResourceManager* pResourceManager)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		mId = pResourceManager->GetResourceId(mName); // \note Update mId, because it's probably Invalid for now
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
		return mState;
	}

	void CBaseResource::SetLoadingPolicy(E_RESOURCE_LOADING_POLICY policy)
	{
		mLoadingPolicy = policy;
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