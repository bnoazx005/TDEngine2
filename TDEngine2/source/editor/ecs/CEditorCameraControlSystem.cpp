#include "./../../include/editor/ecs/CEditorCameraControlSystem.h"
#include "./../../include/ecs/IWorld.h"
#include "./../../include/ecs/CTransform.h"
#include "./../../include/ecs/CEntity.h"
#include "./../../include/graphics/COrthoCamera.h"
#include "./../../include/graphics/CPerspectiveCamera.h"
#include "./../../include/core/IInputContext.h"
#include "./../../include/editor/IEditorsManager.h"
#include "./../../include/utils/CFileLogger.h"


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

			TVector3 forward = Normalize(pCurrTransform->GetForwardVector());
			TVector3 right   = Normalize(pCurrTransform->GetRightVector());

			const std::tuple<E_KEYCODES, F32, TVector3> controls[4]
			{
				{ E_KEYCODES::KC_W, dt * 5.0f, forward },
				{ E_KEYCODES::KC_S, -dt * 5.0f, forward },
				{ E_KEYCODES::KC_D, dt * 5.0f, right },
				{ E_KEYCODES::KC_A, -dt * 5.0f, right },
			};

			for (auto&& currControl : controls)
			{
				if (mpInputContext->IsKey(std::get<E_KEYCODES>(currControl)))
				{
					pCurrTransform->SetPosition(pCurrTransform->GetPosition() + std::get<F32>(currControl) * std::get<TVector3>(currControl));
				}
			}

			_processCameraRotation(*mpInputContext, *pCurrTransform);
		}
	}

	void CEditorCameraControlSystem::_processCameraRotation(IDesktopInputContext& inputContext, CTransform& currTransform)
	{
		if (inputContext.IsMouseButtonPressed(1))
		{
			mLastClickedPosition = inputContext.GetMousePosition();
		}

		if (inputContext.IsMouseButton(1))
		{
			TVector3 currMousePosition = inputContext.GetMousePosition();
			TVector3 deltaVec = (currMousePosition - mLastClickedPosition) * 0.01f;

			mCurrDeltaRotation = TVector3(deltaVec.y, deltaVec.x, 0.0f);
			mCurrRotation = mCurrDeltaRotation;

			currTransform.SetRotation(mCurrDeltaRotation);
		}

		if (inputContext.IsMouseButtonUnpressed(1))
		{
			LOG_MESSAGE(CStringUtils::Format("{0} {1} {2}", mCurrRotation.x, mCurrRotation.y, mCurrRotation.z));
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