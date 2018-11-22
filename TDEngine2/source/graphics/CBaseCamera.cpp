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

	F32 CBaseCamera::GetNearPlane() const
	{
		return mZNear;
	}

	F32 CBaseCamera::GetFarPlane() const
	{
		return mZFar;
	}
}