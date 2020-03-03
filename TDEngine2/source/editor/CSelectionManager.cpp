#include "./../../include/editor/CSelectionManager.h"
#include "./../../include/editor/IEditorsManager.h"
#include "./../../include/ecs/IWorld.h"


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	CSelectionManager::CSelectionManager():
		CBaseObject()
	{
	}

	E_RESULT_CODE CSelectionManager::Init(IEditorsManager* pEditorsManager)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		if (!pEditorsManager)
		{
			return RC_INVALID_ARGS;
		}

		mpEditorsManager = pEditorsManager;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CSelectionManager::Free()
	{
		if (!mIsInitialized)
		{
			return RC_OK;
		}

		mIsInitialized = false;
		delete this;

		return RC_OK;
	}

	E_RESULT_CODE CSelectionManager::BuildSelectionMap(const TRenderFrameCallback& onDrawVisibleObjectsCallback)
	{


		if (onDrawVisibleObjectsCallback)
		{
			onDrawVisibleObjectsCallback();
		}

		return RC_OK;
	}

	E_RESULT_CODE CSelectionManager::SetWorldInstance(IWorld* pWorld)
	{
		if (!pWorld)
		{
			return RC_INVALID_ARGS;
		}

		mpWorld = pWorld;

		return RC_OK;
	}


	TDE2_API ISelectionManager* CreateSelectionManager(IEditorsManager* pEditorsManager, E_RESULT_CODE& result)
	{
		CSelectionManager* pSelectionManagerInstance = new (std::nothrow) CSelectionManager();

		if (!pSelectionManagerInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pSelectionManagerInstance->Init(pEditorsManager);

		if (result != RC_OK)
		{
			delete pSelectionManagerInstance;

			pSelectionManagerInstance = nullptr;
		}

		return dynamic_cast<ISelectionManager*>(pSelectionManagerInstance);
	}
}

#endif