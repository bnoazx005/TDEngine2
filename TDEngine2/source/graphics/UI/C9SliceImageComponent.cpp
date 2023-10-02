#include "../../../include/graphics/UI/C9SliceImageComponent.h"


namespace TDEngine2
{
	TDE2_REGISTER_COMPONENT_FACTORY(Create9SliceImageFactory)


	C9SliceImage::C9SliceImage() :
		CBaseComponent(), mImageResourceId(TResourceId::Invalid), mImageSpriteId(Wrench::StringUtils::GetEmptyStr())
	{
	}


	struct T9SliceImageArchiveKeys
	{
		static const std::string mSpriteIdKey;

		static const std::string mLeftSliceKey;
		static const std::string mRightSliceKey;
		static const std::string mBottomSliceKey;
		static const std::string mTopSliceKey;

		static const std::string mMarginKey;

		static const std::string mColorKey;
	};


	const std::string T9SliceImageArchiveKeys::mSpriteIdKey = "sprite_id";

	const std::string T9SliceImageArchiveKeys::mLeftSliceKey = "left_slice";
	const std::string T9SliceImageArchiveKeys::mRightSliceKey = "right_slice";
	const std::string T9SliceImageArchiveKeys::mBottomSliceKey = "bottom_slice";
	const std::string T9SliceImageArchiveKeys::mTopSliceKey = "top_slice";

	const std::string T9SliceImageArchiveKeys::mMarginKey = "margin";

	const std::string T9SliceImageArchiveKeys::mColorKey = "color";


	E_RESULT_CODE C9SliceImage::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}

		mImageSpriteId = pReader->GetString(T9SliceImageArchiveKeys::mSpriteIdKey);

		mRelativeBorderSize = CMathUtils::Clamp(0.0f, 0.5f, pReader->GetFloat(T9SliceImageArchiveKeys::mMarginKey));

		mXStart = pReader->GetFloat(T9SliceImageArchiveKeys::mLeftSliceKey);
		mXEnd   = pReader->GetFloat(T9SliceImageArchiveKeys::mRightSliceKey);
		mYStart = pReader->GetFloat(T9SliceImageArchiveKeys::mBottomSliceKey);
		mYEnd   = pReader->GetFloat(T9SliceImageArchiveKeys::mTopSliceKey);

		pReader->BeginGroup(T9SliceImageArchiveKeys::mColorKey);

		if (auto colorLoadResult = LoadColor32F(pReader))
		{
			mColor = colorLoadResult.Get();
		}

		pReader->EndGroup();

		return RC_OK;
	}

	E_RESULT_CODE C9SliceImage::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		pWriter->BeginGroup("component");
		{
			pWriter->SetUInt32("type_id", static_cast<U32>(C9SliceImage::GetTypeId()));

			pWriter->SetString(T9SliceImageArchiveKeys::mSpriteIdKey, mImageSpriteId);

			pWriter->SetFloat(T9SliceImageArchiveKeys::mMarginKey, CMathUtils::Clamp(0.0f, 0.5f, mRelativeBorderSize));

			pWriter->SetFloat(T9SliceImageArchiveKeys::mLeftSliceKey, mXStart);
			pWriter->SetFloat(T9SliceImageArchiveKeys::mRightSliceKey, mXEnd);
			pWriter->SetFloat(T9SliceImageArchiveKeys::mBottomSliceKey, mYStart);
			pWriter->SetFloat(T9SliceImageArchiveKeys::mTopSliceKey, mYEnd);

			pWriter->BeginGroup(T9SliceImageArchiveKeys::mColorKey);
			SaveColor32F(pWriter, mColor);
			pWriter->EndGroup();
		}
		pWriter->EndGroup();

		return RC_OK;
	}

	E_RESULT_CODE C9SliceImage::Clone(IComponent*& pDestObject) const
	{
		if (auto pComponent = dynamic_cast<C9SliceImage*>(pDestObject))
		{
			pComponent->mImageResourceId = mImageResourceId;
			pComponent->mImageSpriteId = mImageSpriteId;
			pComponent->mRelativeBorderSize = mRelativeBorderSize;
			pComponent->mXEnd = mXEnd;
			pComponent->mXStart = mXStart;
			pComponent->mYEnd = mYEnd;
			pComponent->mYStart = mYStart;
			pComponent->mColor = mColor;

			return RC_OK;
		}

		return RC_FAIL;
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

	E_RESULT_CODE C9SliceImage::SetRelativeBorderSize(F32 value)
	{
		mRelativeBorderSize = value;
		return RC_OK;
	}

	void C9SliceImage::SetColor(const TColor32F& value)
	{
		mColor = value;
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

	F32 C9SliceImage::GetRelativeBorderSize() const
	{
		return mRelativeBorderSize;
	}

	const TColor32F& C9SliceImage::GetColor() const
	{
		return mColor;
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