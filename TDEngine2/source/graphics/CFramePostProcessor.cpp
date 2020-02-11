#include "./../../include/graphics/CFramePostProcessor.h"


namespace TDEngine2
{
	CFramePostProcessor::CFramePostProcessor() :
		CBaseObject()
	{
	}


	E_RESULT_CODE CFramePostProcessor::Init()
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CFramePostProcessor::Free()
	{
		if (!mIsInitialized)
		{
			return RC_OK;
		}

		mIsInitialized = false;
		delete this;

		return RC_OK;
	}

	E_RESULT_CODE CFramePostProcessor::SetProcessingProfile(const IPostProcessingProfile* pProfileResource)
	{
		TDE2_UNIMPLEMENTED();
		return RC_NOT_IMPLEMENTED_YET;
	}


	TDE2_API IFramePostProcessor* CreateFramePostProcessor(E_RESULT_CODE& result)
	{
		CFramePostProcessor* pPostProcessorInstance = new (std::nothrow) CFramePostProcessor();

		if (!pPostProcessorInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pPostProcessorInstance->Init();

		if (result != RC_OK)
		{
			delete pPostProcessorInstance;

			pPostProcessorInstance = nullptr;
		}

		return dynamic_cast<IFramePostProcessor*>(pPostProcessorInstance);
	}
}