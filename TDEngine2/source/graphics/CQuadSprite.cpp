#include "../../include/graphics/CQuadSprite.h"


namespace TDEngine2
{
	CQuadSprite::CQuadSprite() :
		CBaseComponent(), mMaterialName()
	{
	}

	E_RESULT_CODE CQuadSprite::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CQuadSprite::Load(IArchiveReader* pReader)
	{
		return CBaseComponent::Load(pReader);
	}

	E_RESULT_CODE CQuadSprite::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		pWriter->BeginGroup("component");
		{
			pWriter->SetUInt32("type_id", static_cast<U32>(CQuadSprite::GetTypeId()));

			pWriter->SetString("material", mMaterialName);
			
			pWriter->BeginGroup("color");
			SaveColor32F(pWriter, mColor);
			pWriter->EndGroup();
		}
		pWriter->EndGroup();

		return RC_OK;
	}

	void CQuadSprite::SetMaterialName(const std::string& materialName)
	{
		mMaterialName = materialName;
	}

	void CQuadSprite::SetColor(const TColor32F& color)
	{
		mColor = color;
	}

	const std::string& CQuadSprite::GetMaterialName() const
	{
		return mMaterialName;
	}

	const TColor32F& CQuadSprite::GetColor() const
	{
		return mColor;
	}


	IComponent* CreateQuadSprite(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CQuadSprite, result);
	}


	CQuadSpriteFactory::CQuadSpriteFactory() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CQuadSpriteFactory::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CQuadSpriteFactory::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	IComponent* CQuadSpriteFactory::Create(const TBaseComponentParameters* pParams) const
	{
		if (!pParams)
		{
			return nullptr;
		}

		const TQuadSpriteParameters* transformParams = static_cast<const TQuadSpriteParameters*>(pParams);

		E_RESULT_CODE result = RC_OK;

		return CreateQuadSprite(result);
	}

	IComponent* CQuadSpriteFactory::CreateDefault(const TBaseComponentParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		return CreateQuadSprite(result);
	}

	TypeId CQuadSpriteFactory::GetComponentTypeId() const
	{
		return CQuadSprite::GetTypeId();
	}


	IComponentFactory* CreateQuadSpriteFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CQuadSpriteFactory, result);
	}
}