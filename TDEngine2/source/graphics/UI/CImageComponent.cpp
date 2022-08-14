#include "../../../include/graphics/UI/CImageComponent.h"


namespace TDEngine2
{
	struct TImageArchiveKeys
	{
		static const std::string mSpriteKeyId;
	};


	const std::string TImageArchiveKeys::mSpriteKeyId = "sprite_id";


	CImage::CImage() : /// \todo Replace with global configurable constant
		CBaseComponent(), mImageResourceId(TResourceId::Invalid), mImageSpriteId("DefaultResources/Textures/DefaultUIWhite_Sprite.png")
	{
	}

	E_RESULT_CODE CImage::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}

		mImageSpriteId = pReader->GetString(TImageArchiveKeys::mSpriteKeyId);

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
		}
		pWriter->EndGroup();

		
		return RC_OK;
	}

	E_RESULT_CODE CImage::SetImageId(const std::string& id)
	{
		if (id.empty())
		{
			return RC_INVALID_ARGS;
		}

		mImageSpriteId = id;

		return RC_OK;
	}

	E_RESULT_CODE CImage::SetImageResourceId(TResourceId resourceId)
	{
		if (TResourceId::Invalid == resourceId)
		{
			return RC_INVALID_ARGS;
		}

		mImageResourceId = resourceId;

		return RC_OK;
	}

	const std::string& CImage::GetImageId() const
	{
		return mImageSpriteId;
	}

	TResourceId CImage::GetImageResourceId() const
	{
		return mImageResourceId;
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