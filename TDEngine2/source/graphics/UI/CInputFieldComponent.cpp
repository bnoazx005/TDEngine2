#include "../../../include/graphics/UI/CInputFieldComponent.h"


namespace TDEngine2
{
	TDE2_REGISTER_COMPONENT_FACTORY(CreateInputFieldFactory)


	CInputField::CInputField() :
		CBaseComponent()
	{
	}


	struct TInputFieldArchiveKeys
	{
		static const std::string mCursorEntityRefKeyId;
		static const std::string mLabelEntityRefKeyId;
		static const std::string mValueKeyId;
		static const std::string mCaretBlinkKeyId;
	};


	const std::string TInputFieldArchiveKeys::mCursorEntityRefKeyId = "cursor_entity_ref";
	const std::string TInputFieldArchiveKeys::mLabelEntityRefKeyId = "label_entity_ref";
	const std::string TInputFieldArchiveKeys::mValueKeyId = "value";
	const std::string TInputFieldArchiveKeys::mCaretBlinkKeyId = "caret_blink_rate";

	E_RESULT_CODE CInputField::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}

		mCursorEntityRef = static_cast<TEntityId>(pReader->GetUInt32(TInputFieldArchiveKeys::mCursorEntityRefKeyId));
		mLabelEntityRef = static_cast<TEntityId>(pReader->GetUInt32(TInputFieldArchiveKeys::mLabelEntityRefKeyId));
		mValue = pReader->GetString(TInputFieldArchiveKeys::mValueKeyId);
		mCaretBlinkRate = pReader->GetFloat(TInputFieldArchiveKeys::mCaretBlinkKeyId, 1.0f);

		return RC_OK;
	}

	E_RESULT_CODE CInputField::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		pWriter->BeginGroup("component");
		{
			pWriter->SetUInt32("type_id", static_cast<U32>(CInputField::GetTypeId()));

			pWriter->SetUInt32(TInputFieldArchiveKeys::mCursorEntityRefKeyId, static_cast<U32>(mCursorEntityRef));			
			pWriter->SetUInt32(TInputFieldArchiveKeys::mLabelEntityRefKeyId, static_cast<U32>(mLabelEntityRef));			
			pWriter->SetString(TInputFieldArchiveKeys::mValueKeyId, mValue);
			pWriter->SetFloat(TInputFieldArchiveKeys::mCaretBlinkKeyId, mCaretBlinkRate);
		}
		pWriter->EndGroup();

		return RC_OK;
	}

	E_RESULT_CODE CInputField::PostLoad(CEntityManager* pEntityManager, const TEntitiesMapper& entitiesIdentifiersRemapper)
	{
		mCursorEntityRef = entitiesIdentifiersRemapper.Resolve(mCursorEntityRef);
		mLabelEntityRef = entitiesIdentifiersRemapper.Resolve(mLabelEntityRef);

		return CBaseComponent::PostLoad(pEntityManager, entitiesIdentifiersRemapper);
	}

	E_RESULT_CODE CInputField::Clone(IComponent*& pDestObject) const
	{
		if (auto pComponent = dynamic_cast<CInputField*>(pDestObject))
		{
			pComponent->mCursorEntityRef = mCursorEntityRef;
			pComponent->mLabelEntityRef = mLabelEntityRef;
			pComponent->mValue = mValue;

			return RC_OK;
		}

		return RC_FAIL;
	}

	void CInputField::ResetChanges()
	{
		if (!mIsEditing)
		{
			return;
		}

		std::swap(mTempValue, mValue);
	}

	void CInputField::SetValue(const std::string& value)
	{
		(mIsEditing ? mTempValue : mValue) = value;
	}

	void CInputField::SetCursorEntityId(TEntityId cursorId)
	{
		mCursorEntityRef = cursorId;
	}
	
	void CInputField::SetLabelEntityId(TEntityId labelId)
	{
		mLabelEntityRef = labelId;
	}

	void CInputField::SetEditingFlag(bool state)
	{
		mIsEditing = state;
	}

	void CInputField::SetCaretPosition(I32 value)
	{
		mCurrCaretPosition = value;
	}

	void CInputField::SetFirstVisibleCharPosition(I32 value)
	{
		mFirstVisibleCharPosition = value;
	}

	void CInputField::SetLastVisibleCharPosition(I32 value)
	{
		mLastVisibleCharPosition = value;
	}

	void CInputField::SetCaretBlinkRate(F32 value)
	{
		mCaretBlinkRate = value;
	}

	void CInputField::SetCaretBlinkTimer(F32 value)
	{
		mCaretBlinkTimer = value;
	}

	const std::string& CInputField::GetValue() const
	{
		return mIsEditing ? mTempValue : mValue;
	}

	TEntityId CInputField::GetCursorEntityId() const
	{
		return mCursorEntityRef;
	}

	TEntityId CInputField::GetLabelEntityId() const
	{
		return mLabelEntityRef;
	}

	bool CInputField::IsEditing() const
	{
		return mIsEditing;
	}

	I32 CInputField::GetCaretPosition() const
	{
		return mCurrCaretPosition;
	}

	I32 CInputField::GetFirstVisibleCharPosition() const
	{
		return mFirstVisibleCharPosition;
	}

	I32 CInputField::GetLastVisibleCharPosition() const
	{
		return mLastVisibleCharPosition;
	}

	F32 CInputField::GetCaretBlinkRate() const
	{
		return mCaretBlinkRate;
	}

	F32 CInputField::GetCaretBlinkTimer() const
	{
		return mCaretBlinkTimer;
	}


	IComponent* CreateInputField(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CInputField, result);
	}


	/*!
		\brief CInputFieldFactory's definition
	*/

	CInputFieldFactory::CInputFieldFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* CInputFieldFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return CreateInputField(result);
	}

	E_RESULT_CODE CInputFieldFactory::SetupComponent(CInputField* pComponent, const TInputFieldParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}

		return RC_OK;
	}


	IComponentFactory* CreateInputFieldFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CInputFieldFactory, result);
	}
}