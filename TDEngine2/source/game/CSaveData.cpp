#include "../../include/game/CSaveData.h"


namespace TDEngine2
{
	CSaveData::CSaveData() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CSaveData::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CSaveData::Free()
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

	E_RESULT_CODE CSaveData::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_INVALID_ARGS;
		}

		pReader->BeginGroup("save-header");
		{
			mId = pReader->GetString("id");
			TDE2_ASSERT(!mId.empty());
		}
		pReader->EndGroup();



		return RC_OK;
	}

	E_RESULT_CODE CSaveData::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_INVALID_ARGS;
		}

		return RC_OK;
	}
	
	E_RESULT_CODE CSaveData::SetId(const std::string& saveId)
	{
		if (saveId.empty())
		{
			return RC_INVALID_ARGS;
		}

		mId = saveId;

		return RC_OK;
	}

	E_RESULT_CODE CSaveData::SetPath(const std::string& savePath)
	{
		if (savePath.empty())
		{
			return RC_INVALID_ARGS;
		}

		mPath = savePath;

		return RC_OK;
	}

	const std::string& CSaveData::GetId() const
	{
		return mId;
	}

	const std::string& CSaveData::GetPath() const
	{
		return mPath;
	}

	TDE2_API ISaveData* CreateSaveData(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISaveData, CSaveData, result);
	}
}