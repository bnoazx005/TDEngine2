#include "../../../include/physics/2D/CPhysicsBody2D.h"
#define META_EXPORT_ECS_SECTION
#include "../../include/metadata.h"


namespace TDEngine2
{
	TDE2_REGISTER_COMPONENT_FACTORY(CreatePhysicsBody2DFactory)
	TDE2_DEFINE_COMPONENT_META(TPhysicsBody2DComponentData)


	CPhysicsBody2D::CPhysicsBody2D() :
		CBaseComponentT()
	{
	}

	const std::string& CPhysicsBody2D::GetTypeName() const
	{
		static const std::string id{ "physics_body_2d" };
		return id;
	}


	IComponent* CreatePhysicsBody2D(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CPhysicsBody2D, result);
	}


	/*!
		\briefC PhysicsBody2DFactory's definition
	*/

	CPhysicsBody2DFactory::CPhysicsBody2DFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* CPhysicsBody2DFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return CreatePhysicsBody2D(result);
	}

	E_RESULT_CODE CPhysicsBody2DFactory::SetupComponent(CPhysicsBody2D* pComponent, const TPhysicsBody2DParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}

		return RC_OK;
	}


	IComponentFactory* CreatePhysicsBody2DFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CPhysicsBody2DFactory, result);
	}
}