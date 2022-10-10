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

	E_RESULT_CODE CConvexHullCollisionObject3D::Clone(IComponent*& pDestObject) const
	{
		if (auto pComponent = dynamic_cast<CConvexHullCollisionObject3D*>(pDestObject))
		{
			pComponent->mType = mType;
			pComponent->mMass = mMass;

			pComponent->mHasChanged = true;

			return RC_OK;
		}

		return RC_FAIL;
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


	/*!
		\brief CConvexHullCollisionObject3DFactory's definition
	*/

	CConvexHullCollisionObject3DFactory::CConvexHullCollisionObject3DFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* CConvexHullCollisionObject3DFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return CreateConvexHullCollisionObject3D(result);
	}

	E_RESULT_CODE CConvexHullCollisionObject3DFactory::SetupComponent(CConvexHullCollisionObject3D* pComponent, const TConvexHullCollisionObject3DParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}

		return RC_OK;
	}


	IComponentFactory* CreateConvexHullCollisionObject3DFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CConvexHullCollisionObject3DFactory, result);
	}
}