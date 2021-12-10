#include "../../../include/physics/2D/CTrigger2D.h"


namespace TDEngine2
{
	CTrigger2D::CTrigger2D() :
		CBaseComponent()
	{
	}

	E_RESULT_CODE CTrigger2D::Load(IArchiveReader* pReader)
	{
		return CBaseComponent::Load(pReader);
	}

	E_RESULT_CODE CTrigger2D::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		pWriter->BeginGroup("component");
		{
			pWriter->SetUInt32("type_id", static_cast<U32>(CTrigger2D::GetTypeId()));
		}
		pWriter->EndGroup();

		return RC_OK;
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