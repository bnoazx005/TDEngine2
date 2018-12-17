/*!
	/file IInputDevice.h
	/date 16.12.2018
	/authors Kasimov Ildar
*/

#pragma once


#include <utils/Types.h>
#include <utils/Utils.h>
#include <core/IBaseObject.h>
#include <math/TVector3.h>
#include "CWindowsInputContext.h"


#if defined (TDE2_USE_WIN32PLATFORM)

namespace TDEngine2
{
	class IInputContext;


	/*!
		interface IInputDevice

		\brief The interface describes a functionality of an input device which
		is based on IDirectInputDevice's API
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
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IMouse)
	};
}

#endif