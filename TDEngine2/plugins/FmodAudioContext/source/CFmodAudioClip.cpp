#include "../include/CFmodAudioClip.h"
#include <core/IAudioContext.h>


namespace TDEngine2
{
	CFMODAudioClip::CFMODAudioClip():
		CBaseResource()
	{
	}

	E_RESULT_CODE CFMODAudioClip::Init(IResourceManager* pResourceManager, IAudioContext* pAudioContext, const std::string& name)
	{
		E_RESULT_CODE result = _init(pResourceManager, name);

		if (result != RC_OK)
		{
			return result;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CFMODAudioClip::Reset()
	{
		return RC_OK;
	}

	const IResourceLoader* CFMODAudioClip::_getResourceLoader()
	{
		return mpResourceManager->GetResourceLoader<CFMODAudioClip>();
	}


	IAudioSource* CreateFMODAudioClip(IResourceManager* pResourceManager, IAudioContext* pAudioContext, const std::string& name, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IAudioSource, CFMODAudioClip, result, pResourceManager, pAudioContext, name);
	}


	/*!
		\brief CFMODAudioClipLoader's definition
	*/

	CFMODAudioClipLoader::CFMODAudioClipLoader() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CFMODAudioClipLoader::Init(IResourceManager* pResourceManager, IFileSystem* pFileSystem, IAudioContext* pAudioContext)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pResourceManager || !pAudioContext || !pFileSystem)
		{
			return RC_INVALID_ARGS;
		}

		mpResourceManager = pResourceManager;
		mpFileSystem = pFileSystem;
		mpAudioContext = pAudioContext;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CFMODAudioClipLoader::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	E_RESULT_CODE CFMODAudioClipLoader::LoadResource(IResource* pResource) const
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		TDE2_UNIMPLEMENTED();

		return result;
	}

	TypeId CFMODAudioClipLoader::GetResourceTypeId() const
	{
		return CFMODAudioClip::GetTypeId();
	}


	TDE2_API IResourceLoader* CreateFMODAudioClipLoader(IResourceManager* pResourceManager, IAudioContext* pAudioContext, IFileSystem* pFileSystem, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IResourceLoader, CFMODAudioClipLoader, result, pResourceManager, pFileSystem, pAudioContext);
	}


	/*!
		\brief CFMODAudioClipFactory's definiton
	*/

	CFMODAudioClipFactory::CFMODAudioClipFactory() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CFMODAudioClipFactory::Init(IResourceManager* pResourceManager, IAudioContext* pAudioContext)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pAudioContext || !pResourceManager)
		{
			return RC_INVALID_ARGS;
		}

		mpResourceManager = pResourceManager;
		mpAudioContext = pAudioContext;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CFMODAudioClipFactory::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	IResource* CFMODAudioClipFactory::Create(const std::string& name, const TBaseResourceParameters& params) const
	{
		return CreateDefault(name, params);
	}

	IResource* CFMODAudioClipFactory::CreateDefault(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;
		return dynamic_cast<IResource*>(CreateFMODAudioClip(mpResourceManager, mpAudioContext, name, result));
	}

	TypeId CFMODAudioClipFactory::GetResourceTypeId() const
	{
		return CFMODAudioClip::GetTypeId();
	}


	TDE2_API IResourceFactory* CreateFMODAudioClipFactory(IResourceManager* pResourceManager, IAudioContext* pAudioContext, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IResourceFactory, CFMODAudioClipFactory, result, pResourceManager, pAudioContext);
	}
}