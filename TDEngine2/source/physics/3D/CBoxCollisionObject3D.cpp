#include "../../../include/physics/3D/CBoxCollisionObject3D.h"
#include "../../../include/physics/3D/ICollisionObjects3DVisitor.h"
#include "../../../deps/bullet3/src/btBulletDynamicsCommon.h"


namespace TDEngine2
{
	CBoxCollisionObject3D::CBoxCollisionObject3D() :
		CBaseCollisionObject3D(), mExtents(1.0f)
	{
	}

	E_RESULT_CODE CBoxCollisionObject3D::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}

		pReader->BeginGroup("extents");
		{
			if (auto extentsResult = LoadVector3(pReader))
			{
				mExtents = extentsResult.Get();
			}
		}
		pReader->EndGroup();

		return RC_OK;
	}

	E_RESULT_CODE CBoxCollisionObject3D::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		pWriter->BeginGroup("component");
		{
			pWriter->SetUInt32("type_id", static_cast<U32>(CBoxCollisionObject3D::GetTypeId()));
			pWriter->BeginGroup("extents");
			{
				SaveVector3(pWriter, mExtents);
			}
			pWriter->EndGroup();
		}
		pWriter->EndGroup();

		return RC_OK;
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


	/*!
		\brief CBoxCollisionObject3DFactory's definition
	*/

	CBoxCollisionObject3DFactory::CBoxCollisionObject3DFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* CBoxCollisionObject3DFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return CreateBoxCollisionObject3D(result);
	}

	E_RESULT_CODE CBoxCollisionObject3DFactory::SetupComponent(CBoxCollisionObject3D* pComponent, const TBoxCollisionObject3DParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}

		pComponent->SetSizes(params.mExtents);

		return RC_OK;
	}


	IComponentFactory* CreateBoxCollisionObject3DFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CBoxCollisionObject3DFactory, result);
	}
}