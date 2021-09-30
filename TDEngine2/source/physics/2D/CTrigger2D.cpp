#include "../../../include/physics/2D/CTrigger2D.h"


namespace TDEngine2
{
	CTrigger2D::CTrigger2D() :
		CBaseComponent()
	{
	}

	E_RESULT_CODE CTrigger2D::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
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


	CTrigger2DFactory::CTrigger2DFactory() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CTrigger2DFactory::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	IComponent* CTrigger2DFactory::Create(const TBaseComponentParameters* pParams) const
	{
		if (!pParams)
		{
			return nullptr;
		}

		//const TCircleCollisionObject2DParameters* box2DCollisionParams = static_cast<const TCircleCollisionObject2DParameters*>(pParams);

		E_RESULT_CODE result = RC_OK;

		return CreateTrigger2D(result);
	}

	IComponent* CTrigger2DFactory::CreateDefault(const TBaseComponentParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		return CreateTrigger2D(result);
	}

	TypeId CTrigger2DFactory::GetComponentTypeId() const
	{
		return CTrigger2D::GetTypeId();
	}


	IComponentFactory* CreateTrigger2DFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CTrigger2DFactory, result);
	}
}