/*!
	/file CGamepad.h
	/date 16.12.2018
	/authors Kasimov Ildar
*/

#pragma once


#include <core/CBaseObject.h>
#include "IInputDevice.h"


#if defined (TDE2_USE_WIN32PLATFORM)

#include <XInput.h>


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CGamepad's type

		\param[in, out] pInputContext A pointer to IInputContext implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CGamepad's implementation
	*/

	TDE2_API IInputDevice* CreateGamepadDevice(IInputContext* pInputContext, E_RESULT_CODE& result);


	/*!
		class CGamepad

		\brief The class implements a functionality of a XInput compatible gamepad
	*/

	class CGamepad : public CBaseObject, public IGamepad
	{
		public:
			friend TDE2_API IInputDevice* CreateGamepadDevice(IInputContext* pInputContext, E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an initial state of a keyboard device

				\param[in, out] pInputContext A pointer to IInputContext implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IInputContext* pInputContext) override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method updates the current state of a device

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Update() override;

			/*!
				\brief The method polls input devices and checks up has some button been pressed

				\param[in] button A button's index
			*/

			TDE2_API bool IsButtonPressed(E_GAMEPAD_BUTTONS button) const override;

			/*!
				\brief The method polls input devices and checks up is some button still being pressed

				\param[in] button A button's index
			*/

			TDE2_API bool IsButton(E_GAMEPAD_BUTTONS button) const override;

			/*!
				\brief The method polls input devices and checks up has some button been unpressed

				\param[in] button A button's index
			*/

			TDE2_API bool IsButtonUnpressed(E_GAMEPAD_BUTTONS button) const override;

			/*!
				\brief The method returns current value that is produced by L trigger button
				(The value is normalized and stays between 0 and 1)

				\return The method returns current value that is produced by L trigger button
			*/

			TDE2_API F32 GetLTriggerValue() const override;

			/*!
				\brief The method returns current value that is produced by R trigger button
				(The value is normalized and stays between 0 and 1)

				\return The method returns current value that is produced by R trigger button
			*/

			TDE2_API F32 GetRTriggerValue() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CGamepad)
		protected:
			XINPUT_STATE mPrevGamepadState;

			XINPUT_STATE mCurrGamepadState;

			U16          mGamepadId;
	};
}

#endif