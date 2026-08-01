#include "../include/CTrigger3D.h"
#define META_EXPORT_BULLET_ECS_PLUGIN_SECTION
#include "../../include/metadata.h"


namespace TDEngine2
{
	TDE2_DEFINE_COMPONENT_META(TTrigger3DComponentData)


	CTrigger3D::CTrigger3D() :
		CBaseComponentT()
	{
	}

	const std::string& CTrigger3D::GetTypeName() const
	{
		static const std::string id{ "trigger_3d" };
		return id;
	}


	IComponent* CreateTrigger3D(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CTrigger3D, result);
	}


	/*!
		\brief CTrigger3DFactory's definition
	*/

	CTrigger3DFactory::CTrigger3DFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* CTrigger3DFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return CreateTrigger3D(result);
	}

	E_RESULT_CODE CTrigger3DFactory::SetupComponent(CTrigger3D* pComponent, const TTrigger3DParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}

		return RC_OK;
	}


	IComponentFactory* CreateTrigger3DFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CTrigger3DFactory, result);
	}
}