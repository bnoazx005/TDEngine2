#include "../../../include/graphics/UI/CImageComponent.h"


namespace TDEngine2
{
	struct TImageArchiveKeys
	{
	};


	CImage::CImage() :
		CBaseComponent(), mImageResourceId(TResourceId::Invalid), mImageSpriteId(Wrench::StringUtils::GetEmptyStr())
	{
	}

	E_RESULT_CODE CImage::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}
		
		return RC_OK;
	}

	E_RESULT_CODE CImage::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}
		
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