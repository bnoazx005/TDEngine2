#include "../../include/graphics/CCamera.h"
#include "../../include/math/TAABB.h"
#include "../../include/utils/Utils.h"
#include "../../include/ecs/CComponentManager.h"
#include "../../include/ecs/IWorld.h"
#include <array>
#include "deferOperation.hpp"
#define META_EXPORT_ECS_SECTION
#include "../../../include/metadata.h"


namespace TDEngine2
{
	TDE2_REGISTER_COMPONENT_FACTORY(CreateCameraFactory)
	TDE2_REGISTER_COMPONENT_FACTORY(CreateCamerasContextComponentFactory)
	TDE2_DEFINE_COMPONENT_META(TCameraComponentData);
	TDE2_DEFINE_COMPONENT_META(TCamerasContextComponentData);


	CCamera::CCamera():
		CBaseComponentT()
	{
	}

	void CCamera::SetViewProjMatrix(const TMatrix4& viewProjMatrix, F32 zNDCMin)
	{
		mData.mViewProjMatrix    = viewProjMatrix;
		mData.mInvViewProjMatrix = Inverse(viewProjMatrix);

		if (!mpCameraFrustum)
		{
			E_RESULT_CODE result = RC_OK;
			mpCameraFrustum = TPtr<IFrustum>(CreateFrustum(result));
		}

		if (mpCameraFrustum)
		{
			TDE2_ASSERT(mpCameraFrustum->ComputeBounds(mData.mInvViewProjMatrix, zNDCMin) == RC_OK);
		}
	}

	E_RESULT_CODE CCamera::Clone(IComponent*& pDestObject) const
	{
		if (CCamera* pDestComponentPtr = dynamic_cast<CCamera*>(pDestObject))
		{
			pDestComponentPtr->mData           = mData;
			pDestComponentPtr->mpCameraFrustum = mpCameraFrustum;

			return RC_OK;
		}

		return RC_FAIL;
	}

	F32 CCamera::GetNearPlane() const
	{
		return mData.mZNear;
	}

	F32 CCamera::GetFarPlane() const
	{
		return mData.mZFar;
	}


	F32 CCamera::GetWidth() const
	{
		TDE2_ASSERT(mData.mType == E_CAMERA_PROJECTION_TYPE::ORTHOGRAPHIC);
		return mData.mParams.x;
	}

	F32 CCamera::GetHeight() const
	{
		TDE2_ASSERT(mData.mType == E_CAMERA_PROJECTION_TYPE::ORTHOGRAPHIC);
		return mData.mParams.y;
	}

	F32 CCamera::GetFOV() const
	{
		TDE2_ASSERT(mData.mType == E_CAMERA_PROJECTION_TYPE::PERSPECTIVE);
		return mData.mParams.x;
	}

	F32 CCamera::GetAspect() const
	{
		TDE2_ASSERT(mData.mType == E_CAMERA_PROJECTION_TYPE::PERSPECTIVE);
		return mData.mParams.y;
	}

	const TMatrix4& CCamera::GetProjMatrix() const
	{
		return mData.mProjMatrix;
	}

	const TMatrix4& CCamera::GetViewMatrix() const
	{
		return mData.mViewMatrix;
	}

	const TMatrix4& CCamera::GetViewProjMatrix() const
	{
		return mData.mViewProjMatrix;
	}

	const TMatrix4& CCamera::GetInverseViewProjMatrix() const
	{
		return mData.mInvViewProjMatrix;
	}

	TPtr<IFrustum> CCamera::GetFrustum() const
	{
		return mpCameraFrustum;
	}

	const TVector3& CCamera::GetPosition() const
	{
		return mData.mPosition;
	}

	const std::string& CCamera::GetTypeName() const
	{
		static const std::string id{ "camera" };
		return id;
	}


	IComponent* CreateCamera(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CCamera, result);
	}


	/*!
		\brief CCameraFactory's definition
	*/

	CCameraFactory::CCameraFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* CCameraFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return CreateCamera(result);
	}

	E_RESULT_CODE CCameraFactory::SetupComponent(CCamera* pComponent, const TCameraParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}

		return RC_OK;
	}


	IComponentFactory* CreateCameraFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CCameraFactory, result);
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

		return frustumVertices;
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

	// Based on https://gdbooks.gitbooks.io/3dcollisions/content/Chapter2/static_aabb_plane.html
	static inline bool IsAABBInsidePlane(const TVector3& center, const TVector3& extents, const TPlaneF32& plane)
	{
		const F32 radius = extents.x * std::abs(plane.a) + extents.y * std::abs(plane.b) + extents.z * std::abs(plane.c);
		return CalcDistanceFromPlaneToPoint(plane, center) >= -radius;
	}


	bool CFrustum::TestAABB(const TAABB& box) const
	{
		TVector3 min = box.min;
		TVector3 max = box.max;

		const TVector3 center = 0.5f * (min + max);
		const TVector3 extents = max - center;

		for (auto&& currPlane : mPlanes)
		{
			if (!IsAABBInsidePlane(center, extents, currPlane))
			{
				return false;
			}
		}

		return true;
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
		CBaseComponentT()
	{
	}

	void CCamerasContextComponent::SetActiveCameraEntity(TEntityId entityId)
	{
		mData.mPrevCameraEntityId = mData.mActiveCameraEntityId;
		mData.mActiveCameraEntityId = entityId;
	}

	E_RESULT_CODE CCamerasContextComponent::RestorePreviousCameraEntity()
	{
		if (TEntityId::Invalid == mData.mPrevCameraEntityId)
		{
			return RC_FAIL;
		}

		mData.mActiveCameraEntityId = mData.mPrevCameraEntityId;
		mData.mPrevCameraEntityId = TEntityId::Invalid;

		return RC_OK;
	}

	TEntityId CCamerasContextComponent::GetActiveCameraEntityId() const
	{
		return mData.mActiveCameraEntityId;
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