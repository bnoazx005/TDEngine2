#include "../include/CFmodAudioContext.h"
#include "../include/CFmodAudioClip.h"
#include <stringUtils.hpp>
#include <utils/CFileLogger.h>
#include <utils/Utils.h>
#include <unordered_map>
#include <fmod.hpp>
#include <fmod_studio.hpp>
#include <fmod_errors.h>


namespace TDEngine2
{
	CFMODAudioContext::CFMODAudioContext() :
		CBaseObject(), mpSystem(nullptr)
	{
	}
	
	E_RESULT_CODE CFMODAudioContext::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = _initInternalContext();
		if (RC_OK != result)
		{
			return result;
		}

		LOG_MESSAGE("[FMOD Audio Context] FMOD audio context was successfully initialized");

		mIsInitialized = true;

		return RC_OK;
	}
	
	E_RESULT_CODE CFMODAudioContext::_onFreeInternal()
	{
		FMOD_RESULT result = mpSystem->release();
		if (FMOD_OK != result)
		{
			return RC_FAIL;
		}

		return RC_OK;
	}

	E_RESULT_CODE CFMODAudioContext::Update()
	{
		if (!mpSystem)
		{
			return RC_FAIL;
		}

		return (mpSystem->update() == FMOD_OK) ? RC_OK : RC_FAIL;
	}

	E_RESULT_CODE CFMODAudioContext::Play(IAudioSource* pAudioSource)
	{
		if (!pAudioSource)
		{
			return RC_INVALID_ARGS;
		}

		if (CFMODAudioClip* pAudioClip = dynamic_cast<CFMODAudioClip*>(pAudioSource))
		{
			FMOD_RESULT result = mpCoreSystem->playSound(pAudioClip->GetInternalHandle(), nullptr, false, 0);
			if (FMOD_OK != result)
			{
				return RC_FAIL;
			}
		}

		return RC_FAIL;
	}

	E_RESULT_CODE CFMODAudioContext::SetListenerPosition(const TVector3& position)
	{
		mCurrListenerPosition = position;

		const FMOD_VECTOR pos { position.x, position.y, position.z };

		// \todo Update this with proper orienting using camera's data
		const FMOD_VECTOR forward { ForwardVector3.x, ForwardVector3.y, ForwardVector3.z };
		const FMOD_VECTOR up { UpVector3.x, UpVector3.y, UpVector3.z };

		mpCoreSystem->set3DListenerAttributes(0, &pos, nullptr, &forward, &up);

		return RC_OK;
	}

	const TVector3& CFMODAudioContext::GetListenerPosition() const
	{
		return mCurrListenerPosition;
	}

	E_ENGINE_SUBSYSTEM_TYPE CFMODAudioContext::GetType() const
	{
		return E_ENGINE_SUBSYSTEM_TYPE::EST_AUDIO_CONTEXT;
	}

	FMOD::System* CFMODAudioContext::GetInternalContext() const
	{
		return mpCoreSystem;
	}

	E_RESULT_CODE CFMODAudioContext::_initInternalContext()
	{
		auto logError = [](const FMOD_RESULT& result)
		{
			LOG_ERROR(Wrench::StringUtils::Format("[FMOD Audio Context] An error's occurred during initialization, error_code: {0},\n message {1}", result, FMOD_ErrorString(result)));
		};

		FMOD_RESULT result = FMOD::Studio::System::create(&mpSystem); // Create the Studio System object.
		if (FMOD_OK != result)
		{
			logError(result);
			return RC_FAIL;
		}

		// Initialize FMOD Studio, which will also initialize FMOD Core
		result = mpSystem->initialize(512, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, 0);
		if (FMOD_OK != result)
		{
			logError(result);
			return RC_FAIL;
		}

		result = mpSystem->getCoreSystem(&mpCoreSystem);
		if (FMOD_OK != result)
		{
			logError(result);
			return RC_FAIL;
		}

		return RC_OK;
	}


	TDE2_API IAudioContext* CreateFMODAudioContext(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IAudioContext, CFMODAudioContext, result);
	}
}