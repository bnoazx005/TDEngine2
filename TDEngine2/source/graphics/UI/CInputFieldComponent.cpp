#include "../../../include/graphics/UI/CInputFieldComponent.h"


namespace TDEngine2
{
	CInputField::CInputField() :
		CBaseComponent()
	{
	}


	struct TInputFieldArchiveKeys
	{
		static const std::string mCursorEntityRefKeyId;
		static const std::string mLabelEntityRefKeyId;
		static const std::string mValueKeyId;
	};


	const std::string TInputFieldArchiveKeys::mCursorEntityRefKeyId = "cursor_entity_ref";
	const std::string TInputFieldArchiveKeys::mLabelEntityRefKeyId = "label_entity_ref";
	const std::string TInputFieldArchiveKeys::mValueKeyId = "value";

	E_RESULT_CODE CInputField::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}

		mCursorEntityRef = static_cast<TEntityId>(pReader->GetUInt32(TInputFieldArchiveKeys::mCursorEntityRefKeyId));
		mLabelEntityRef = static_cast<TEntityId>(pReader->GetUInt32(TInputFieldArchiveKeys::mLabelEntityRefKeyId));
		mValue = pReader->GetString(TInputFieldArchiveKeys::mValueKeyId);

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

	void CInputField::SetValue(const std::string& value)
	{
		mValue = value;
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

	const std::string& CInputField::GetValue() const
	{
		return mValue;
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