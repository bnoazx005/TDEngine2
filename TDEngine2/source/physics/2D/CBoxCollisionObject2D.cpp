#include "../../../include/physics/2D/CBoxCollisionObject2D.h"
#include "../../../include/physics/2D/ICollisionObjectsVisitor.h"


namespace TDEngine2
{
	CBoxCollisionObject2D::CBoxCollisionObject2D() :
		CBaseCollisionObject2D(), mWidth(1.0f), mHeight(1.0f)
	{
	}

	E_RESULT_CODE CBoxCollisionObject2D::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}

		mWidth = pReader->GetFloat("width");
		mHeight = pReader->GetFloat("height");

		return RC_OK;
	}

	E_RESULT_CODE CBoxCollisionObject2D::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		pWriter->BeginGroup("component");
		{
			pWriter->SetUInt32("type_id", static_cast<U32>(CBoxCollisionObject2D::GetTypeId()));

			pWriter->SetFloat("width", mWidth);
			pWriter->SetFloat("height", mHeight);
		}
		pWriter->EndGroup();

		return RC_OK;
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
		
		b2PolygonShape boxShape = pVisitor->CreateBoxCollisionShape(*this);

		callback(&boxShape);
	}


	IComponent* CreateBoxCollisionObject2D(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CBoxCollisionObject2D, result);
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
		return CREATE_IMPL(IComponentFactory, CBoxCollisionObject2DFactory, result);
	}
}