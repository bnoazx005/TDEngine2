#include "../../../include/physics/2D/CCircleCollisionObject2D.h"
#include "../../../include/physics/2D/ICollisionObjectsVisitor.h"


namespace TDEngine2
{
	CCircleCollisionObject2D::CCircleCollisionObject2D() :
		CBaseCollisionObject2D(), mRadius(1.0f)
	{
	}

	E_RESULT_CODE CCircleCollisionObject2D::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}

		mRadius = pReader->GetFloat("radius");

		return RC_OK;
	}

	E_RESULT_CODE CCircleCollisionObject2D::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		pWriter->BeginGroup("component");
		{
			pWriter->SetUInt32("type_id", static_cast<U32>(CCircleCollisionObject2D::GetTypeId()));

			pWriter->SetFloat("radius", mRadius);
		}
		pWriter->EndGroup();

		return RC_OK;
	}

	void CCircleCollisionObject2D::SetRadius(F32 radius)
	{
		mRadius = radius;
	}
	
	F32 CCircleCollisionObject2D::GetRadius() const
	{
		return mRadius;
	}

	void CCircleCollisionObject2D::GetCollisionShape(const ICollisionObjectsVisitor* pVisitor, const std::function<void(const b2Shape*)>& callback) const
	{
		if (!pVisitor)
		{
			return;
		}

		b2CircleShape circleShape = pVisitor->CreateCircleCollisionShape(*this);

		callback(&circleShape);
	}


	IComponent* CreateCircleCollisionObject2D(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CCircleCollisionObject2D, result);
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
		return CREATE_IMPL(IComponentFactory, CCircleCollisionObject2DFactory, result);
	}
}