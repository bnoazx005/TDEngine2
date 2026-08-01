#include "../include/CBoxCollisionObject3D.h"
#include "../deps/bullet3/src/btBulletDynamicsCommon.h"
#define META_EXPORT_BULLET_ECS_PLUGIN_SECTION
#include "../../include/metadata.h"


namespace TDEngine2
{
	TDE2_DEFINE_COMPONENT_META(TBoxCollisionObject3DComponentData)


	CBoxCollisionObject3D::CBoxCollisionObject3D() :
		CBaseComponentT()
	{
	}

	void CBoxCollisionObject3D::SetSizes(const TVector3& extents)
	{
		mData.mExtents = extents;
	}

	const TVector3& CBoxCollisionObject3D::GetSizes() const
	{
		return mData.mExtents;
	}

	const std::string& CBoxCollisionObject3D::GetTypeName() const
	{
		static std::string typeName{ "box_collision_3d" };
		return typeName;
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