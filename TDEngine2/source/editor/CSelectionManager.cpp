#include "./../../include/editor/CSelectionManager.h"
#include "./../../include/editor/IEditorsManager.h"
#include "./../../include/ecs/IWorld.h"
#include "./../../include/ecs/CObjectsSelectionSystem.h"
#include "./../../include/utils/CFileLogger.h"
#include <functional>


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

	E_RESULT_CODE CSelectionManager::OnEvent(const TBaseEvent* pEvent)
	{
		TypeId eventType = pEvent->GetEventType();

		static const std::unordered_map<TypeId, std::function<E_RESULT_CODE()>> handlers 
		{
			{ 
				TOnEditorModeEnabled::GetTypeId(), [this, pEvent]
				{
					return mpWorld->ActivateSystem(mObjectSelectionSystemId);
				} 
			},
			{ 
				TOnEditorModeDisabled::GetTypeId(), [this, pEvent]
				{
					return mpWorld->DeactivateSystem(mObjectSelectionSystemId);
				} 
			},
		};

		auto iter = handlers.cbegin();
		
		if ((iter = handlers.find(eventType)) != handlers.cend())
		{
			return iter->second();
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

		if ((mObjectSelectionSystemId = mpWorld->FindSystem<CObjectsSelectionSystem>()) == InvalidSystemId)
		{
			LOG_ERROR("[CSelectionManager] \"ObjectsSelection\" system wasn't found");
			return RC_FAIL;
		}

		return mpWorld->DeactivateSystem(mObjectSelectionSystemId);
	}

	TEventListenerId CSelectionManager::GetListenerId() const
	{
		return GetTypeId();
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