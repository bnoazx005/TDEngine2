#include "../../../include/physics/3D/CCapsuleCollisionObject3D.h"
#include "../../../include/physics/3D/ICollisionObjects3DVisitor.h"
#include "../../../deps/bullet3/src/btBulletDynamicsCommon.h"


namespace TDEngine2
{
	struct TCapsuleCollisionObjectArchiveKeys
	{
		static const std::string mRadiusKeyId;
		static const std::string mHeightKeyId;
	};


	const std::string TCapsuleCollisionObjectArchiveKeys::mRadiusKeyId = "radius";
	const std::string TCapsuleCollisionObjectArchiveKeys::mHeightKeyId = "height";


	CCapsuleCollisionObject3D::CCapsuleCollisionObject3D() :
		CBaseCollisionObject3D(), mRadius(1.0f)
	{
	}

	E_RESULT_CODE CCapsuleCollisionObject3D::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}

		mRadius = pReader->GetFloat(TCapsuleCollisionObjectArchiveKeys::mRadiusKeyId);
		mHeight = pReader->GetFloat(TCapsuleCollisionObjectArchiveKeys::mHeightKeyId);

		return RC_OK;
	}

	E_RESULT_CODE CCapsuleCollisionObject3D::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		pWriter->BeginGroup("component");
		{
			pWriter->SetUInt32("type_id", static_cast<U32>(CCapsuleCollisionObject3D::GetTypeId()));

			pWriter->SetFloat(TCapsuleCollisionObjectArchiveKeys::mRadiusKeyId, mRadius);
			pWriter->SetFloat(TCapsuleCollisionObjectArchiveKeys::mHeightKeyId, mHeight);
		}
		pWriter->EndGroup();

		return RC_OK;
	}

	E_RESULT_CODE CCapsuleCollisionObject3D::Clone(IComponent*& pDestObject) const
	{
		if (auto pComponent = dynamic_cast<CCapsuleCollisionObject3D*>(pDestObject))
		{
			pComponent->mRadius = mRadius;
			pComponent->mHeight = mHeight;

			pComponent->mType = mType;
			pComponent->mMass = mMass;

			pComponent->mHasChanged = true;

			return RC_OK;
		}

		return RC_FAIL;
	}

	E_RESULT_CODE CCapsuleCollisionObject3D::SetRadius(F32 radius)
	{
		if (radius < 0.0f)
		{
			return RC_INVALID_ARGS;
		}

		mRadius = radius;
		
		return RC_OK;
	}

	E_RESULT_CODE CCapsuleCollisionObject3D::SetHeight(F32 height)
	{
		if (height < 0.0f)
		{
			return RC_INVALID_ARGS;
		}

		mHeight = height;

		return RC_OK;
	}

	F32 CCapsuleCollisionObject3D::GetRadius() const
	{
		return mRadius;
	}

	F32 CCapsuleCollisionObject3D::GetHeight() const
	{
		return mHeight;
	}

	btCollisionShape* CCapsuleCollisionObject3D::GetCollisionShape(const ICollisionObjects3DVisitor* pVisitor) const
	{
		return dynamic_cast<btCollisionShape*>(pVisitor->CreateCapsuleCollisionShape(*this));
	}


	IComponent* CreateCapsuleCollisionObject3D(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CCapsuleCollisionObject3D, result);
	}


	/*!
		\brief CCapsuleCollisionObject3DFactory's definition
	*/

	CCapsuleCollisionObject3DFactory::CCapsuleCollisionObject3DFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* CCapsuleCollisionObject3DFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return CreateCapsuleCollisionObject3D(result);
	}

	E_RESULT_CODE CCapsuleCollisionObject3DFactory::SetupComponent(CCapsuleCollisionObject3D* pComponent, const TCapsuleCollisionObject3DParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}

		pComponent->SetRadius(params.mRadius);
		pComponent->SetHeight(params.mHeight);

		return RC_OK;
	}


	IComponentFactory* CreateCapsuleCollisionObject3DFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CCapsuleCollisionObject3DFactory, result);
	}
}