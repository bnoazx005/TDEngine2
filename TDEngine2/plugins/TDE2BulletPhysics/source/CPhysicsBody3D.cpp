#include "../include/CPhysicsBody3D.h"
#define META_EXPORT_BULLET_ECS_PLUGIN_SECTION
#include "../../include/metadata.h"


namespace TDEngine2
{
	TDE2_DEFINE_COMPONENT_META(TPhysicsBody3DComponentData)


	CPhysicsBody3D::CPhysicsBody3D() :
		CBaseComponentT()
	{
	}

	void CPhysicsBody3D::SetCollisionType(E_COLLISION_OBJECT_TYPE type)
	{
		mData.mType = type;
		mData.mHasChanged = true;
	}

	void CPhysicsBody3D::SetMass(F32 mass)
	{
		mData.mMass = mass;
		mData.mHasChanged = true;
	}

	E_COLLISION_OBJECT_TYPE CPhysicsBody3D::GetCollisionType() const
	{
		return mData.mType;
	}

	F32 CPhysicsBody3D::GetMass() const
	{
		return mData.mMass;
	}

	const std::string& CPhysicsBody3D::GetTypeName() const
	{
		static const std::string id{ "physics_body_3d" };
		return id;
	}


	IComponent* CreatePhysicsBody3D(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CPhysicsBody3D, result);
	}


	/*!
		\brief CPhysicsBody3DFactory's definition
	*/

	CPhysicsBody3DFactory::CPhysicsBody3DFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* CPhysicsBody3DFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return CreatePhysicsBody3D(result);
	}

	E_RESULT_CODE CPhysicsBody3DFactory::SetupComponent(CPhysicsBody3D* pComponent, const TPhysicsBody3DParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}

		//TPhysicsBody3DComponentData& physics3dBodyData = pComponent->GetData();
		//physics3dBodyData.mMass = params.

		return RC_OK;
	}


	IComponentFactory* CreatePhysicsBody3DFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CPhysicsBody3DFactory, result);
	}
}