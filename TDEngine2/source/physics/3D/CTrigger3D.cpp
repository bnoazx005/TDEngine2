#include "../../../include/physics/3D/CTrigger3D.h"


namespace TDEngine2
{
	CTrigger3D::CTrigger3D() :
		CBaseComponent()
	{
	}

	E_RESULT_CODE CTrigger3D::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
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


	CTrigger3DFactory::CTrigger3DFactory() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CTrigger3DFactory::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	IComponent* CTrigger3DFactory::Create(const TBaseComponentParameters* pParams) const
	{
		if (!pParams)
		{
			return nullptr;
		}

		//const TCircleCollisionObject3DParameters* box3DCollisionParams = static_cast<const TCircleCollisionObject3DParameters*>(pParams);

		E_RESULT_CODE result = RC_OK;

		return CreateTrigger3D(result);
	}

	IComponent* CTrigger3DFactory::CreateDefault(const TBaseComponentParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		return CreateTrigger3D(result);
	}

	TypeId CTrigger3DFactory::GetComponentTypeId() const
	{
		return CTrigger3D::GetTypeId();
	}


	IComponentFactory* CreateTrigger3DFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CTrigger3DFactory, result);
	}
}