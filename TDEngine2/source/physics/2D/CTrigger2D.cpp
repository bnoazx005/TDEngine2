#include "./../../../include/physics/2D/CTrigger2D.h"


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


	IComponent* CreateTrigger2D(E_RESULT_CODE& result)
	{
		CTrigger2D* pTrigger2DInstance = new (std::nothrow) CTrigger2D();

		if (!pTrigger2DInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pTrigger2DInstance->Init();

		if (result != RC_OK)
		{
			delete pTrigger2DInstance;

			pTrigger2DInstance = nullptr;
		}

		return pTrigger2DInstance;
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

	E_RESULT_CODE CTrigger2DFactory::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

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
		CTrigger2DFactory* pTrigger2DFactoryInstance = new (std::nothrow) CTrigger2DFactory();

		if (!pTrigger2DFactoryInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pTrigger2DFactoryInstance->Init();

		if (result != RC_OK)
		{
			delete pTrigger2DFactoryInstance;

			pTrigger2DFactoryInstance = nullptr;
		}

		return pTrigger2DFactoryInstance;
	}
}