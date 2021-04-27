#include "../../../include/graphics/UI/CLayoutElementComponent.h"


namespace TDEngine2
{
	struct TLayoutElementArchiveKeys
	{
	};


	CLayoutElement::CLayoutElement() :
		CBaseComponent()
	{
	}

	E_RESULT_CODE CLayoutElement::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CLayoutElement::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}
		
		return RC_OK;
	}

	E_RESULT_CODE CLayoutElement::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		return RC_OK;
	}

	E_RESULT_CODE CLayoutElement::SetMinAnchor(const TVector2& value)
	{
		if (value.x > 1.0 || value.x < 0.0)
		{
			return RC_INVALID_ARGS;
		}

		mMinAnchor = value;
		return RC_OK;
	}

	E_RESULT_CODE CLayoutElement::SetMaxAnchor(const TVector2& value)
	{
		if (value.x > 1.0 || value.x < 0.0)
		{
			return RC_INVALID_ARGS;
		}

		mMaxAnchor = value;
		return RC_OK;
	}

	E_RESULT_CODE CLayoutElement::SetMinOffset(const TVector2& value)
	{
		mMinOffset = value;
		return RC_OK;
	}

	E_RESULT_CODE CLayoutElement::SetMaxOffset(const TVector2& value)
	{
		mMaxOffset = value;
		return RC_OK;
	}

	const TVector2& CLayoutElement::GetMinAnchor() const
	{
		return mMinAnchor;
	}

	const TVector2& CLayoutElement::GetMaxAnchor() const
	{
		return mMaxAnchor;
	}

	const TVector2& CLayoutElement::GetMinOffset() const
	{
		return mMinOffset;
	}

	const TVector2& CLayoutElement::GetMaxOffset() const
	{
		return mMaxOffset;
	}


	IComponent* CreateLayoutElement(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CLayoutElement, result);
	}


	CLayoutElementFactory::CLayoutElementFactory() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CLayoutElementFactory::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CLayoutElementFactory::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	IComponent* CLayoutElementFactory::Create(const TBaseComponentParameters* pParams) const
	{
		if (!pParams)
		{
			return nullptr;
		}

		const TLayoutElementParameters* params = static_cast<const TLayoutElementParameters*>(pParams);

		E_RESULT_CODE result = RC_OK;

		return CreateLayoutElement(result);
	}

	IComponent* CLayoutElementFactory::CreateDefault(const TBaseComponentParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		return CreateLayoutElement(result);
	}

	TypeId CLayoutElementFactory::GetComponentTypeId() const
	{
		return CLayoutElement::GetTypeId();
	}


	IComponentFactory* CreateLayoutElementFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CLayoutElementFactory, result);
	}
}