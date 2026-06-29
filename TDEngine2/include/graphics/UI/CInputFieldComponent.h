/*!
	\file CInputFieldComponent.h
	\date 24.03.2023
	\authors Kasimov Ildar
*/

#pragma once


#include "../../ecs/CBaseComponent.h"
#include "../../ecs/CEntity.h"


namespace TDEngine2
{
	CLASS_META(SECTION = ecs, flags = SERIALIZE_MARKED_ONLY_FIELDS)
	struct TInputFieldComponentData
	{
		FIELD_META(name = cursor_entity_ref) TEntityId mCursorEntityRef;
		FIELD_META(name = label_entity_ref) TEntityId  mLabelEntityRef;

		FIELD_META(name = value) std::string           mValue;
		std::string                                    mTempValue; ///< Used to revert previous changes when a user cancels input

		bool                                           mIsEditing = false;

		I32                                            mCurrCaretPosition = 0;
		I32                                            mFirstVisibleCharPosition = 0;
		I32                                            mLastVisibleCharPosition = 0;

		FIELD_META(name = caret_blink_rate) F32        mCaretBlinkRate = 1.0f;
		F32                                            mCaretBlinkTimer = 0.0f;

		TDE2_DECLARE_COMPONENT_META(TInputFieldComponentData);
	};


	/*!
		\brief A factory function for creation objects of CInputField's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CInputField's implementation
	*/

	TDE2_API IComponent* CreateInputField(E_RESULT_CODE& result);


	/*!
		class CInputField

		\brief The implementation of a UI editable element
	*/

	class CInputField : public CBaseComponentT<CInputField, TInputFieldComponentData>
	{
		public:
			friend TDE2_API IComponent* CreateInputField(E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CInputField)

			/*!
				\brief The method is called after all entities of particular scene were loaded. It remaps all identifiers to
				make them correctly corresponds to saved state

				\param[in, out] pEntityManager A pointer to entities manager
				\param[in] entitiesIdentifiersRemapper A structure that maps saved identifier to current runtime equivalent
			*/

			TDE2_API E_RESULT_CODE PostLoad(CEntityManager* pEntityManager, const TEntitiesMapper& entitiesIdentifiersRemapper) override;

			TDE2_API void SetValue(const std::string& value);
			TDE2_API void SetCursorEntityId(TEntityId cursorId);
			TDE2_API void SetLabelEntityId(TEntityId labelId);

			TDE2_API void SetEditingFlag(bool state);
			TDE2_API void ResetChanges();

			TDE2_API void SetCaretPosition(I32 value);
			TDE2_API void SetFirstVisibleCharPosition(I32 value);
			TDE2_API void SetLastVisibleCharPosition(I32 value);
			TDE2_API void SetCaretBlinkRate(F32 value);
			TDE2_API void SetCaretBlinkTimer(F32 value);

			TDE2_API const std::string& GetValue() const;
			TDE2_API TEntityId GetCursorEntityId() const;
			TDE2_API TEntityId GetLabelEntityId() const;

			TDE2_API bool IsEditing() const;

			TDE2_API I32 GetCaretPosition() const;
			TDE2_API I32 GetFirstVisibleCharPosition() const;
			TDE2_API I32 GetLastVisibleCharPosition() const;
			TDE2_API F32 GetCaretBlinkRate() const;
			TDE2_API F32 GetCaretBlinkTimer() const;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CInputField)
	};


	/*!
		struct TInputFieldParameters

		\brief The structure contains parameters for creation of CInputField
	*/

	typedef struct TInputFieldParameters : public TBaseComponentParameters
	{
	} TInputFieldParameters;


	TDE2_DECLARE_COMPONENT_FACTORY(InputField, TInputFieldParameters);
}