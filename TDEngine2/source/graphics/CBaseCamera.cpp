#include "../../include/graphics/CBaseCamera.h"
#include "../../include/math/TAABB.h"
#include "../../include/utils/Utils.h"
#include <array>


namespace TDEngine2
{
	CBaseCamera::CBaseCamera():
		CBaseComponent()
	{
	}

	E_RESULT_CODE CBaseCamera::Free()
	{
		CDeferOperation releaseFrustumObject([this] 
		{ 
			if (mpCameraFrustum)
			{
				mpCameraFrustum->Free();
			}
		});

		return CBaseComponent::Free();
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

		if (mpCameraFrustum)
		{
			TDE2_ASSERT(mpCameraFrustum->ComputeBounds(mInvViewProjMatrix, zNDCMin) == RC_OK);
		}
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

	IFrustum* CBaseCamera::GetFrustum() const
	{
		return mpCameraFrustum;
	}

	E_RESULT_CODE CBaseCamera::_initInternal()
	{
		E_RESULT_CODE result = RC_OK;

		mpCameraFrustum = CreateFrustum(result);

		if (result != RC_OK)
		{
			mIsInitialized = false;
			return result;
		}

		return RC_OK;
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

	E_RESULT_CODE CFrustum::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;
		delete this;

		return RC_OK;
	}

	E_RESULT_CODE CFrustum::ComputeBounds(const TMatrix4& invViewProj, F32 zMin)
	{
		std::array<TVector4, 8> frustumVertices
		{
			invViewProj * TVector4(-1.0f,  1.0f, zMin, 1.0f), // near plane of the cube
			invViewProj * TVector4( 1.0f,  1.0f, zMin, 1.0f),
			invViewProj * TVector4(-1.0f, -1.0f, zMin, 1.0f),
			invViewProj * TVector4( 1.0f, -1.0f, zMin, 1.0f),

			invViewProj * TVector4(-1.0f,  1.0f, 1.0f, 1.0f),
			invViewProj * TVector4(1.0f,  1.0f, 1.0f, 1.0f),
			invViewProj * TVector4(-1.0f, -1.0f, 1.0f, 1.0f),
			invViewProj * TVector4( 1.0f, -1.0f, 1.0f, 1.0f),
		};

		for (TVector4& currVertex : frustumVertices)
		{
			currVertex = currVertex * (1.0f / currVertex.w);
		}

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
}