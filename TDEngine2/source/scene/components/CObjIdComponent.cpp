#include "../../include/scene/components/CObjIdComponent.h"


namespace TDEngine2
{
	CObjIdComponent::CObjIdComponent() :
		CBaseComponent()
	{
	}

	E_RESULT_CODE CObjIdComponent::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}

		mId = pReader->GetUInt32("obj_id");

		return RC_OK;
	}

	E_RESULT_CODE CObjIdComponent::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		pWriter->BeginGroup("component");
		{
			pWriter->SetUInt32("type_id", static_cast<U32>(CObjIdComponent::GetTypeId()));
			pWriter->SetUInt32("obj_id", mId);
		}
		pWriter->EndGroup();

		return RC_OK;
	}

	E_RESULT_CODE CObjIdComponent::Clone(IComponent*& pDestObject) const
	{
		if (auto pComponent = dynamic_cast<CObjIdComponent*>(pDestObject))
		{
			pComponent->mId = mId;
			
			return RC_OK;
		}

		return RC_FAIL;
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