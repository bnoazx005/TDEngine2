#include "../../../include/graphics/UI/CLabelComponent.h"
#include "../../../include/core/CFont.h"
#include "../../../include/metadata.h"
#include <stringUtils.hpp>


namespace TDEngine2
{
	struct TLabelArchiveKeys
	{
		static const std::string mTextKeyId;
		static const std::string mFontKeyId;

		static const std::string mAlignTextKeyId;
		static const std::string mOverflowPolicyKeyId;

		static const std::string mColorKeyId;
	};


	const std::string TLabelArchiveKeys::mTextKeyId = "text";
	const std::string TLabelArchiveKeys::mFontKeyId = "font";

	const std::string TLabelArchiveKeys::mAlignTextKeyId = "align_type";
	const std::string TLabelArchiveKeys::mOverflowPolicyKeyId = "overflow_policy_type";
	
	const std::string TLabelArchiveKeys::mColorKeyId = "color";


	CLabel::CLabel() :
		CBaseComponent(), 
		mText("New Text"), 
		mPrevText(Wrench::StringUtils::GetEmptyStr()),
		mFontResourceId(Wrench::StringUtils::GetEmptyStr()), 
		mAlignType(E_FONT_ALIGN_POLICY::CENTER),
		mFontResourceHandle(TResourceId::Invalid)
	{
	}

	E_RESULT_CODE CLabel::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}

		mText = pReader->GetString(TLabelArchiveKeys::mTextKeyId);
		mFontResourceId = pReader->GetString(TLabelArchiveKeys::mFontKeyId);

		mAlignType = Meta::EnumTrait<E_FONT_ALIGN_POLICY>::FromString(pReader->GetString(TLabelArchiveKeys::mAlignTextKeyId));
		mOverflowPolicyType = Meta::EnumTrait<E_TEXT_OVERFLOW_POLICY>::FromString(pReader->GetString(TLabelArchiveKeys::mOverflowPolicyKeyId));

		pReader->BeginGroup(TLabelArchiveKeys::mColorKeyId);

		if (auto colorLoadResult = LoadColor32F(pReader))
		{
			mFontVertexColor = colorLoadResult.Get();
		}

		pReader->EndGroup();

		return RC_OK;
	}

	E_RESULT_CODE CLabel::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		pWriter->BeginGroup("component");
		{
			pWriter->SetUInt32("type_id", static_cast<U32>(CLabel::GetTypeId()));

			pWriter->SetString(TLabelArchiveKeys::mTextKeyId, mText);
			pWriter->SetString(TLabelArchiveKeys::mFontKeyId, mFontResourceId);
			
			pWriter->SetString(TLabelArchiveKeys::mAlignTextKeyId, Meta::EnumTrait<E_FONT_ALIGN_POLICY>::ToString(mAlignType));
			pWriter->SetString(TLabelArchiveKeys::mOverflowPolicyKeyId, Meta::EnumTrait<E_TEXT_OVERFLOW_POLICY>::ToString(mOverflowPolicyType));

			pWriter->BeginGroup(TLabelArchiveKeys::mColorKeyId);
			SaveColor32F(pWriter, mFontVertexColor);
			pWriter->EndGroup();
		}
		pWriter->EndGroup();
		
		return RC_OK;
	}

	E_RESULT_CODE CLabel::Clone(IComponent*& pDestObject) const
	{
		if (auto pComponent = dynamic_cast<CLabel*>(pDestObject))
		{
			pComponent->mAlignType = mAlignType;
			pComponent->mFontDataVersionId = mFontDataVersionId;
			pComponent->mFontResourceHandle = mFontResourceHandle;
			pComponent->mFontResourceId = mFontResourceId;
			pComponent->mOverflowPolicyType = mOverflowPolicyType;
			pComponent->mPrevText = mPrevText;
			pComponent->mText = mText;
			pComponent->mFontVertexColor = mFontVertexColor;

			return RC_OK;
		}

		return RC_FAIL;
	}

	void CLabel::SetText(const std::string& text)
	{
		mPrevText = mText;
		mText = text;
	}

	E_RESULT_CODE CLabel::SetFontId(const std::string& fontId)
	{
		if (fontId.empty())
		{
			return RC_INVALID_ARGS;
		}

		mFontResourceId = fontId;

		return RC_OK;
	}

	E_RESULT_CODE CLabel::SetFontResourceHandle(TResourceId handle)
	{
		if (TResourceId::Invalid == handle)
		{
			return RC_INVALID_ARGS;
		}

		mFontResourceHandle = handle;

		return RC_OK;
	}

	void CLabel::SetAlignType(E_FONT_ALIGN_POLICY value)
	{
		mAlignType = value;
	}

	void CLabel::SetOverflowPolicyType(E_TEXT_OVERFLOW_POLICY value)
	{
		mOverflowPolicyType = value;
	}

	void CLabel::SetFontDataVersionId(U32 value)
	{
		mFontDataVersionId = value;
	}

	void CLabel::SetColor(const TColor32F& color)
	{
		mFontVertexColor = color;
		mPrevText = Wrench::StringUtils::GetEmptyStr();
	}

	void CLabel::ResetDirtyFlag()
	{
		mPrevText = mText;
	}

	const std::string& CLabel::GetText() const
	{
		return mText;
	}

	const std::string& CLabel::GetFontId() const
	{
		return mFontResourceId;
	}

	TResourceId CLabel::GetFontResourceHandle() const
	{
		return mFontResourceHandle;
	}

	E_FONT_ALIGN_POLICY CLabel::GetAlignType() const
	{
		return mAlignType;
	}

	E_TEXT_OVERFLOW_POLICY CLabel::GetOverflowPolicyType() const
	{
		return mOverflowPolicyType;
	}

	U32 CLabel::GetFontDataVersionId() const
	{
		return mFontDataVersionId;
	}

	const TColor32F& CLabel::GetColor() const
	{
		return mFontVertexColor;
	}

	bool CLabel::IsDirty() const
	{
		return mPrevText != mText;
	}


	IComponent* CreateLabel(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CLabel, result);
	}


	/*!
		\brief CLabelFactory's definition
	*/

	CLabelFactory::CLabelFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* CLabelFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return CreateLabel(result);
	}

	E_RESULT_CODE CLabelFactory::SetupComponent(CLabel* pComponent, const TLabelParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}

		return RC_OK;
	}


	IComponentFactory* CreateLabelFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CLabelFactory, result);
	}
}