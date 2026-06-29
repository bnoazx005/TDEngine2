#include "../../../include/graphics/UI/CLabelComponent.h"
#include "../../../include/core/CFont.h"
#define META_EXPORT_UI_SECTION
#define META_EXPORT_ECS_SECTION
#include "../../../include/metadata.h"
#include <stringUtils.hpp>


namespace TDEngine2
{
	TDE2_REGISTER_COMPONENT_FACTORY(CreateLabelFactory)
	TDE2_DEFINE_COMPONENT_META(TLabelComponentData)

	CLabel::CLabel() :
		CBaseComponentT()
	{
	}

	void CLabel::SetText(const std::string& text)
	{
		TDE2_MULTI_THREAD_ACCESS_CHECK(mMTCheckLock);

		mData.mPrevText = mData.mText;
		mData.mText = text;
	}

	E_RESULT_CODE CLabel::SetFontId(const std::string& fontId)
	{
		TDE2_MULTI_THREAD_ACCESS_CHECK(mMTCheckLock);

		if (fontId.empty())
		{
			return RC_INVALID_ARGS;
		}

		mData.mFontResourceId = fontId;

		return RC_OK;
	}

	E_RESULT_CODE CLabel::SetFontResourceHandle(TResourceId handle)
	{
		TDE2_MULTI_THREAD_ACCESS_CHECK(mMTCheckLock);

		if (TResourceId::Invalid == handle)
		{
			return RC_INVALID_ARGS;
		}

		mData.mFontResourceHandle = handle;

		return RC_OK;
	}

	void CLabel::SetAlignType(E_FONT_ALIGN_POLICY value)
	{
		TDE2_MULTI_THREAD_ACCESS_CHECK(mMTCheckLock);
		mData.mAlignType = value;
	}

	void CLabel::SetOverflowPolicyType(E_TEXT_OVERFLOW_POLICY value)
	{
		TDE2_MULTI_THREAD_ACCESS_CHECK(mMTCheckLock);
		mData.mOverflowPolicyType = value;
	}

	void CLabel::SetFontDataVersionId(U32 value)
	{
		TDE2_MULTI_THREAD_ACCESS_CHECK(mMTCheckLock);
		mData.mFontDataVersionId = value;
	}

	void CLabel::SetColor(const TColor32F& color)
	{
		TDE2_MULTI_THREAD_ACCESS_CHECK(mMTCheckLock);
		mData.mFontVertexColor = color;
		mData.mPrevText = Wrench::StringUtils::GetEmptyStr();
	}

	void CLabel::SetTextHeight(U32 height)
	{
		TDE2_MULTI_THREAD_ACCESS_CHECK(mMTCheckLock);
		mData.mTextHeight = height;
		mData.mPrevText = Wrench::StringUtils::GetEmptyStr();
	}

	void CLabel::ResetDirtyFlag()
	{
		TDE2_MULTI_THREAD_ACCESS_CHECK(mMTCheckLock);
		mData.mPrevText = mData.mText;
	}

	const std::string& CLabel::GetText() const
	{
		TDE2_MULTI_THREAD_ACCESS_CHECK(mMTCheckLock);
		return mData.mText;
	}

	const std::string& CLabel::GetFontId() const
	{
		TDE2_MULTI_THREAD_ACCESS_CHECK(mMTCheckLock);
		return mData.mFontResourceId;
	}

	TResourceId CLabel::GetFontResourceHandle() const
	{
		TDE2_MULTI_THREAD_ACCESS_CHECK(mMTCheckLock);
		return mData.mFontResourceHandle;
	}

	E_FONT_ALIGN_POLICY CLabel::GetAlignType() const
	{
		TDE2_MULTI_THREAD_ACCESS_CHECK(mMTCheckLock);
		return mData.mAlignType;
	}

	E_TEXT_OVERFLOW_POLICY CLabel::GetOverflowPolicyType() const
	{
		TDE2_MULTI_THREAD_ACCESS_CHECK(mMTCheckLock);
		return mData.mOverflowPolicyType;
	}

	U32 CLabel::GetFontDataVersionId() const
	{
		TDE2_MULTI_THREAD_ACCESS_CHECK(mMTCheckLock);
		return mData.mFontDataVersionId;
	}

	const TColor32F& CLabel::GetColor() const
	{
		TDE2_MULTI_THREAD_ACCESS_CHECK(mMTCheckLock);
		return mData.mFontVertexColor;
	}

	U32 CLabel::GetTextHeight() const
	{
		TDE2_MULTI_THREAD_ACCESS_CHECK(mMTCheckLock);
		return mData.mTextHeight;
	}

	bool CLabel::IsDirty() const
	{
		TDE2_MULTI_THREAD_ACCESS_CHECK(mMTCheckLock);
		return mData.mPrevText != mData.mText;
	}

	const std::string& CLabel::GetTypeName() const
	{
		static const std::string id{ "label" };
		return id;
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