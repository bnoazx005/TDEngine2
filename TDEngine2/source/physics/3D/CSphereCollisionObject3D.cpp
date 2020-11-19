#include "../../../include/physics/3D/CSphereCollisionObject3D.h"
#include "../../../include/physics/3D/ICollisionObjects3DVisitor.h"
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


	CSphereCollisionObject3DFactory::CSphereCollisionObject3DFactory() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CSphereCollisionObject3DFactory::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CSphereCollisionObject3DFactory::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	IComponent* CSphereCollisionObject3DFactory::Create(const TBaseComponentParameters* pParams) const
	{
		if (!pParams)
		{
			return nullptr;
		}

		const TSphereCollisionObject3DParameters* sphereCollisionParams = static_cast<const TSphereCollisionObject3DParameters*>(pParams);

		E_RESULT_CODE result = RC_OK;

		if (CSphereCollisionObject3D* pSphereCollisionObject = dynamic_cast<CSphereCollisionObject3D*>(CreateSphereCollisionObject3D(result)))
		{
			pSphereCollisionObject->SetRadius(sphereCollisionParams->mRadius);

			return pSphereCollisionObject;
		}

		return nullptr;
	}

	IComponent* CSphereCollisionObject3DFactory::CreateDefault(const TBaseComponentParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		return CreateSphereCollisionObject3D(result);
	}

	TypeId CSphereCollisionObject3DFactory::GetComponentTypeId() const
	{
		return CSphereCollisionObject3D::GetTypeId();
	}


	IComponentFactory* CreateSphereCollisionObject3DFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CSphereCollisionObject3DFactory, result);
	}
}