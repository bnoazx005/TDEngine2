#include "../../include/platform/MountableStorages.h"


namespace TDEngine2
{
	/*!
		\brief CPhysicalFilesStorage's definition
	*/

	CPhysicalFilesStorage::CPhysicalFilesStorage() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CPhysicalFilesStorage::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CPhysicalFilesStorage::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		delete this;

		mIsInitialized = false;

		return RC_OK;
	}
	
	E_RESULT_CODE CPhysicalFilesStorage::OnMounted()
	{
		TDE2_UNIMPLEMENTED();
		return RC_NOT_IMPLEMENTED_YET;
	}


	IMountableStorage* CreatePhysicalFilesStorage(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IMountableStorage, CPhysicalFilesStorage, result);
	}


	/*!
		\brief CPackageFilesStorage's definition
	*/

	CPackageFilesStorage::CPackageFilesStorage() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CPackageFilesStorage::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CPackageFilesStorage::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		delete this;

		mIsInitialized = false;

		return RC_OK;
	}

	E_RESULT_CODE CPackageFilesStorage::OnMounted()
	{
		TDE2_UNIMPLEMENTED();
		return RC_NOT_IMPLEMENTED_YET;
	}


	IMountableStorage* CreatePackageFilesStorage(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IMountableStorage, CPackageFilesStorage, result);
	}
}