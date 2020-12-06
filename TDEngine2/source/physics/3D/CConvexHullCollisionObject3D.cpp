#include "../../../include/physics/3D/CConvexHullCollisionObject3D.h"
#include "../../../include/physics/3D/ICollisionObjects3DVisitor.h"
#include "../../../deps/bullet3/src/btBulletDynamicsCommon.h"


namespace TDEngine2
{
	CConvexHullCollisionObject3D::CConvexHullCollisionObject3D() :
		CBaseCollisionObject3D()
	{
	}

	E_RESULT_CODE CConvexHullCollisionObject3D::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}
/*
		pReader->BeginGroup("extents");
		{
			if (auto extentsResult = LoadVector3(pReader))
			{
				mExtents = extentsResult.Get();
			}
		}
		pReader->EndGroup();*/

		return RC_OK;
	}

	E_RESULT_CODE CConvexHullCollisionObject3D::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}
/*
		pWriter->BeginGroup("component");
		{
			pWriter->SetUInt32("type_id", static_cast<U32>(CConvexHullCollisionObject3D::GetTypeId()));
			pWriter->BeginGroup("extents");
			{
				SaveVector3(pWriter, mExtents);
			}
			pWriter->EndGroup();
		}
		pWriter->EndGroup();*/

		return RC_OK;
	}

	E_RESULT_CODE CConvexHullCollisionObject3D::SetVertices(const std::vector<TVector4>& vertices)
	{
		mpVertices = &vertices;
		return RC_OK;
	}

	const std::vector<TVector4>& CConvexHullCollisionObject3D::GetVertices() const
	{
		return *mpVertices;
	}

	btCollisionShape* CConvexHullCollisionObject3D::GetCollisionShape(const ICollisionObjects3DVisitor* pVisitor) const
	{
		return dynamic_cast<btCollisionShape*>(pVisitor->CreateConvexHullCollisionShape(*this));
	}


	IComponent* CreateConvexHullCollisionObject3D(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CConvexHullCollisionObject3D, result);
	}


	CConvexHullCollisionObject3DFactory::CConvexHullCollisionObject3DFactory() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CConvexHullCollisionObject3DFactory::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CConvexHullCollisionObject3DFactory::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	IComponent* CConvexHullCollisionObject3DFactory::Create(const TBaseComponentParameters* pParams) const
	{
		if (!pParams)
		{
			return nullptr;
		}

		const TConvexHullCollisionObject3DParameters* box3DCollisionParams = static_cast<const TConvexHullCollisionObject3DParameters*>(pParams);

		E_RESULT_CODE result = RC_OK;

		if (CConvexHullCollisionObject3D* pConvexHullCollisionObject = dynamic_cast<CConvexHullCollisionObject3D*>(CreateConvexHullCollisionObject3D(result)))
		{
		//	pConvexHullCollisionObject->SetSizes(box3DCollisionParams->mExtents);

			return pConvexHullCollisionObject;
		}

		return nullptr;
	}

	IComponent* CConvexHullCollisionObject3DFactory::CreateDefault(const TBaseComponentParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		return CreateConvexHullCollisionObject3D(result);
	}

	TypeId CConvexHullCollisionObject3DFactory::GetComponentTypeId() const
	{
		return CConvexHullCollisionObject3D::GetTypeId();
	}


	IComponentFactory* CreateConvexHullCollisionObject3DFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CConvexHullCollisionObject3DFactory, result);
	}
}