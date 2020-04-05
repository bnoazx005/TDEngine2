#include "./../../include/graphics/CBaseCamera.h"


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
}