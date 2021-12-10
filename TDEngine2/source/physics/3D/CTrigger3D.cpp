#include "../../../include/physics/3D/CTrigger3D.h"


namespace TDEngine2
{
	CTrigger3D::CTrigger3D() :
		CBaseComponent()
	{
	}

	E_RESULT_CODE CTrigger3D::Load(IArchiveReader* pReader)
	{
		return CBaseComponent::Load(pReader);
	}

	E_RESULT_CODE CTrigger3D::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		pWriter->BeginGroup("component");
		{
			pWriter->SetUInt32("type_id", static_cast<U32>(CTrigger3D::GetTypeId()));
		}
		pWriter->EndGroup();

		return RC_OK;
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