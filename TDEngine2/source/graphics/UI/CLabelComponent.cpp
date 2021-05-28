#include "../../../include/graphics/UI/CLabelComponent.h"
#include <stringUtils.hpp>


namespace TDEngine2
{
	struct TLabelArchiveKeys
	{
	};


	CLabel::CLabel() :
		CBaseComponent()
	{
	}

	E_RESULT_CODE CLabel::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mText = "New Text";
		mFontResourceId = Wrench::StringUtils::GetEmptyStr();

		mFontResourceHandle = TResourceId::Invalid;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CLabel::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}
		
		return RC_OK;
	}

	E_RESULT_CODE CLabel::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}
		
		return RC_OK;
	}

	void CLabel::SetText(const std::string& text)
	{
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


	IComponent* CreateLabel(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CLabel, result);
	}


	CLabelFactory::CLabelFactory() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CLabelFactory::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CLabelFactory::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	IComponent* CLabelFactory::Create(const TBaseComponentParameters* pParams) const
	{
		if (!pParams)
		{
			return nullptr;
		}

		const TLabelParameters* params = static_cast<const TLabelParameters*>(pParams);

		E_RESULT_CODE result = RC_OK;

		return CreateLabel(result);
	}

	IComponent* CLabelFactory::CreateDefault(const TBaseComponentParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		return CreateLabel(result);
	}

	TypeId CLabelFactory::GetComponentTypeId() const
	{
		return CLabel::GetTypeId();
	}


	IComponentFactory* CreateLabelFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CLabelFactory, result);
	}
}