#include "../../include/ecs/CCameraSystem.h"
#include "../../include/ecs/IWorld.h"
#include "../../include/ecs/CWorld.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/graphics/CBaseCamera.h"
#include "../../include/graphics/CPerspectiveCamera.h"
#include "../../include/graphics/COrthoCamera.h"
#include "../../include/ecs/CTransform.h"
#include "../../include/core/IGraphicsContext.h"
#include "../../include/core/IWindowSystem.h"
#include "../../include/graphics/IRenderer.h"
#include "../../include/editor/CPerfProfiler.h"
#include <algorithm>


namespace TDEngine2
{
	CCameraSystem::CCameraSystem() :
		CBaseSystem()
	{
	}

	E_RESULT_CODE CCameraSystem::Init(const IWindowSystem* pWindowSystem, IGraphicsContext* pGraphicsContext, IRenderer* pRenderer)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pGraphicsContext || !pWindowSystem || !pRenderer)
		{
			return RC_INVALID_ARGS;
		}

		mpGraphicsContext = pGraphicsContext;

		mpWindowSystem = pWindowSystem;

		mpRenderer = pRenderer;

		mIsInitialized = true;

		return RC_OK;
	}

	void CCameraSystem::InjectBindings(IWorld* pWorld)
	{
		auto&& entities = pWorld->FindEntitiesWithAny<COrthoCamera, CPerspectiveCamera>();

		auto& cameras = mCamerasContext.mpCameras;
		auto& transforms = mCamerasContext.mpTransforms;
		auto& ents = mCamerasContext.mEntities;

		cameras.clear();
		transforms.clear();
		ents.clear();

		for (auto currEntityId : entities)
		{
			if (auto pCurrEntity = pWorld->FindEntity(currEntityId))
			{
				CBaseCamera* pCamera = pCurrEntity->GetComponent<CPerspectiveCamera>();
				pCamera = pCamera ? pCamera : pCurrEntity->GetComponent<COrthoCamera>();

				cameras.push_back(pCamera);
				transforms.push_back(pCurrEntity->GetComponent<CTransform>());
				ents.push_back(currEntityId);
			}
		}

		if (CEntity* pCamerasContextEntity = pWorld->FindEntity(pWorld->FindEntityWithUniqueComponent<CCamerasContextComponent>()))
		{
			mpCamerasContextComponent = pCamerasContextEntity->GetComponent<CCamerasContextComponent>();
		}
	}

	void CCameraSystem::Update(IWorld* pWorld, F32 dt)
	{
		TDE2_PROFILER_SCOPE("CCameraSystem::Update");

		TDE2_ASSERT(mpCamerasContextComponent);

		CTransform* pCurrTransform = nullptr;

		CBaseCamera* pCurrCamera = nullptr;

		const F32 graphicsCtxPositiveZAxisDirection = mpGraphicsContext->GetPositiveZAxisDirection();
		const F32 ndcZmin = mpGraphicsContext->GetContextInfo().mNDCBox.min.z;

		CEntity* pCurrEntity = nullptr;

		for (USIZE i = 0; i < mCamerasContext.mpCameras.size(); ++i)
		{
			pCurrCamera = mCamerasContext.mpCameras[i];
			pCurrTransform = mCamerasContext.mpTransforms[i];

			TMatrix4 viewMatrix = pCurrTransform->GetLocalToWorldTransform();

			viewMatrix.m[0][3] = -viewMatrix.m[0][3];
			viewMatrix.m[1][3] = -viewMatrix.m[1][3];

			// \note This thing is a kind of a hack for OpenGL graphics context which is using orthographic projection to make it uniform for both GAPIs
			viewMatrix.m[2][3] *= ((pCurrCamera->GetProjType() == E_CAMERA_PROJECTION_TYPE::ORTHOGRAPHIC) && (graphicsCtxPositiveZAxisDirection < 0.0f)) ? 1.0f : -1.0f;
			
			pCurrCamera->SetViewMatrix(viewMatrix);
			pCurrCamera->ComputeProjectionMatrix(this);

			pCurrCamera->SetViewProjMatrix(pCurrCamera->GetProjMatrix() * pCurrCamera->GetViewMatrix(), ndcZmin);
			pCurrCamera->SetPosition(pCurrTransform->GetPosition());
		}

		/// \note Update active camera
		auto it = std::find(mCamerasContext.mEntities.cbegin(), mCamerasContext.mEntities.cend(), mpCamerasContextComponent->GetActiveCameraEntityId());
		SetMainCamera(mCamerasContext.mpCameras[std::distance(mCamerasContext.mEntities.cbegin(), it)]);
	}

	E_RESULT_CODE CCameraSystem::ComputePerspectiveProjection(IPerspectiveCamera* pCamera) const
	{
		if (!pCamera)
		{
			return RC_INVALID_ARGS;
		}

		F32 aspect = mpWindowSystem->GetWidth() / static_cast<F32>(mpWindowSystem->GetHeight());
		
		pCamera->SetAspect(aspect);

		pCamera->SetProjMatrix(mpGraphicsContext->CalcPerspectiveMatrix(pCamera->GetFOV(), aspect, pCamera->GetNearPlane(), pCamera->GetFarPlane()));

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

		pCamera->SetProjMatrix(mpGraphicsContext->CalcOrthographicMatrix(-halfWidth, halfHeight, halfWidth, -halfHeight, 
																				   pCamera->GetNearPlane(), pCamera->GetFarPlane()));

		return RC_OK;
	}

	E_RESULT_CODE CCameraSystem::SetMainCamera(const ICamera* pCamera)
	{
		if (!pCamera)
		{
			return RC_INVALID_ARGS;
		}

		mpMainCamera = pCamera;

		if (!mpRenderer)
		{
			return RC_FAIL;
		}

		mpRenderer->SetCamera(mpMainCamera);

		return RC_OK;
	}

	const ICamera* CCameraSystem::GetMainCamera() const
	{
		return mpMainCamera;
	}


	TDE2_API ISystem* CreateCameraSystem(const IWindowSystem* pWindowSystem, IGraphicsContext* pGraphicsContext, IRenderer* pRenderer, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem, CCameraSystem, result, pWindowSystem, pGraphicsContext, pRenderer);
	}


	TRay3D NormalizedScreenPointToWorldRay(const ICamera& pCamera, const TVector2& pos)
	{
		const TMatrix4& viewProjInverse = pCamera.GetInverseViewProjMatrix();

		TVector4 origin = viewProjInverse * TVector4(pos.x, pos.y, 0.0f, 1.0f);
		origin = origin * (1.0f / origin.w);

		TVector4 rayEnd = viewProjInverse * TVector4(pos.x, pos.y, 1.0f, 1.0f);
		rayEnd = rayEnd * (1.0f / rayEnd.w);

		TVector4 dir = Normalize(rayEnd - origin);

		return { { origin.x, origin.y, origin.z }, { dir.x, dir.y, dir.z } };
	}


	TVector3 WorldToNormalizedScreenPoint(const ICamera& pCamera, const TVector3& pos)
	{
		const TMatrix4& viewProj = pCamera.GetViewProjMatrix();

		TVector4 screenSpacePoint = viewProj * TVector4(pos, 1.0f);
		screenSpacePoint = screenSpacePoint * (1.0f / screenSpacePoint.w);

		return TVector3(screenSpacePoint.x, screenSpacePoint.y, screenSpacePoint.z);
	}
}