#include "../../../include/graphics/UI/CImageComponent.h"


namespace TDEngine2
{
	struct TImageArchiveKeys
	{
	};


	CImage::CImage() :
		CBaseComponent()
	{
	}

	E_RESULT_CODE CImage::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mImageResourceId = TResourceId::Invalid;
		mImageSpriteId = Wrench::StringUtils::GetEmptyStr();

		mIsInitialized = true;

		return RC_OK;
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


	CImageFactory::CImageFactory() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CImageFactory::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	IComponent* CImageFactory::Create(const TBaseComponentParameters* pParams) const
	{
		if (!pParams)
		{
			return nullptr;
		}

		const TImageParameters* params = static_cast<const TImageParameters*>(pParams);

		E_RESULT_CODE result = RC_OK;

		return CreateImage(result);
	}

	IComponent* CImageFactory::CreateDefault(const TBaseComponentParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		return CreateImage(result);
	}

	TypeId CImageFactory::GetComponentTypeId() const
	{
		return CImage::GetTypeId();
	}


	IComponentFactory* CreateImageFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CImageFactory, result);
	}
}