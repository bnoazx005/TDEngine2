#include "../../include/game/CSaveManager.h"
#include "../../include/core/IFileSystem.h"
#include "../../include/utils/CFileLogger.h"


namespace TDEngine2
{
	CSaveManager::CSaveManager():
		CBaseObject()
	{
	}

	E_RESULT_CODE CSaveManager::Init(const TSaveManagerParametersDesc& desc)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!desc.mpFileSystem)
		{
			return RC_INVALID_ARGS;
		}

		std::lock_guard<std::mutex> lock(mMutex);

		mpFileSystem = desc.mpFileSystem;

		LOG_MESSAGE("[Save Manager] The save manager system was successfully initialized");

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CSaveManager::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		--mRefCounter;

		if (!mRefCounter)
		{
			mIsInitialized = false;
			delete this;
		}

		return RC_OK;
	}

	E_ENGINE_SUBSYSTEM_TYPE CSaveManager::GetType() const
	{
		return ISaveManager::GetTypeID();
	}


	TDE2_API ISaveManager* CreateSaveManager(const TSaveManagerParametersDesc& desc, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISaveManager, CSaveManager, result, desc);
	}
}