#include "../../../include/physics/2D/CCircleCollisionObject2D.h"
#define META_EXPORT_ECS_SECTION
#include "../../include/metadata.h"


namespace TDEngine2
{
	TDE2_REGISTER_COMPONENT_FACTORY(CreateCircleCollisionObject2DFactory)
	TDE2_DEFINE_COMPONENT_META(TCircleCollisionObject2DComponentData);


	CCircleCollisionObject2D::CCircleCollisionObject2D() :
		CBaseComponentT()
	{
	}


	IComponent* CreateCircleCollisionObject2D(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CCircleCollisionObject2D, result);
	}


	/*!
		\brief CCircleCollisionObject2DFactory's definition
	*/

	CCircleCollisionObject2DFactory::CCircleCollisionObject2DFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* CCircleCollisionObject2DFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return CreateCircleCollisionObject2D(result);
	}

	E_RESULT_CODE CCircleCollisionObject2DFactory::SetupComponent(CCircleCollisionObject2D* pComponent, const TCircleCollisionObject2DParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}

		pComponent->GetData().mRadius = params.mRadius;

		return RC_OK;
	}


	IComponentFactory* CreateCircleCollisionObject2DFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CCircleCollisionObject2DFactory, result);
	}
}