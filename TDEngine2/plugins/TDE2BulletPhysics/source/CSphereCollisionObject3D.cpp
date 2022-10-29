#include "../include/CSphereCollisionObject3D.h"
#include "../include/ICollisionObjects3DVisitor.h"
#include "../../../deps/bullet3/src/btBulletDynamicsCommon.h"


namespace TDEngine2
{
	CSphereCollisionObject3D::CSphereCollisionObject3D() :
		CBaseCollisionObject3D(), mRadius(1.0f)
	{
	}

	E_RESULT_CODE CSphereCollisionObject3D::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}

		mRadius = pReader->GetFloat("radius");

		return RC_OK;
	}

	E_RESULT_CODE CSphereCollisionObject3D::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		pWriter->BeginGroup("component");
		{
			pWriter->SetUInt32("type_id", static_cast<U32>(CSphereCollisionObject3D::GetTypeId()));
			pWriter->SetFloat("radius", mRadius);
		}
		pWriter->EndGroup();

		return RC_OK;
	}

	E_RESULT_CODE CSphereCollisionObject3D::Clone(IComponent*& pDestObject) const
	{
		if (auto pComponent = dynamic_cast<CSphereCollisionObject3D*>(pDestObject))
		{
			pComponent->mRadius = mRadius;
			pComponent->mType = mType;
			pComponent->mMass = mMass;

			pComponent->mHasChanged = true;

			return RC_OK;
		}

		return RC_FAIL;
	}

	void CSphereCollisionObject3D::SetRadius(F32 radius)
	{
		mRadius = radius;
	}

	F32 CSphereCollisionObject3D::GetRadius() const
	{
		return mRadius;
	}

	btCollisionShape* CSphereCollisionObject3D::GetCollisionShape(const ICollisionObjects3DVisitor* pVisitor) const
	{
		return dynamic_cast<btCollisionShape*>(pVisitor->CreateSphereCollisionShape(*this));
	}


	IComponent* CreateSphereCollisionObject3D(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CSphereCollisionObject3D, result);
	}


	/*!
		\brief CSphereCollisionObject3DFactory's definition
	*/

	CSphereCollisionObject3DFactory::CSphereCollisionObject3DFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* CSphereCollisionObject3DFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return CreateSphereCollisionObject3D(result);
	}

	E_RESULT_CODE CSphereCollisionObject3DFactory::SetupComponent(CSphereCollisionObject3D* pComponent, const TSphereCollisionObject3DParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}

		pComponent->SetRadius(params.mRadius);

		return RC_OK;
	}


	IComponentFactory* CreateSphereCollisionObject3DFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CSphereCollisionObject3DFactory, result);
	}
}