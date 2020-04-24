#include "./../../include/graphics/CBaseCamera.h"
#include <array>


namespace TDEngine2
{
	CBaseCamera::CBaseCamera():
		CBaseComponent()
	{
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

	void CBaseCamera::SetViewProjMatrix(const TMatrix4& viewProjMatrix)
	{
		mViewProjMatrix    = viewProjMatrix;
		mInvViewProjMatrix = Inverse(viewProjMatrix);
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
			invViewProj * TVector4(-1.0f,  1.0f, zMin, 1.0f),

			invViewProj * TVector4(-1.0f,  1.0f, 1.0f, 1.0f),
			invViewProj * TVector4(1.0f,  1.0f, 1.0f, 1.0f),
			invViewProj * TVector4(-1.0f, -1.0f, 1.0f, 1.0f),
			invViewProj * TVector4(-1.0f,  1.0f, 1.0f, 1.0f),
		};

		for (TVector4& currVertex : frustumVertices)
		{
			currVertex = currVertex * (1.0f / currVertex.w);
		}

		return RC_OK;
	}


	TDE2_API IFrustum* CreateFrustum(E_RESULT_CODE& result)
	{
		CFrustum* pGeometryBuilder = new (std::nothrow) CFrustum();

		if (!pGeometryBuilder)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pGeometryBuilder->Init();

		if (result != RC_OK)
		{
			delete pGeometryBuilder;

			pGeometryBuilder = nullptr;
		}

		return pGeometryBuilder;
	}
}