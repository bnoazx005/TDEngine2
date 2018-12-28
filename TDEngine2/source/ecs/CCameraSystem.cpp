#include "./../../include/ecs/CCameraSystem.h"
#include "./../../include/ecs/IWorld.h"
#include "./../../include/ecs/CEntity.h"
#include "./../../include/graphics/CBaseCamera.h"
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
		std::vector<TEntityId> entities = pWorld->FindEntitiesWithComponents<CTransform, CBaseCamera>();

		mCamerasTransforms.clear();

		mCameras.clear();

		CEntity* pCurrEntity = nullptr;

		for (auto iter = entities.begin(); iter != entities.end(); ++iter)
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