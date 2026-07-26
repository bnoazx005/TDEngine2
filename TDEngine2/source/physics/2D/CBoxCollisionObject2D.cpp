#include "../../../include/physics/2D/CBoxCollisionObject2D.h"
#define META_EXPORT_ECS_SECTION
#include "../../include/metadata.h"


namespace TDEngine2
{
	TDE2_REGISTER_COMPONENT_FACTORY(CreateBoxCollisionObject2DFactory)
	TDE2_DEFINE_COMPONENT_META(TBoxCollisionObject2DComponentData)


	CBoxCollisionObject2D::CBoxCollisionObject2D() :
		CBaseComponentT()
	{
	}


	IComponent* CreateBoxCollisionObject2D(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CBoxCollisionObject2D, result);
	}


	/*!
		\brief CBoxCollisionObject2DFactory's definition
	*/

	CBoxCollisionObject2DFactory::CBoxCollisionObject2DFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* CBoxCollisionObject2DFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return CreateBoxCollisionObject2D(result);
	}

	E_RESULT_CODE CBoxCollisionObject2DFactory::SetupComponent(CBoxCollisionObject2D* pComponent, const TBoxCollisionObject2DParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}

		TBoxCollisionObject2DComponentData& box2DComponentData = pComponent->GetData();
		box2DComponentData.mWidth  = params.mWidth;
		box2DComponentData.mHeight = params.mHeight;

		return RC_OK;
	}


	IComponentFactory* CreateBoxCollisionObject2DFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CBoxCollisionObject2DFactory, result);
	}
}