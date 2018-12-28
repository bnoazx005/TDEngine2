#include "./../../include/ecs/CCameraSystem.h"
#include "./../../include/ecs/IWorld.h"
#include "./../../include/ecs/CEntity.h"
#include "./../../include/graphics/CBaseCamera.h"
#include "./../../include/graphics/CPerspectiveCamera.h"
#include "./../../include/graphics/COrthoCamera.h"
#include "./../../include/ecs/CTransform.h"
#include "./../../include/core/IGraphicsContext.h"
#include "./../../include/core/IWindowSystem.h"


namespace TDEngine2
{
	CCameraSystem::CCameraSystem() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CCameraSystem::Init(const IWindowSystem* pWindowSystem, IGraphicsContext* pGraphicsContext)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pGraphicsContext || !pWindowSystem)
		{
			return RC_INVALID_ARGS;
		}

		mpGraphicsContext = pGraphicsContext;

		mpWindowSystem = pWindowSystem;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CCameraSystem::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	void CCameraSystem::InjectBindings(IWorld* pWorld)
	{
		std::vector<TEntityId> perspectiveCameras  = pWorld->FindEntitiesWithComponents<CTransform, CPerspectiveCamera>();
		std::vector<TEntityId> orthographicCameras = pWorld->FindEntitiesWithComponents<CTransform, COrthoCamera>();

		mCamerasTransforms.clear();

		mCameras.clear();

		CEntity* pCurrEntity = nullptr;

		/// \todo refactor and clean up this code after refactoring Typing system
		for (auto iter = perspectiveCameras.begin(); iter != perspectiveCameras.end(); ++iter)
		{
			pCurrEntity = pWorld->FindEntity(*iter);

			if (!pCurrEntity)
			{
				continue;
			}

			mCamerasTransforms.push_back(pCurrEntity->GetComponent<CTransform>());

			mCameras.push_back(pCurrEntity->GetComponent<CPerspectiveCamera>());
		}

		/// \todo refactor and clean up this code after refactoring Typing system
		for (auto iter = orthographicCameras.begin(); iter != orthographicCameras.end(); ++iter)
		{
			pCurrEntity = pWorld->FindEntity(*iter);

			if (!pCurrEntity)
			{
				continue;
			}

			mCamerasTransforms.push_back(pCurrEntity->GetComponent<CTransform>());

			mCameras.push_back(pCurrEntity->GetComponent<CBaseCamera>());
		}
	}

	void CCameraSystem::Update(IWorld* pWorld, F32 dt)
	{
		CTransform* pCurrTransform = nullptr;

		CBaseCamera* pCurrCamera = nullptr;

		for (U32 i = 0; i < mCameras.size(); ++i)
		{
			pCurrCamera = mCameras[i];

			pCurrTransform = mCamerasTransforms[i];

			pCurrCamera->SetViewMatrix(Transpose(pCurrTransform->GetTransform()));

			pCurrCamera->ComputeProjectionMatrix(this);
		}
	}

	E_RESULT_CODE CCameraSystem::ComputePerspectiveProjection(IPerspectiveCamera* pCamera) const
	{
		if (!pCamera)
		{
			return RC_INVALID_ARGS;
		}
		
		pCamera->SetProjMatrix(Transpose(mpGraphicsContext->CalcPerspectiveMatrix(pCamera->GetFOV(), pCamera->GetAspect(), 
																				  pCamera->GetNearPlane(), pCamera->GetFarPlane())));

		return RC_OK;
	}

	E_RESULT_CODE CCameraSystem::ComputeOrthographicProjection(IOrthoCamera* pCamera) const
	{
		if (!pCamera)
		{
			return RC_INVALID_ARGS;
		}

		F32 halfWidth  = 0.5f * pCamera->GetWidth();
		F32 halfHeight = 0.5f * pCamera->GetHeight();

		pCamera->SetProjMatrix(Transpose(mpGraphicsContext->CalcOrthographicMatrix(-halfWidth, halfHeight, halfWidth, -halfHeight, 
																				   pCamera->GetNearPlane(), pCamera->GetFarPlane())));

		return RC_OK;
	}


	TDE2_API ISystem* CreateCameraSystem(const IWindowSystem* pWindowSystem, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		CCameraSystem* pSystemInstance = new (std::nothrow) CCameraSystem();

		if (!pSystemInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pSystemInstance->Init(pWindowSystem, pGraphicsContext);

		if (result != RC_OK)
		{
			delete pSystemInstance;

			pSystemInstance = nullptr;
		}

		return dynamic_cast<ISystem*>(pSystemInstance);
	}
}