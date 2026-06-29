#include "../../../include/graphics/UI/C9SliceImageComponent.h"
#define META_EXPORT_ECS_SECTION
#include "../../../include/metadata.h"


namespace TDEngine2
{
	TDE2_REGISTER_COMPONENT_FACTORY(Create9SliceImageFactory)
	TDE2_DEFINE_COMPONENT_META(T9SliceImageComponentData)


	C9SliceImage::C9SliceImage() :
		CBaseComponentT()
	{
	}

	E_RESULT_CODE C9SliceImage::SetImageId(const std::string& id)
	{
		if (id.empty())
		{
			return RC_INVALID_ARGS;
		}

		mData.mImageSpriteId = id;

		return RC_OK;
	}

	E_RESULT_CODE C9SliceImage::SetImageResourceId(TResourceId resourceId)
	{
		if (TResourceId::Invalid == resourceId)
		{
			return RC_INVALID_ARGS;
		}

		mData.mImageResourceId = resourceId;

		return RC_OK;
	}

	E_RESULT_CODE C9SliceImage::SetLeftXSlicer(F32 value)
	{
		mData.mXStart = value;
		return RC_OK;
	}

	E_RESULT_CODE C9SliceImage::SetRightXSlicer(F32 value)
	{
		mData.mXEnd = value;
		return RC_OK;
	}

	E_RESULT_CODE C9SliceImage::SetBottomYSlicer(F32 value)
	{
		mData.mYStart = value;
		return RC_OK;
	}

	E_RESULT_CODE C9SliceImage::SetTopYSlicer(F32 value)
	{
		mData.mYEnd = value;
		return RC_OK;
	}

	E_RESULT_CODE C9SliceImage::SetRelativeBorderSize(F32 value)
	{
		mData.mRelativeBorderSize = value;
		return RC_OK;
	}

	void C9SliceImage::SetColor(const TColor32F& value)
	{
		mData.mColor = value;
	}

	const std::string& C9SliceImage::GetImageId() const
	{
		return mData.mImageSpriteId;
	}

	TResourceId C9SliceImage::GetImageResourceId() const
	{
		return mData.mImageResourceId;
	}

	F32 C9SliceImage::GetLeftXSlicer() const
	{
		return mData.mXStart;
	}

	F32 C9SliceImage::GetRightXSlicer() const
	{
		return mData.mXEnd;
	}

	F32 C9SliceImage::GetBottomYSlicer() const
	{
		return mData.mYStart;
	}

	F32 C9SliceImage::GetTopYSlicer() const
	{
		return mData.mYEnd;
	}

	F32 C9SliceImage::GetRelativeBorderSize() const
	{
		return mData.mRelativeBorderSize;
	}

	const TColor32F& C9SliceImage::GetColor() const
	{
		return mData.mColor;
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