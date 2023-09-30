#include "../../include/graphics/CQuadSprite.h"


namespace TDEngine2
{
	TDE2_REGISTER_COMPONENT_FACTORY(CreateQuadSpriteFactory)


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
		if (!pReader)
		{
			return RC_FAIL;
		}

		pReader->BeginGroup("color");
		{
			if (auto colorResult = LoadColor32F(pReader))
			{
				mColor = colorResult.Get();
			}
		}
		pReader->EndGroup();

		mMaterialName = pReader->GetString("material");
		TDE2_ASSERT(!mMaterialName.empty());

		return RC_OK;
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

	E_RESULT_CODE CQuadSprite::Clone(IComponent*& pDestObject) const
	{
		if (auto pComponent = dynamic_cast<CQuadSprite*>(pDestObject))
		{
			pComponent->mMaterialName = mMaterialName;
			pComponent->mColor = mColor;

			return RC_OK;
		}

		return RC_FAIL;
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


	/*!
		\brief CQuadSpriteFactory's definition
	*/

	CQuadSpriteFactory::CQuadSpriteFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* CQuadSpriteFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return CreateQuadSprite(result);
	}

	E_RESULT_CODE CQuadSpriteFactory::SetupComponent(CQuadSprite* pComponent, const TQuadSpriteParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}

		return RC_OK;
	}


	IComponentFactory* CreateQuadSpriteFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CQuadSpriteFactory, result);
	}
}