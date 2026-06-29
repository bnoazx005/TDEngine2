#include "../../../include/graphics/UI/CInputReceiverComponent.h"
#define META_EXPORT_ECS_SECTION
#include "../../../include/metadata.h"


namespace TDEngine2
{
	TDE2_REGISTER_COMPONENT_FACTORY(CreateInputReceiverFactory)
	TDE2_DEFINE_COMPONENT_META(TInputReceiverComponentData)


	CInputReceiver::CInputReceiver() :
		CBaseComponentT()
	{
	}

	const std::string& CInputReceiver::GetTypeName() const
	{
		static const std::string typeName = "input_receiver";
		return typeName;
	}


	IComponent* CreateInputReceiver(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CInputReceiver, result);
	}


	/*!
		\brief CInputReceiverFactory's definition
	*/

	CInputReceiverFactory::CInputReceiverFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* CInputReceiverFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return CreateInputReceiver(result);
	}

	E_RESULT_CODE CInputReceiverFactory::SetupComponent(CInputReceiver* pComponent, const TInputReceiverParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}

		return RC_OK;
	}


	IComponentFactory* CreateInputReceiverFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CInputReceiverFactory, result);
	}
}