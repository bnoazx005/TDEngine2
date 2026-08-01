#include "../include/CSphereCollisionObject3D.h"
#include "../deps/bullet3/src/btBulletDynamicsCommon.h"
#define META_EXPORT_BULLET_ECS_PLUGIN_SECTION
#include "../../include/metadata.h"


namespace TDEngine2
{
	TDE2_DEFINE_COMPONENT_META(TSphereCollisionObject3DComponentData)


	CSphereCollisionObject3D::CSphereCollisionObject3D() :
		CBaseComponentT()
	{
	}

	void CSphereCollisionObject3D::SetRadius(F32 radius)
	{
		mData.mRadius = radius;
	}

	F32 CSphereCollisionObject3D::GetRadius() const
	{
		return mData.mRadius;
	}

	const std::string& CSphereCollisionObject3D::GetTypeName() const
	{
		static std::string typeName{ "sphere_collision_3d" };
		return typeName;
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