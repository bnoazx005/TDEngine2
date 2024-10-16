#include "../../include/editor/ecs/CEditorCameraControlSystem.h"
#include "../../include/ecs/IWorld.h"
#include "../../include/ecs/CTransform.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/graphics/CBaseCamera.h"
#include "../../include/graphics/COrthoCamera.h"
#include "../../include/graphics/CPerspectiveCamera.h"
#include "../../include/core/IInputContext.h"
#include "../../include/editor/IEditorsManager.h"
#include "../../include/utils/CFileLogger.h"


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

		mCurrRotation = ZeroVector3;

		mIsInitialized = true;

		return RC_OK;
	}

	void CEditorCameraControlSystem::InjectBindings(IWorld* pWorld)
	{
		mCameras = pWorld->FindEntitiesWithAny<CEditorCamera>();
	}

	void CEditorCameraControlSystem::Update(IWorld* pWorld, F32 dt)
	{
		if (!mpEditorManager->IsEditorModeEnabled())
		{
			return;
		}

		CEntity* pCurrEntity = nullptr;

		CTransform* pCurrTransform = nullptr;

		const F32 cameraSpeed = (mpInputContext->IsKey(E_KEYCODES::KC_LSHIFT) || mpInputContext->IsKey(E_KEYCODES::KC_RSHIFT)) ? mIncreasedEditorCameraSpeed : mDefaultEditorCameraSpeed;

		for (TEntityId currCameraEntityId : mCameras)
		{
			if (!(pCurrEntity = pWorld->FindEntity(currCameraEntityId)))
			{
				continue;
			}

			pCurrTransform = pCurrEntity->GetComponent<CTransform>();
			TDE2_ASSERT(pCurrTransform);

			const TMatrix4& camera2World = Transpose(pCurrTransform->GetLocalToWorldTransform());
			
			const TVector3 forward = Normalize(TVector3(camera2World.m[0][2], camera2World.m[1][2], camera2World.m[2][2]));
			const TVector3 right   = Normalize(TVector3(camera2World.m[0][0], camera2World.m[1][0], camera2World.m[2][0]));

			const std::tuple<E_KEYCODES, F32, TVector3> controls[4]
			{
				{ E_KEYCODES::KC_W, dt * cameraSpeed, forward },
				{ E_KEYCODES::KC_S, -dt * cameraSpeed, forward },
				{ E_KEYCODES::KC_D, dt * cameraSpeed, right },
				{ E_KEYCODES::KC_A, -dt * cameraSpeed, right },
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
		if (!inputContext.IsMouseButton(1))
		{
			return;
		}

		const TVector3 currMousePosition = inputContext.GetMouseShiftVec();
		mCurrRotation.x -= mEditorCameraSensitivity * currMousePosition.y;
		mCurrRotation.y -= mEditorCameraSensitivity * currMousePosition.x;

		currTransform.SetRotation(mCurrRotation);
	}


	TDE2_API ISystem* CreateEditorCameraControlSystem(IInputContext* pInputContext, IEditorsManager* pEditorManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem, CEditorCameraControlSystem, result, pInputContext, pEditorManager);
	}
}

#endif