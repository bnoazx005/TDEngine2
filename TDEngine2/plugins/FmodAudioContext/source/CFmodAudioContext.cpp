#include "../include/CFMODAudioContext.h"
#include <stringUtils.hpp>
#include <utils/CFileLogger.h>
#include <utils/Utils.h>
#include <unordered_map>


namespace TDEngine2
{
	CFMODAudioContext::CFMODAudioContext() :
		mIsInitialized(false)
	{
	}
	
	E_RESULT_CODE CFMODAudioContext::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}
	
	E_RESULT_CODE CFMODAudioContext::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		delete this;

		mIsInitialized = false;

		return RC_OK;
	}

	E_ENGINE_SUBSYSTEM_TYPE CFMODAudioContext::GetType() const
	{
		return E_ENGINE_SUBSYSTEM_TYPE::EST_AUDIO_CONTEXT;
	}


	TDE2_API IAudioContext* CreateFMODAudioContext(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IAudioContext, CFMODAudioContext, result);
	}
}