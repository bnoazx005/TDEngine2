#include "../../../include/graphics/UI/CInputFieldComponent.h"
#define META_EXPORT_ECS_SECTION
#include "../../../include/metadata.h"


namespace TDEngine2
{
	TDE2_REGISTER_COMPONENT_FACTORY(CreateInputFieldFactory)
	TDE2_DEFINE_COMPONENT_META(TInputFieldComponentData)

	CInputField::CInputField() :
		CBaseComponentT()
	{
	}

	E_RESULT_CODE CInputField::PostLoad(CEntityManager* pEntityManager, const TEntitiesMapper& entitiesIdentifiersRemapper)
	{
		mData.mCursorEntityRef = entitiesIdentifiersRemapper.Resolve(mData.mCursorEntityRef);
		mData.mLabelEntityRef = entitiesIdentifiersRemapper.Resolve(mData.mLabelEntityRef);

		return CBaseComponentT::PostLoad(pEntityManager, entitiesIdentifiersRemapper);
	}

	void CInputField::ResetChanges()
	{
		if (!mData.mIsEditing)
		{
			return;
		}

		std::swap(mData.mTempValue, mData.mValue);
	}

	void CInputField::SetValue(const std::string& value)
	{
		(mData.mIsEditing ? mData.mTempValue : mData.mValue) = value;
	}

	void CInputField::SetCursorEntityId(TEntityId cursorId)
	{
		mData.mCursorEntityRef = cursorId;
	}
	
	void CInputField::SetLabelEntityId(TEntityId labelId)
	{
		mData.mLabelEntityRef = labelId;
	}

	void CInputField::SetEditingFlag(bool state)
	{
		mData.mIsEditing = state;
	}

	void CInputField::SetCaretPosition(I32 value)
	{
		mData.mCurrCaretPosition = value;
	}

	void CInputField::SetFirstVisibleCharPosition(I32 value)
	{
		mData.mFirstVisibleCharPosition = value;
	}

	void CInputField::SetLastVisibleCharPosition(I32 value)
	{
		mData.mLastVisibleCharPosition = value;
	}

	void CInputField::SetCaretBlinkRate(F32 value)
	{
		mData.mCaretBlinkRate = value;
	}

	void CInputField::SetCaretBlinkTimer(F32 value)
	{
		mData.mCaretBlinkTimer = value;
	}

	const std::string& CInputField::GetValue() const
	{
		return mData.mIsEditing ? mData.mTempValue : mData.mValue;
	}

	TEntityId CInputField::GetCursorEntityId() const
	{
		return mData.mCursorEntityRef;
	}

	TEntityId CInputField::GetLabelEntityId() const
	{
		return mData.mLabelEntityRef;
	}

	bool CInputField::IsEditing() const
	{
		return mData.mIsEditing;
	}

	I32 CInputField::GetCaretPosition() const
	{
		return mData.mCurrCaretPosition;
	}

	I32 CInputField::GetFirstVisibleCharPosition() const
	{
		return mData.mFirstVisibleCharPosition;
	}

	I32 CInputField::GetLastVisibleCharPosition() const
	{
		return mData.mLastVisibleCharPosition;
	}

	F32 CInputField::GetCaretBlinkRate() const
	{
		return mData.mCaretBlinkRate;
	}

	F32 CInputField::GetCaretBlinkTimer() const
	{
		return mData.mCaretBlinkTimer;
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