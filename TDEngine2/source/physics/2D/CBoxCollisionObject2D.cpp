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

	E_RESULT_CODE CBoxCollisionObject2D::Clone(IComponent*& pDestObject) const
	{
		if (auto pComponent = dynamic_cast<CBoxCollisionObject2D*>(pDestObject))
		{
			pComponent->mWidth = mWidth;
			pComponent->mHeight = mHeight;
			pComponent->mMass = mMass;
			pComponent->mType = mType;

			pComponent->mHasChanged = true;

			return RC_OK;
		}

		return RC_FAIL;
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


	/*!
		\brief CBoxCollisionObject2DFactory's definition
	*/

	CBoxCollisionObject2DFactory::CBoxCollisionObject2DFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* CBoxCollisionObject2DFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return CreateBoxCollisionObject2D(result);
	}

	E_RESULT_CODE CBoxCollisionObject2DFactory::SetupComponent(CBoxCollisionObject2D* pComponent, const TBoxCollisionObject2DParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}

		pComponent->SetWidth(params.mWidth);
		pComponent->SetHeight(params.mHeight);

		return RC_OK;
	}


	IComponentFactory* CreateBoxCollisionObject2DFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CBoxCollisionObject2DFactory, result);
	}
}