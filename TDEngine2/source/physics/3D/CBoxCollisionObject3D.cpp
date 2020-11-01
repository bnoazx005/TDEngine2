#include "../../../include/physics/3D/CBoxCollisionObject3D.h"
#include "../../../include/physics/3D/ICollisionObjects3DVisitor.h"
#include "../../../deps/bullet3/src/btBulletDynamicsCommon.h"


namespace TDEngine2
{
	CBoxCollisionObject3D::CBoxCollisionObject3D() :
		CBaseCollisionObject3D(), mExtents(1.0f)
	{
	}

	void CBoxCollisionObject3D::SetSizes(const TVector3& extents)
	{
		mExtents = extents;
	}

	const TVector3& CBoxCollisionObject3D::GetSizes() const
	{
		return mExtents;
	}

	btCollisionShape* CBoxCollisionObject3D::GetCollisionShape(const ICollisionObjects3DVisitor* pVisitor) const
	{
		return dynamic_cast<btCollisionShape*>(pVisitor->CreateBoxCollisionShape(*this));
	}


	IComponent* CreateBoxCollisionObject3D(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CBoxCollisionObject3D, result);
	}


	CBoxCollisionObject3DFactory::CBoxCollisionObject3DFactory() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CBoxCollisionObject3DFactory::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CBoxCollisionObject3DFactory::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	IComponent* CBoxCollisionObject3DFactory::Create(const TBaseComponentParameters* pParams) const
	{
		if (!pParams)
		{
			return nullptr;
		}

		const TBoxCollisionObject3DParameters* box3DCollisionParams = static_cast<const TBoxCollisionObject3DParameters*>(pParams);

		E_RESULT_CODE result = RC_OK;

		if (CBoxCollisionObject3D* pBoxCollisionObject = dynamic_cast<CBoxCollisionObject3D*>(CreateBoxCollisionObject3D(result)))
		{
			pBoxCollisionObject->SetSizes(box3DCollisionParams->mExtents);

			return pBoxCollisionObject;
		}

		return nullptr;
	}

	IComponent* CBoxCollisionObject3DFactory::CreateDefault(const TBaseComponentParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		return CreateBoxCollisionObject3D(result);
	}

	TypeId CBoxCollisionObject3DFactory::GetComponentTypeId() const
	{
		return CBoxCollisionObject3D::GetTypeId();
	}


	IComponentFactory* CreateBoxCollisionObject3DFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CBoxCollisionObject3DFactory, result);
	}
}