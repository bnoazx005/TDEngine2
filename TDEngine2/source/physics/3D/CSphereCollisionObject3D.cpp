#include "./../../../include/physics/3D/CSphereCollisionObject3D.h"
#include "./../../../include/physics/3D/ICollisionObjects3DVisitor.h"
#include "./../../../deps/bullet3/src/btBulletDynamicsCommon.h"


namespace TDEngine2
{
	CSphereCollisionObject3D::CSphereCollisionObject3D() :
		CBaseCollisionObject3D(), mRadius(1.0f)
	{
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
		CSphereCollisionObject3D* pSphereCollisionObject3DInstance = new (std::nothrow) CSphereCollisionObject3D();

		if (!pSphereCollisionObject3DInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pSphereCollisionObject3DInstance->Init();

		if (result != RC_OK)
		{
			delete pSphereCollisionObject3DInstance;

			pSphereCollisionObject3DInstance = nullptr;
		}

		return pSphereCollisionObject3DInstance;
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
		CSphereCollisionObject3DFactory* pSphereCollisionObject3DFactoryInstance = new (std::nothrow) CSphereCollisionObject3DFactory();

		if (!pSphereCollisionObject3DFactoryInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pSphereCollisionObject3DFactoryInstance->Init();

		if (result != RC_OK)
		{
			delete pSphereCollisionObject3DFactoryInstance;

			pSphereCollisionObject3DFactoryInstance = nullptr;
		}

		return pSphereCollisionObject3DFactoryInstance;
	}
}