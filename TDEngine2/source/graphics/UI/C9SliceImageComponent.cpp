#include "../../../include/graphics/UI/C9SliceImageComponent.h"


namespace TDEngine2
{
	struct T9SliceImageArchiveKeys
	{
	};


	C9SliceImage::C9SliceImage() :
		CBaseComponent(), mImageResourceId(TResourceId::Invalid), mImageSpriteId(Wrench::StringUtils::GetEmptyStr())
	{
	}

	E_RESULT_CODE C9SliceImage::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}
		
		/// \todo Implement C9SliceImage::Load

		return RC_OK;
	}

	E_RESULT_CODE C9SliceImage::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		/// \todo Implement C9SliceImage::Save

		return RC_OK;
	}

	E_RESULT_CODE C9SliceImage::SetImageId(const std::string& id)
	{
		if (id.empty())
		{
			return RC_INVALID_ARGS;
		}

		mImageSpriteId = id;

		return RC_OK;
	}

	E_RESULT_CODE C9SliceImage::SetImageResourceId(TResourceId resourceId)
	{
		if (TResourceId::Invalid == resourceId)
		{
			return RC_INVALID_ARGS;
		}

		mImageResourceId = resourceId;

		return RC_OK;
	}

	E_RESULT_CODE C9SliceImage::SetLeftXSlicer(F32 value)
	{
		mXStart = value;
		return RC_OK;
	}

	E_RESULT_CODE C9SliceImage::SetRightXSlicer(F32 value)
	{
		mXEnd = value;
		return RC_OK;
	}

	E_RESULT_CODE C9SliceImage::SetBottomYSlicer(F32 value)
	{
		mYStart = value;
		return RC_OK;
	}

	E_RESULT_CODE C9SliceImage::SetTopYSlicer(F32 value)
	{
		mYEnd = value;
		return RC_OK;
	}

	const std::string& C9SliceImage::GetImageId() const
	{
		return mImageSpriteId;
	}

	TResourceId C9SliceImage::GetImageResourceId() const
	{
		return mImageResourceId;
	}

	F32 C9SliceImage::GetLeftXSlicer() const
	{
		return mXStart;
	}

	F32 C9SliceImage::GetRightXSlicer() const
	{
		return mXEnd;
	}

	F32 C9SliceImage::GetBottomYSlicer() const
	{
		return mYStart;
	}

	F32 C9SliceImage::GetTopYSlicer() const
	{
		return mYEnd;
	}

	IComponent* Create9SliceImage(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, C9SliceImage, result);
	}


	/*!
		\brief C9SliceImageFactory's definition
	*/

	C9SliceImageFactory::C9SliceImageFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* C9SliceImageFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return Create9SliceImage(result);
	}

	E_RESULT_CODE C9SliceImageFactory::SetupComponent(C9SliceImage* pComponent, const T9SliceImageParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}

		return RC_OK;
	}


	IComponentFactory* Create9SliceImageFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, C9SliceImageFactory, result);
	}
}