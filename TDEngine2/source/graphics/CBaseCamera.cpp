#include "../../include/graphics/CBaseCamera.h"
#include "../../include/math/TAABB.h"
#include "../../include/utils/Utils.h"
#include "../../include/ecs/CComponentManager.h"
#include "../../include/ecs/IWorld.h"
#include <array>
#include "deferOperation.hpp"


namespace TDEngine2
{
	TDE2_REGISTER_COMPONENT_FACTORY(CreateCamerasContextComponentFactory)


	CBaseCamera::CBaseCamera():
		CBaseComponent(),
		mZNear(0.1f),
		mZFar(1000.f),
		mpCameraFrustum(nullptr)
	{
	}

	E_RESULT_CODE CBaseCamera::_onFreeInternal()
	{
		defer([this] 
		{ 
			if (mpCameraFrustum)
			{
				mpCameraFrustum->Free();
			}
		});

		return CBaseComponent::_onFreeInternal();
	}

	void CBaseCamera::SetNearPlane(F32 zn)
	{
		mZNear = zn;
	}

	void CBaseCamera::SetFarPlane(F32 zf)
	{
		mZFar = zf;
	}

	void CBaseCamera::SetProjMatrix(const TMatrix4& projMatrix)
	{
		mProjMatrix = projMatrix;
	}

	void CBaseCamera::SetViewMatrix(const TMatrix4& viewMatrix)
	{
		mViewMatrix = viewMatrix;
	}

	void CBaseCamera::SetViewProjMatrix(const TMatrix4& viewProjMatrix, F32 zNDCMin)
	{
		mViewProjMatrix    = viewProjMatrix;
		mInvViewProjMatrix = Inverse(viewProjMatrix);

		if (!mpCameraFrustum)
		{
			E_RESULT_CODE result = RC_OK;
			mpCameraFrustum = CreateFrustum(result);
		}

		if (mpCameraFrustum)
		{
			TDE2_ASSERT(mpCameraFrustum->ComputeBounds(mInvViewProjMatrix, zNDCMin) == RC_OK);
		}
	}

	void CBaseCamera::SetPosition(const TVector3& position)
	{
		mPosition = position;
	}

	F32 CBaseCamera::GetNearPlane() const
	{
		return mZNear;
	}

	F32 CBaseCamera::GetFarPlane() const
	{
		return mZFar;
	}

	const TMatrix4& CBaseCamera::GetProjMatrix() const
	{
		return mProjMatrix;
	}

	const TMatrix4& CBaseCamera::GetViewMatrix() const
	{
		return mViewMatrix;
	}

	const TMatrix4& CBaseCamera::GetViewProjMatrix() const
	{
		return mViewProjMatrix;
	}

	const TMatrix4& CBaseCamera::GetInverseViewProjMatrix() const
	{
		return mInvViewProjMatrix;
	}

	const TVector3& CBaseCamera::GetPosition() const
	{
		return mPosition;
	}

	IFrustum* CBaseCamera::GetFrustum() const
	{
		return mpCameraFrustum;
	}

	
	/*!
		\brief CFrustum's definition
	*/

	CFrustum::CFrustum() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CFrustum::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CFrustum::ComputeBounds(const TMatrix4& invViewProj, F32 zMin)
	{
		auto&& frustumVertices = GetVertices(invViewProj, zMin);

		TVector3 planePoints[][3]
		{
			{ frustumVertices[0], frustumVertices[1], frustumVertices[2] }, // near plane
			{ frustumVertices[4], frustumVertices[6], frustumVertices[5] }, // far plane
			{ frustumVertices[0], frustumVertices[6], frustumVertices[4] }, // left plane
			{ frustumVertices[1], frustumVertices[5], frustumVertices[7] }, // right plane
			{ frustumVertices[2], frustumVertices[7], frustumVertices[6] }, // bottom plane
			{ frustumVertices[0], frustumVertices[4], frustumVertices[5] }, // top plane
		};

		for (U8 i = 0; i < static_cast<U8>(mPlanes.size()); ++i)
		{
			mPlanes[i] = std::move(TPlaneF32{ planePoints[i] });
		}

		return RC_OK;
	}

	std::array<TVector4, 8> CFrustum::GetVertices(const TMatrix4& invViewProj, F32 zMin) const
	{
		std::array<TVector4, 8> frustumVertices
		{
			invViewProj * TVector4(-1.0f,  1.0f, zMin, 1.0f), // near plane of the cube
			invViewProj * TVector4(1.0f,  1.0f, zMin, 1.0f),
			invViewProj * TVector4(-1.0f, -1.0f, zMin, 1.0f),
			invViewProj * TVector4(1.0f, -1.0f, zMin, 1.0f),

			invViewProj * TVector4(-1.0f,  1.0f, 1.0f, 1.0f),
			invViewProj * TVector4(1.0f,  1.0f, 1.0f, 1.0f),
			invViewProj * TVector4(-1.0f, -1.0f, 1.0f, 1.0f),
			invViewProj * TVector4(1.0f, -1.0f, 1.0f, 1.0f),
		};

		for (TVector4& currVertex : frustumVertices)
		{
			currVertex = currVertex * (1.0f / currVertex.w);
		}

		return std::move(frustumVertices);
	}

	bool CFrustum::TestPoint(const TVector3& point) const
	{
		for (auto&& currPlane : mPlanes)
		{
			if (CalcDistanceFromPlaneToPoint(currPlane, point) < 0.0f)
			{
				return false;
			}
		}

		return true;
	}

	bool CFrustum::TestSphere(const TVector3& center, F32 radius) const
	{
		for (auto&& currPlane : mPlanes)
		{
			if (CalcDistanceFromPlaneToPoint(currPlane, center) < -radius)
			{
				return false;
			}
		}

		return true;
	}

	bool CFrustum::TestAABB(const TAABB& box) const
	{
		TVector3 min = box.min;
		TVector3 max = box.max;

		std::array<TVector3, 8> boxVertices
		{
			min,
			TVector3(min.x, min.y, max.z),
			TVector3(min.x, max.y, min.z),
			TVector3(min.x, max.y, max.z),
			TVector3(max.x, min.y, min.z),
			TVector3(max.x, min.y, max.z),
			TVector3(max.x, max.y, min.z),
			max,
		};

		for (auto&& v : boxVertices)
		{
			if (TestPoint(v))
			{
				return true;
			}
		}

		return false;
	}


	TDE2_API IFrustum* CreateFrustum(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IFrustum, CFrustum, result);
	}


	/*!
		\brief CCamerasContextComponent's definition
	*/


	TDE2_REGISTER_UNIQUE_COMPONENT(CCamerasContextComponent);

	CCamerasContextComponent::CCamerasContextComponent():
		CBaseComponent()
	{
	}

	void CCamerasContextComponent::SetActiveCameraEntity(TEntityId entityId)
	{
		mPrevCameraEntityId = mActiveCameraEntityId;
		mActiveCameraEntityId = entityId;
	}

	E_RESULT_CODE CCamerasContextComponent::RestorePreviousCameraEntity()
	{
		if (TEntityId::Invalid == mPrevCameraEntityId)
		{
			return RC_FAIL;
		}

		mActiveCameraEntityId = mPrevCameraEntityId;
		mPrevCameraEntityId = TEntityId::Invalid;

		return RC_OK;
	}

	TEntityId CCamerasContextComponent::GetActiveCameraEntityId() const
	{
		return mActiveCameraEntityId;
	}

	TDE2_API CCamerasContextComponent* CreateCamerasContextComponent(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(CCamerasContextComponent, CCamerasContextComponent, result);
	}


	/*!
		\brief CCamerasContextCameraFactory's definition
	*/

	CCamerasContextComponentFactory::CCamerasContextComponentFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* CCamerasContextComponentFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return CreateCamerasContextComponent(result);
	}

	E_RESULT_CODE CCamerasContextComponentFactory::SetupComponent(CCamerasContextComponent* pComponent, const TCamerasContextParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}

		pComponent->SetActiveCameraEntity(params.mActiveCameraEntityId);

		return RC_OK;
	}


	IComponentFactory* CreateCamerasContextComponentFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CCamerasContextComponentFactory, result);
	}


#if TDE2_EDITORS_ENABLED
	TDE2_DEFINE_FLAG_COMPONENT(EditorCamera)
#endif

	
	E_RESULT_CODE SetActiveCamera(TPtr<IWorld> pWorld, TEntityId cameraEntityId)
	{
		if (!pWorld || TEntityId::Invalid == cameraEntityId)
		{
			return RC_INVALID_ARGS;
		}

		if (auto pCamerasContextEntity = pWorld->FindEntity(pWorld->FindEntityWithUniqueComponent<CCamerasContextComponent>()))
		{
			if (auto pCamerasContext = pCamerasContextEntity->GetComponent<CCamerasContextComponent>())
			{
				pCamerasContext->SetActiveCameraEntity(cameraEntityId);
			}
		}

		return RC_OK;
	}
}