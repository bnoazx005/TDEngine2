#include "../../include/scene/components/CObjIdComponent.h"
#define META_EXPORT_ECS_SECTION
#include "../../include/metadata.h"


namespace TDEngine2
{
	TDE2_REGISTER_COMPONENT_FACTORY(CreateObjIdComponentFactory)
	TDE2_DEFINE_COMPONENT_META(TObjIdComponentData)


	CObjIdComponent::CObjIdComponent() :
		CBaseComponentT()
	{
	}

	bool CObjIdComponent::IsRuntimeOnly() const
	{
		return true;
	}

	const std::string& CObjIdComponent::GetTypeName() const
	{
		static const std::string typeName = "obj_id";
		return typeName;
	}


	IComponent* CreateObjIdComponent(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CObjIdComponent, result);
	}


	/*!
		\brief CObjIdComponentFactory's definition
	*/

	CObjIdComponentFactory::CObjIdComponentFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* CObjIdComponentFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return CreateObjIdComponent(result);
	}

	E_RESULT_CODE CObjIdComponentFactory::SetupComponent(CObjIdComponent* pComponent, const TObjIdComponentParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}

		return RC_OK;
	}


	IComponentFactory* CreateObjIdComponentFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CObjIdComponentFactory, result);
	}
}