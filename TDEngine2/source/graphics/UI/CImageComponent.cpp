#include "../../../include/graphics/UI/CImageComponent.h"


namespace TDEngine2
{
	struct TImageArchiveKeys
	{
		static const std::string mSpriteKeyId;
		static const std::string mColorKeyId;
	};


	const std::string TImageArchiveKeys::mSpriteKeyId = "sprite_id";
	const std::string TImageArchiveKeys::mColorKeyId = "color";


	CImage::CImage() : /// \todo Replace with global configurable constant
		CBaseComponent(), mImageResourceId(TResourceId::Invalid), mImageSpriteId("DefaultResources/Textures/DefaultUIWhite_Sprite.png"),
		mColor(TColorUtils::mWhite), mIsDirty(true)
	{
	}

	E_RESULT_CODE CImage::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}

		mImageSpriteId = pReader->GetString(TImageArchiveKeys::mSpriteKeyId);

		pReader->BeginGroup(TImageArchiveKeys::mColorKeyId);
		
		if (auto colorLoadResult = LoadColor32F(pReader))
		{
			mColor = colorLoadResult.Get();
		}

		pReader->EndGroup();

		return RC_OK;
	}

	E_RESULT_CODE CImage::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		pWriter->BeginGroup("component");
		{
			pWriter->SetUInt32("type_id", static_cast<U32>(CImage::GetTypeId()));

			pWriter->SetString(TImageArchiveKeys::mSpriteKeyId, mImageSpriteId);

			pWriter->BeginGroup(TImageArchiveKeys::mColorKeyId);
			SaveColor32F(pWriter, mColor);
			pWriter->EndGroup();
		}
		pWriter->EndGroup();

		
		return RC_OK;
	}

	E_RESULT_CODE CImage::Clone(IComponent*& pDestObject) const
	{
		if (auto pComponent = dynamic_cast<CImage*>(pDestObject))
		{
			pComponent->mColor = mColor;
			pComponent->mImageResourceId = mImageResourceId;
			pComponent->mImageSpriteId = mImageSpriteId;
			pComponent->mIsDirty = true;

			return RC_OK;
		}

		return RC_FAIL;
	}

	E_RESULT_CODE CImage::SetImageId(const std::string& id)
	{
		if (id.empty())
		{
			return RC_INVALID_ARGS;
		}

		mImageSpriteId = id;
		mIsDirty = true;

		return RC_OK;
	}

	E_RESULT_CODE CImage::SetImageResourceId(TResourceId resourceId)
	{
		if (TResourceId::Invalid == resourceId)
		{
			return RC_INVALID_ARGS;
		}

		mImageResourceId = resourceId;
		mIsDirty = true;

		return RC_OK;
	}

	void CImage::SetColor(const TColor32F& value)
	{
		mColor = value;
		mIsDirty = true;
	}

	void CImage::SetDirtyFlag(bool value)
	{
		mIsDirty = value;
	}

	const std::string& CImage::GetImageId() const
	{
		return mImageSpriteId;
	}

	TResourceId CImage::GetImageResourceId() const
	{
		return mImageResourceId;
	}

	const TColor32F& CImage::GetColor() const
	{
		return mColor;
	}

	bool CImage::IsDirty() const
	{
		return mIsDirty;
	}
	
	const std::string& CImage::GetTypeName() const
	{
		static const std::string typeName = "image_ui";
		return typeName;
	}


	IComponent* CreateImage(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CImage, result);
	}


	/*!
		\brief CImageFactory's definition
	*/

	CImageFactory::CImageFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* CImageFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return CreateImage(result);
	}

	E_RESULT_CODE CImageFactory::SetupComponent(CImage* pComponent, const TImageParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}

		return RC_OK;
	}


	IComponentFactory* CreateImageFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CImageFactory, result);
	}
}