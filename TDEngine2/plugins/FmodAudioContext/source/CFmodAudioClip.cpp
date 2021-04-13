#include "../include/CFmodAudioClip.h"
#include "../include/CFmodAudioContext.h"
#include <fmod.hpp>
#include <fmod_studio.hpp>
#include <fmod_errors.h>
#include <fmod_common.h>


namespace TDEngine2
{
	CFMODAudioClip::CFMODAudioClip():
		CBaseResource()
	{
	}

	E_RESULT_CODE CFMODAudioClip::Init(IResourceManager* pResourceManager, IAudioContext* pAudioContext, const std::string& name)
	{
		E_RESULT_CODE result = _init(pResourceManager, name);

		if (RC_OK != result)
		{
			return result;
		}

		mpAudioContext = pAudioContext;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CFMODAudioClip::Reset()
	{
		return ResetInternalSoundHandle(nullptr);
	}

	E_RESULT_CODE CFMODAudioClip::ResetInternalSoundHandle(FMOD::Sound* pSoundHandle)
	{
		if (!pSoundHandle)
		{
			return RC_INVALID_ARGS;
		}

		if (mpSoundHandle)
		{
			mpSoundHandle->release();
		}

		mpSoundHandle = pSoundHandle;
		
		return RC_OK;
	}

	E_RESULT_CODE CFMODAudioClip::Play()
	{
		return mpAudioContext->Play(this);
	}

	FMOD::Sound* CFMODAudioClip::GetInternalHandle() const
	{
		return mpSoundHandle;
	}

	const IResourceLoader* CFMODAudioClip::_getResourceLoader()
	{
		return mpResourceManager->GetResourceLoader<IAudioSource>();
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

		CFMODAudioClip* pAudioClip = dynamic_cast<CFMODAudioClip*>(pResource);
		if (!pAudioClip)
		{
			return RC_INVALID_ARGS;
		}

		CFMODAudioContext* pAudioContext = dynamic_cast<CFMODAudioContext*>(mpAudioContext);

		FMOD::Sound* pSound = nullptr;

		FMOD_RESULT result = pAudioContext->GetInternalContext()->createSound(pAudioClip->GetName().c_str(), FMOD_NONBLOCKING, nullptr, &pSound);
		if (FMOD_OK != result)
		{
			return RC_FAIL;
		}

		pSound->setMode(FMOD_LOOP_OFF);

		return pAudioClip->ResetInternalSoundHandle(pSound);
	}

	TypeId CFMODAudioClipLoader::GetResourceTypeId() const
	{
		return IAudioSource::GetTypeId();
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
		return IAudioSource::GetTypeId();
	}


	TDE2_API IResourceFactory* CreateFMODAudioClipFactory(IResourceManager* pResourceManager, IAudioContext* pAudioContext, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IResourceFactory, CFMODAudioClipFactory, result, pResourceManager, pAudioContext);
	}
}