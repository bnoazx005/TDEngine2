#include "../include/CCapsuleCollisionObject3D.h"
#define META_EXPORT_BULLET_ECS_PLUGIN_SECTION
#include "../../include/metadata.h"


namespace TDEngine2
{
	TDE2_DEFINE_COMPONENT_META(TCapsuleCollisionObject3DComponentData)


	CCapsuleCollisionObject3D::CCapsuleCollisionObject3D() :
		CBaseComponentT()
	{
	}

	E_RESULT_CODE CCapsuleCollisionObject3D::SetRadius(F32 radius)
	{
		if (radius < 0.0f)
		{
			return RC_INVALID_ARGS;
		}

		mData.mRadius = radius;
		
		return RC_OK;
	}

	E_RESULT_CODE CCapsuleCollisionObject3D::SetHeight(F32 height)
	{
		if (height < 0.0f)
		{
			return RC_INVALID_ARGS;
		}

		mData.mHeight = height;

		return RC_OK;
	}

	F32 CCapsuleCollisionObject3D::GetRadius() const
	{
		return mData.mRadius;
	}

	F32 CCapsuleCollisionObject3D::GetHeight() const
	{
		return mData.mHeight;
	}

	const std::string& CCapsuleCollisionObject3D::GetTypeName() const
	{
		static std::string typeName{ "capsule_collision_3d" };
		return typeName;
	}


	IComponent* CreateCapsuleCollisionObject3D(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CCapsuleCollisionObject3D, result);
	}


	/*!
		\brief CCapsuleCollisionObject3DFactory's definition
	*/

	CCapsuleCollisionObject3DFactory::CCapsuleCollisionObject3DFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* CCapsuleCollisionObject3DFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return CreateCapsuleCollisionObject3D(result);
	}

	E_RESULT_CODE CCapsuleCollisionObject3DFactory::SetupComponent(CCapsuleCollisionObject3D* pComponent, const TCapsuleCollisionObject3DParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}

		pComponent->SetRadius(params.mRadius);
		pComponent->SetHeight(params.mHeight);

		return RC_OK;
	}


	IComponentFactory* CreateCapsuleCollisionObject3DFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CCapsuleCollisionObject3DFactory, result);
	}
}