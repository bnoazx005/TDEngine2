#include "./../../../include/physics/2D/CBoxCollisionObject2D.h"
#include "./../../../include/physics/2D/ICollisionObjectsVisitor.h"


namespace TDEngine2
{
	CBoxCollisionObject2D::CBoxCollisionObject2D() :
		CBaseCollisionObject2D(), mWidth(1.0f), mHeight(1.0f)
	{
	}

	void CBoxCollisionObject2D::SetWidth(F32 width)
	{
		mWidth = width;
	}

	void CBoxCollisionObject2D::SetHeight(F32 height)
	{
		mHeight = height;
	}

	F32 CBoxCollisionObject2D::GetWidth() const
	{
		return mWidth;
	}

	F32 CBoxCollisionObject2D::GetHeight() const
	{
		return mHeight;
	}
	
	void CBoxCollisionObject2D::GetCollisionShape(const ICollisionObjectsVisitor* pVisitor, const std::function<void(const b2Shape*)>& callback) const
	{
		if (!pVisitor)
		{
			return;
		}
		
		callback(&pVisitor->CreateBoxCollisionShape(*this));
	}


	IComponent* CreateBoxCollisionObject2D(E_RESULT_CODE& result)
	{
		CBoxCollisionObject2D* pBoxCollisionObject2DInstance = new (std::nothrow) CBoxCollisionObject2D();

		if (!pBoxCollisionObject2DInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pBoxCollisionObject2DInstance->Init();

		if (result != RC_OK)
		{
			delete pBoxCollisionObject2DInstance;

			pBoxCollisionObject2DInstance = nullptr;
		}

		return pBoxCollisionObject2DInstance;
	}


	CBoxCollisionObject2DFactory::CBoxCollisionObject2DFactory() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CBoxCollisionObject2DFactory::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CBoxCollisionObject2DFactory::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	IComponent* CBoxCollisionObject2DFactory::Create(const TBaseComponentParameters* pParams) const
	{
		if (!pParams)
		{
			return nullptr;
		}

		const TBoxCollisionObject2DParameters* box2DCollisionParams = static_cast<const TBoxCollisionObject2DParameters*>(pParams);

		E_RESULT_CODE result = RC_OK;

		return CreateBoxCollisionObject2D(result);
	}

	IComponent* CBoxCollisionObject2DFactory::CreateDefault(const TBaseComponentParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		return CreateBoxCollisionObject2D(result);
	}

	TypeId CBoxCollisionObject2DFactory::GetComponentTypeId() const
	{
		return CBoxCollisionObject2D::GetTypeId();
	}


	IComponentFactory* CreateBoxCollisionObject2DFactory(E_RESULT_CODE& result)
	{
		CBoxCollisionObject2DFactory* pBoxCollisionObject2DFactoryInstance = new (std::nothrow) CBoxCollisionObject2DFactory();

		if (!pBoxCollisionObject2DFactoryInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pBoxCollisionObject2DFactoryInstance->Init();

		if (result != RC_OK)
		{
			delete pBoxCollisionObject2DFactoryInstance;

			pBoxCollisionObject2DFactoryInstance = nullptr;
		}

		return pBoxCollisionObject2DFactoryInstance;
	}
}