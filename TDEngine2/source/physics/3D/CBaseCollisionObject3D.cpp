#include "./../../../include/physics/3D/CBaseCollisionObject3D.h"
#include "./../../../include/math/MathUtils.h"


namespace TDEngine2
{
	CBaseCollisionObject3D::CBaseCollisionObject3D() :
		CBaseComponent(), mType(E_COLLISION_OBJECT_TYPE::COT_DYNAMIC), mMass(1.0f)
	{
	}

	E_RESULT_CODE CBaseCollisionObject3D::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	void CBaseCollisionObject3D::Reset()
	{
		mHasChanged = true;
	}

	bool CBaseCollisionObject3D::HasChanged() const
	{
		return mHasChanged;
	}

	void CBaseCollisionObject3D::SetCollisionType(E_COLLISION_OBJECT_TYPE type)
	{
		mType = type;
	}

	void CBaseCollisionObject3D::SetMass(F32 mass)
	{
		mMass = CMathUtils::Max(0.0f, mass);
	}

	E_COLLISION_OBJECT_TYPE CBaseCollisionObject3D::GetCollisionType() const
	{
		return mType;
	}

	F32 CBaseCollisionObject3D::GetMass() const
	{
		return mMass;
	}
}