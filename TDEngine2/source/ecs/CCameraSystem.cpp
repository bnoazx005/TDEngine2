#include "../../include/ecs/CCameraSystem.h"
#include "../../include/ecs/IWorld.h"
#include "../../include/ecs/CWorld.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/ecs/CSystemManager.h"
#include "../../include/ecs/CTransformSystem.h"
#include "../../include/editor/ecs/CEditorCameraControlSystem.h"
#include "../../include/editor/ecs/EditorComponents.h"
#include "../../include/graphics/CCamera.h"
#include "../../include/graphics/IDebugUtility.h"
#include "../../include/ecs/CTransform.h"
#include "../../include/core/IGraphicsContext.h"
#include "../../include/core/IWindowSystem.h"
#include "../../include/graphics/IRenderer.h"
#include "../../include/editor/CPerfProfiler.h"
#include "../../include/utils/CContainers.h"
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
		auto&& entities = pWorld->FindEntitiesWithAny<CCamera>();

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
				CCamera* pCamera = pCurrEntity->GetComponent<CCamera>();

				cameras.push_back(pCamera);
				transforms.push_back(pCurrEntity->GetComponent<CTransform>());
				ents.push_back(currEntityId);
			}
		}

		if (CEntity* pCamerasContextEntity = pWorld->FindEntity(pWorld->FindEntityWithUniqueComponent<CCamerasContextComponent>()))
		{
			mpCamerasContextComponent = pCamerasContextEntity->GetComponent<CCamerasContextComponent>();
		}

#if TDE2_EDITORS_ENABLED
		mpWorld = pWorld;
#endif
	}


#if TDE2_EDITORS_ENABLED
	
	static void DrawCameraFrustums(IDebugUtility* pDebugUtility, IWorld* pWorld, const CCameraSystem::TSystemContext& camerasContext, TEntityId activeCameraId, F32 ndcZMin)
	{
		for (USIZE i = 0; i < camerasContext.mEntities.size(); i++)
		{
			if (camerasContext.mEntities[i] == activeCameraId)
			{
				continue;
			}

			CEntity* pEntity = pWorld->FindEntity(camerasContext.mEntities[i]);
			if (!pEntity->HasComponent<CSelectedEntityComponent>())
			{
				continue;
			}

			CCamera* pCamera = camerasContext.mpCameras[i];
			TPtr<IFrustum> pFrustum = pCamera->GetFrustum();

			const auto& vertices = pFrustum->GetVertices(pCamera->GetInverseViewProjMatrix(), ndcZMin);

			pDebugUtility->DrawLine(static_cast<TVector3>(vertices[0]), static_cast<TVector3>(vertices[1]), TColorUtils::mMagenta);
			pDebugUtility->DrawLine(static_cast<TVector3>(vertices[1]), static_cast<TVector3>(vertices[3]), TColorUtils::mMagenta);
			pDebugUtility->DrawLine(static_cast<TVector3>(vertices[2]), static_cast<TVector3>(vertices[3]), TColorUtils::mMagenta);
			pDebugUtility->DrawLine(static_cast<TVector3>(vertices[2]), static_cast<TVector3>(vertices[0]), TColorUtils::mMagenta);

			pDebugUtility->DrawLine(static_cast<TVector3>(vertices[4]), static_cast<TVector3>(vertices[5]), TColorUtils::mMagenta);
			pDebugUtility->DrawLine(static_cast<TVector3>(vertices[5]), static_cast<TVector3>(vertices[7]), TColorUtils::mMagenta);
			pDebugUtility->DrawLine(static_cast<TVector3>(vertices[6]), static_cast<TVector3>(vertices[7]), TColorUtils::mMagenta);
			pDebugUtility->DrawLine(static_cast<TVector3>(vertices[6]), static_cast<TVector3>(vertices[4]), TColorUtils::mMagenta);

			for (USIZE i = 0; i < 4; i++)
			{
				pDebugUtility->DrawLine(static_cast<TVector3>(vertices[i]), static_cast<TVector3>(vertices[4 + i]), TColorUtils::mMagenta);
			}
		}
	}

#endif

	void CCameraSystem::Update(IWorld* pWorld, F32 dt)
	{
		TDE2_PROFILER_SCOPE("CCameraSystem::Update");

		TDE2_ASSERT(mpCamerasContextComponent);

		CTransform* pCurrTransform = nullptr;

		CCamera* pCurrCamera = nullptr;

		const F32 graphicsCtxPositiveZAxisDirection = mpGraphicsContext->GetPositiveZAxisDirection();
		const F32 ndcZmin = mpGraphicsContext->GetContextInfo().mNDCBox.min.z;

		CEntity* pCurrEntity = nullptr;

		for (USIZE i = 0; i < mCamerasContext.mpCameras.size(); ++i)
		{
			pCurrCamera = mCamerasContext.mpCameras[i];
			pCurrTransform = mCamerasContext.mpTransforms[i];

			TCameraComponentData& cameraData = pCurrCamera->GetData();

			TMatrix4 viewMatrix = pCurrTransform->GetLocalToWorldTransform();

			viewMatrix.m[0][3] = -viewMatrix.m[0][3];
			viewMatrix.m[1][3] = -viewMatrix.m[1][3];

			// \note This thing is a kind of a hack for OpenGL graphics context which is using orthographic projection to make it uniform for both GAPIs
			viewMatrix.m[2][3] *= ((cameraData.mType == E_CAMERA_PROJECTION_TYPE::ORTHOGRAPHIC) && (graphicsCtxPositiveZAxisDirection < 0.0f)) ? 1.0f : -1.0f;
			
			cameraData.mViewMatrix = viewMatrix;

			switch (cameraData.mType)
			{
				case E_CAMERA_PROJECTION_TYPE::ORTHOGRAPHIC:
					{
						const TVector2 halfSizes = 0.5f * cameraData.mParams;

						cameraData.mProjMatrix = mpGraphicsContext->CalcOrthographicMatrix(-halfSizes.x, halfSizes.y, halfSizes.x, -halfSizes.y,
							cameraData.mZNear, cameraData.mZFar);
					}
					break;
				case E_CAMERA_PROJECTION_TYPE::PERSPECTIVE:
					{
						const F32 aspect = mpWindowSystem->GetWidth() / static_cast<F32>(mpWindowSystem->GetHeight());

						cameraData.mParams.y = aspect;
						cameraData.mProjMatrix = mpGraphicsContext->CalcPerspectiveMatrix(cameraData.mParams.x, aspect, cameraData.mZNear, cameraData.mZFar);
					}
					break;
			}

			pCurrCamera->SetViewProjMatrix(pCurrCamera->GetProjMatrix() * pCurrCamera->GetViewMatrix(), ndcZmin);
			cameraData.mPosition = pCurrTransform->GetPosition();
		}
	}

#if TDE2_EDITORS_ENABLED

	void CCameraSystem::DebugOutput(IDebugUtility* pDebugUtility, F32 dt) const
	{
		DrawCameraFrustums(pDebugUtility, mpWorld, mCamerasContext, mpCamerasContextComponent->GetActiveCameraEntityId(), mpGraphicsContext->GetContextInfo().mNDCBox.min.z);
	}

#endif


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


	struct TCameraUpdateSystemAutoInitializer : TSystemAutoInitializer
	{
		virtual ~TCameraUpdateSystemAutoInitializer() = default;

		E_RESULT_CODE ManageDependencies(IWorld* pWorld) override
		{
			CFixedVector<TSystemId, 2> dependenciesHandles
			{
				pWorld->FindSystem<CTransformSystem>(),
#if TDE2_EDITORS_ENABLED
				pWorld->FindSystem<CEditorCameraControlSystem>(),
#endif
			};

			E_RESULT_CODE result = RC_OK;

			for (const TSystemId& currDependencyHandle : dependenciesHandles)
			{
				result = result | (currDependencyHandle == TSystemId::Invalid ? RC_FAIL : RC_OK);
				result = result | pSystemInstance->AddDependency(pWorld->GetSystem(currDependencyHandle));
			}

			return result;
		}

		ISystem* GetSystem(IWorld* pWorld, const TRequestSubsystemCallback& subsystemsProviderCallback)
		{
			E_RESULT_CODE result = RC_OK;
			pSystemInstance = CreateCameraSystem(
				PolymorphicCast<IWindowSystem*>(subsystemsProviderCallback(E_ENGINE_SUBSYSTEM_TYPE::EST_WINDOW)),
				PolymorphicCast<IGraphicsContext*>(subsystemsProviderCallback(E_ENGINE_SUBSYSTEM_TYPE::EST_GRAPHICS_CONTEXT)),
				PolymorphicCast<IRenderer*>(subsystemsProviderCallback(E_ENGINE_SUBSYSTEM_TYPE::EST_RENDERER)), result);

			return pSystemInstance;
		}

		ISystem* pSystemInstance = nullptr;
	};


	TDE2_REGISTER_SYSTEM(TCameraUpdateSystemAutoInitializer);
}