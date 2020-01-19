#include "./../../include/editor/ecs/CEditorCameraControlSystem.h"
#include "./../../include/ecs/IWorld.h"
#include "./../../include/ecs/CTransform.h"
#include "./../../include/ecs/CEntity.h"
#include "./../../include/graphics/COrthoCamera.h"
#include "./../../include/graphics/CPerspectiveCamera.h"
#include "./../../include/core/IInputContext.h"
#include "./../../include/editor/IEditorsManager.h"


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	CEditorCameraControlSystem::CEditorCameraControlSystem() :
		CBaseSystem()
	{
	}

	E_RESULT_CODE CEditorCameraControlSystem::Init(IInputContext* pInputContext, IEditorsManager* pEditorManager)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pInputContext || !pEditorManager)
		{
			return RC_INVALID_ARGS;
		}

		mpInputContext  = dynamic_cast<IDesktopInputContext*>(pInputContext);
		mpEditorManager = pEditorManager;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CEditorCameraControlSystem::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	void CEditorCameraControlSystem::InjectBindings(IWorld* pWorld)
	{
		mCameras = pWorld->FindEntitiesWithAny<CPerspectiveCamera, COrthoCamera>();
	}

	void CEditorCameraControlSystem::Update(IWorld* pWorld, F32 dt)
	{
		if (!mpEditorManager->IsEditorModeEnabled())
		{
			return;
		}

		CEntity* pCurrEntity = nullptr;

		CTransform* pCurrTransform = nullptr;

		for (TEntityId currCameraEntityId : mCameras)
		{
			if (!(pCurrEntity = pWorld->FindEntity(currCameraEntityId)))
			{
				continue;
			}

			pCurrTransform = pCurrEntity->GetComponent<CTransform>();
			TDE2_ASSERT(pCurrTransform);

			if (mpInputContext->IsKey(E_KEYCODES::KC_W))
			{
				pCurrTransform->SetPosition(pCurrTransform->GetPosition() + dt * 5.0f * UpVector3);
			}

			if (mpInputContext->IsKey(E_KEYCODES::KC_S))
			{
				pCurrTransform->SetPosition(pCurrTransform->GetPosition() - dt * 5.0f * UpVector3);
			}

			if (mpInputContext->IsKey(E_KEYCODES::KC_A))
			{
				pCurrTransform->SetPosition(pCurrTransform->GetPosition() - dt * 5.0f * RightVector3);
			}

			if (mpInputContext->IsKey(E_KEYCODES::KC_D))
			{
				pCurrTransform->SetPosition(pCurrTransform->GetPosition() + dt * 5.0f * RightVector3);
			}

			if (mpInputContext->IsKey(E_KEYCODES::KC_Q))
			{
				pCurrTransform->SetPosition(pCurrTransform->GetPosition() - dt * 5.0f * ForwardVector3);
			}

			if (mpInputContext->IsKey(E_KEYCODES::KC_E))
			{
				pCurrTransform->SetPosition(pCurrTransform->GetPosition() + dt * 5.0f * ForwardVector3);
			}
		}
	}


	TDE2_API ISystem* CreateEditorCameraControlSystem(IInputContext* pInputContext, IEditorsManager* pEditorManager, E_RESULT_CODE& result)
	{
		CEditorCameraControlSystem* pSystemInstance = new (std::nothrow) CEditorCameraControlSystem();

		if (!pSystemInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pSystemInstance->Init(pInputContext, pEditorManager);

		if (result != RC_OK)
		{
			delete pSystemInstance;

			pSystemInstance = nullptr;
		}

		return dynamic_cast<ISystem*>(pSystemInstance);
	}
}

#endif