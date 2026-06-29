/*!
	\file CInputReceiverComponent.h
	\date 21.05.2021
	\authors Kasimov Ildar
*/

#pragma once


#include "../../ecs/CBaseComponent.h"
#include "../../math/TVector2.h"
#include "../../math/TRect.h"


namespace TDEngine2
{
	enum class TResourceId : U32;


	enum class E_INPUT_ACTIONS : U32
	{
		MOVE_LEFT,
		MOVE_RIGHT,
		MOVE_UP,
		MOVE_DOWN,
		DELETE_CHAR,
		BACKSPACE,
		MOVE_HOME,
		MOVE_END,
		CHAR_INPUT,
		CANCEL_INPUT,
		SCROLL,
		NONE
	};


	CLASS_META(SECTION = ecs, flags = SERIALIZE_MARKED_ONLY_FIELDS)
	struct TInputReceiverComponentData
	{
		bool                                   mPrevState = false;
		bool                                   mCurrState = false;
		bool                                   mIsHovered : 1;
		bool                                   mIsFocused : 1;

		bool                                   mIsControlModifierActive : 1;
		bool                                   mIsShiftModifierActive : 1;

		FIELD_META(name = ignore_input) bool   mIsIgnoreInput = false;
		FIELD_META(name = bypass_enabled) bool mIsInputBypassEnabled : 1;

		E_INPUT_ACTIONS                        mActionType;

		TVector2                               mNormalizedInputPosition; ///< Position of an input event that's normalized relative to receiver's rectangle
		TVector3                               mMouseShiftVec = ZeroVector3; ///< z contains scroll delta

		std::string                            mInputBuffer;

		TDE2_DECLARE_COMPONENT_META(TInputReceiverComponentData);
	};


	/*!
		\brief A factory function for creation objects of CInputReceiver's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CInputReceiver's implementation
	*/

	TDE2_API IComponent* CreateInputReceiver(E_RESULT_CODE& result);


	/*!
		class CInputReceiver

		\brief The interface describes a functionality of UI elements that should react on input events.
		The best usage example is a button
	*/

	class CInputReceiver : public CBaseComponentT<CInputReceiver, TInputReceiverComponentData>
	{
		public:
			friend TDE2_API IComponent* CreateInputReceiver(E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CInputReceiver)

			/*!
				\return The method returns type name (lowercase is preffered)
			*/

			TDE2_API const std::string& GetTypeName() const override;

		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CInputReceiver)
	};


	/*!
		struct TInputReceiverParameters

		\brief The structure contains parameters for creation of CInputReceiver
	*/

	typedef struct TInputReceiverParameters : public TBaseComponentParameters
	{
	} TInputReceiverParameters;


	TDE2_DECLARE_COMPONENT_FACTORY(InputReceiver, TInputReceiverParameters);
}