#include "./../../../include/physics/2D/CBaseCollisionObject2D.h"


namespace TDEngine2
{
	CBaseCollisionObject2D::CBaseCollisionObject2D() :
		CBaseComponent(), mType(E_COLLISION_OBJECT_TYPE::COT_DYNAMIC), mMass(1.0f)
	{
	}

	E_RESULT_CODE CBaseCollisionObject2D::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	void CBaseCollisionObject2D::Reset()
	{
		mHasChanged = true;
	}

	bool CBaseCollisionObject2D::HasChanged() const
	{
		return mHasChanged;
	}
	
	void CBaseCollisionObject2D::SetCollisionType(E_COLLISION_OBJECT_TYPE type)
	{
		mType = type;
	}

	void CBaseCollisionObject2D::SetMass(F32 mass)
	{
		mMass = mass < 0.0f ? 1.0f : mass;
	}

	E_COLLISION_OBJECT_TYPE CBaseCollisionObject2D::GetCollisionType() const
	{
		return mType;
	}

	F32 CBaseCollisionObject2D::GetMass() const
	{
		return mMass;
	}
}