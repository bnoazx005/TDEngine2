#include "../include/CBaseCollisionObject3D.h"
#include <math/MathUtils.h>
#include <metadata.h>


namespace TDEngine2
{
	struct TBaseCollisionObjectArchiveKeys
	{
		static const std::string mMassKeyId;
		static const std::string mTypeKeyId;
	};

	const std::string TBaseCollisionObjectArchiveKeys::mMassKeyId = "mass";
	const std::string TBaseCollisionObjectArchiveKeys::mTypeKeyId = "type";


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
	
	E_RESULT_CODE CBaseCollisionObject3D::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_INVALID_ARGS;
		}

		mMass = pReader->GetFloat(TBaseCollisionObjectArchiveKeys::mMassKeyId);
		mType = Meta::EnumTrait<E_COLLISION_OBJECT_TYPE>::FromString(pReader->GetString(TBaseCollisionObjectArchiveKeys::mTypeKeyId));

		return RC_OK;
	}

	E_RESULT_CODE CBaseCollisionObject3D::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_INVALID_ARGS;
		}

		pWriter->SetFloat(TBaseCollisionObjectArchiveKeys::mMassKeyId, mMass);
		pWriter->SetString(TBaseCollisionObjectArchiveKeys::mTypeKeyId, Meta::EnumTrait<E_COLLISION_OBJECT_TYPE>::ToString(mType));

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