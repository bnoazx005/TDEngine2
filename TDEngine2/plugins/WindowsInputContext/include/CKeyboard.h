/*!
	/file CKeyboard.h
	/date 16.12.2018
	/authors Kasimov Ildar
*/

#pragma once


#include "CBaseInputDevice.h"


#if defined (TDE2_USE_WINPLATFORM)

namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CKeyboard's type

		\param[in, out] pInputContext A pointer to IInputContext implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CKeyboard's implementation
	*/

	TDE2_API IInputDevice* CreateKeyboardDevice(IInputContext* pInputContext, E_RESULT_CODE& result);


	/*!
		class CKeyboard

		\brief The class implements a functionality of a system keyboard
	*/

	class CKeyboard : public CBaseInputDevice, public IKeyboard
	{
		public:
			friend TDE2_API IInputDevice* CreateKeyboardDevice(IInputContext* pInputContext, E_RESULT_CODE& result);
		public:
			/*!
				\brief The method updates the current state of a device

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Update() override;

			/*!
				\brief The method polls input devices and checks up has some key been pressed

				\param[in] keyCode A key's code
			*/

			TDE2_API bool IsKeyPressed(E_KEYCODES keyCode) override;

			/*!
				\brief The method polls input devices and checks up is some key still being pressed

				\param[in] keyCode A key's code
			*/

			TDE2_API bool IsKey(E_KEYCODES keyCode) override;

			/*!
				\brief The method polls input devices and checks up has some key been unpressed

				\param[in] keyCode A key's code
			*/

			TDE2_API bool IsKeyUnpressed(E_KEYCODES keyCode) override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CKeyboard)

			TDE2_API E_RESULT_CODE _createInternalHandlers(const TInternalInputData& params) override;
		protected:
			IDirectInputDevice8* mpKeyboardDevice;

			static const U16     mKeysCount = 256;

			U8                   mKeyboardState[mKeysCount];

			U8                   mPrevKeyboardState[mKeysCount];

	};
}

#endif