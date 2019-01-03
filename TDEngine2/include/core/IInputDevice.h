/*!
	/file IInputDevice.h
	/date 16.12.2018
	/authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Types.h"
#include "./../utils/Utils.h"
#include "./../core/IBaseObject.h"
#include "./../math/TVector2.h"
#include "./../math/TVector3.h"
#include "IInputContext.h"


namespace TDEngine2
{
	/*!
		interface IInputDevice

		\brief The interface describes a functionality of an input device
	*/

	class IInputDevice: public virtual IBaseObject
	{
		public:
			/*!
				\brief The method initializes an initial state of an input device

				\param[in, out] pInputContext A pointer to IInputContext implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IInputContext* pInputContext) = 0;

			/*!
				\brief The method updates the current state of a device

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Update() = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IInputDevice)
	};


	/*!
		interface IKeyboard

		\brief The interface describes a functionality of a keyboard
	*/

	class IKeyboard : public virtual IInputDevice
	{
		public:
			/*!
				\brief The method polls input devices and checks up has some key been pressed

				\param[in] keyCode A key's code
			*/

			TDE2_API virtual bool IsKeyPressed(E_KEYCODES keyCode) = 0;

			/*!
				\brief The method polls input devices and checks up is some key still being pressed

				\param[in] keyCode A key's code
			*/

			TDE2_API virtual bool IsKey(E_KEYCODES keyCode) = 0;

			/*!
				\brief The method polls input devices and checks up has some key been unpressed

				\param[in] keyCode A key's code
			*/

			TDE2_API virtual bool IsKeyUnpressed(E_KEYCODES keyCode) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IKeyboard)
	};


	/*!
		interface IMouse

		\brief The interface describes a functionality of a system mouse
	*/

	class IMouse : public virtual IInputDevice
	{
		public:
			/*!
				\brief The method polls input devices and checks up has some button been pressed

				\param[in] button A button's index
			*/

			TDE2_API virtual bool IsButtonPressed(U8 button) = 0;

			/*!
				\brief The method polls input devices and checks up is some button still being pressed

				\param[in] button A button's index
			*/

			TDE2_API virtual bool IsButton(U8 button) = 0;

			/*!
				\brief The method polls input devices and checks up has some button been unpressed

				\param[in] button A button's index
			*/

			TDE2_API virtual bool IsButtonUnpressed(U8 button) = 0;

			/*!
				\brief The method returns a position of a cursor

				\return The method returns a position of a cursor
			*/

			TDE2_API virtual TVector3 GetMousePosition() const = 0;

			/*!
				\brief The method returns a 3d vector that's composed from
				shift values of each mouse axis

				\return The method returns a 3d vector that's composed from
				shift values of each mouse axis
			*/

			TDE2_API virtual TVector3 GetMouseShiftVec() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IMouse)
	};


	/*!
		interface IGamepad

		\brief The interface represents a functionality of a gamepad controller
	*/

	class IGamepad : public virtual IInputDevice
	{
		public:
			/*!
				\brief The method initializes an initial state of an input device

				\param[in, out] pInputContext A pointer to IInputContext implementation

				\param[in] gamepadId An identifier of a gamepad

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IInputContext* pInputContext, U8 gamepadId) = 0;

			/*!
				\brief The method polls input devices and checks up has some button been pressed

				\param[in] button A button's index
			*/

			TDE2_API virtual bool IsButtonPressed(E_GAMEPAD_BUTTONS button) const = 0;

			/*!
				\brief The method polls input devices and checks up is some button still being pressed

				\param[in] button A button's index
			*/

			TDE2_API virtual bool IsButton(E_GAMEPAD_BUTTONS button) const = 0;

			/*!
				\brief The method polls input devices and checks up has some button been unpressed

				\param[in] button A button's index
			*/

			TDE2_API virtual bool IsButtonUnpressed(E_GAMEPAD_BUTTONS button) const = 0;

			/*!
				\brief The method returns current value that is produced by L trigger button 
				(The value is normalized and stays between 0 and 1)

				\return The method returns current value that is produced by L trigger button
			*/

			TDE2_API virtual F32 GetLTriggerValue() const = 0;

			/*!
				\brief The method returns current value that is produced by R trigger button 
				(The value is normalized and stays between 0 and 1)

				\return The method returns current value that is produced by R trigger button
			*/

			TDE2_API virtual F32 GetRTriggerValue() const = 0;
			
			/*!
				\brief The method returns a vector which represents a left 
				thumb stick's offset from its origin position

				\return The method returns a vector which represents a left
				thumb stick's offset from its origin position
			*/

			TDE2_API virtual TVector2 GetLThumbShiftVec() const = 0;

			/*!
				\brief The method returns a vector which represents a right 
				thumb stick's offset from its origin position

				\return The method returns a vector which represents a right
				thumb stick's offset from its origin position
			*/

			TDE2_API virtual TVector2 GetRThumbShiftVec() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IGamepad)
	};
}