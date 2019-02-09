#include "./../../../include/physics/2D/CCircleCollisionObject2D.h"


namespace TDEngine2
{
	CCircleCollisionObject2D::CCircleCollisionObject2D() :
		CBaseCollisionObject2D(), mRadius(1.0f)
	{
	}

	void CCircleCollisionObject2D::SetRadius(F32 radius)
	{
		mRadius = radius;
	}
	
	F32 CCircleCollisionObject2D::GetRadius() const
	{
		return mRadius;
	}


	IComponent* CreateCircleCollisionObject2D(E_RESULT_CODE& result)
	{
		CCircleCollisionObject2D* pCircleCollisionObject2DInstance = new (std::nothrow) CCircleCollisionObject2D();

		if (!pCircleCollisionObject2DInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pCircleCollisionObject2DInstance->Init();

		if (result != RC_OK)
		{
			delete pCircleCollisionObject2DInstance;

			pCircleCollisionObject2DInstance = nullptr;
		}

		return pCircleCollisionObject2DInstance;
	}


	CCircleCollisionObject2DFactory::CCircleCollisionObject2DFactory() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CCircleCollisionObject2DFactory::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CCircleCollisionObject2DFactory::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	IComponent* CCircleCollisionObject2DFactory::Create(const TBaseComponentParameters* pParams) const
	{
		if (!pParams)
		{
			return nullptr;
		}

		const TCircleCollisionObject2DParameters* box2DCollisionParams = static_cast<const TCircleCollisionObject2DParameters*>(pParams);

		E_RESULT_CODE result = RC_OK;

		return CreateCircleCollisionObject2D(result);
	}

	IComponent* CCircleCollisionObject2DFactory::CreateDefault(const TBaseComponentParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		return CreateCircleCollisionObject2D(result);
	}

	TypeId CCircleCollisionObject2DFactory::GetComponentTypeId() const
	{
		return CCircleCollisionObject2D::GetTypeId();
	}


	IComponentFactory* CreateCircleCollisionObject2DFactory(E_RESULT_CODE& result)
	{
		CCircleCollisionObject2DFactory* pCircleCollisionObject2DFactoryInstance = new (std::nothrow) CCircleCollisionObject2DFactory();

		if (!pCircleCollisionObject2DFactoryInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pCircleCollisionObject2DFactoryInstance->Init();

		if (result != RC_OK)
		{
			delete pCircleCollisionObject2DFactoryInstance;

			pCircleCollisionObject2DFactoryInstance = nullptr;
		}

		return pCircleCollisionObject2DFactoryInstance;
	}
}