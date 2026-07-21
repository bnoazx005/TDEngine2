#include "../../../include/physics/2D/CTrigger2D.h"
#define META_EXPORT_ECS_SECTION
#include "../../include/metadata.h"


namespace TDEngine2
{
	TDE2_REGISTER_COMPONENT_FACTORY(CreateTrigger2DFactory)
	TDE2_DEFINE_COMPONENT_META(TTrigger2DComponentData)


	CTrigger2D::CTrigger2D() :
		CBaseComponentT()
	{
	}

	const std::string& CTrigger2D::GetTypeName() const
	{
		static const std::string id{ "trigger_2d" };
		return id;
	}


	IComponent* CreateTrigger2D(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CTrigger2D, result);
	}


	/*!
		\brief CTrigger2DFactory's definition
	*/

	CTrigger2DFactory::CTrigger2DFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* CTrigger2DFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return CreateTrigger2D(result);
	}

	E_RESULT_CODE CTrigger2DFactory::SetupComponent(CTrigger2D* pComponent, const TTrigger2DParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}
		
		return RC_OK;
	}


	IComponentFactory* CreateTrigger2DFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CTrigger2DFactory, result);
	}
}