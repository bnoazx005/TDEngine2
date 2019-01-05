/*!
	/file CMouse.h
	/date 16.12.2018
	/authors Kasimov Ildar
*/

#pragma once


#include "CBaseInputDevice.h"
#include <math/TRect.h>


#if defined (TDE2_USE_WIN32PLATFORM)

namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CMouse's type

		\param[in, out] pInputContext A pointer to IInputContext implementation
		
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CMouse's implementation
	*/

	TDE2_API IInputDevice* CreateMouseDevice(IInputContext* pInputContext, E_RESULT_CODE& result);


	/*!
		class CMouse

		\brief The class implements a functionality of a system mouse
	*/

	class CMouse : public CBaseInputDevice, public IMouse
	{
		public:
			friend TDE2_API IInputDevice* CreateMouseDevice(IInputContext* pInputContext, E_RESULT_CODE& result);
		public:
			/*!
				\brief The method updates the current state of a device

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Update() override;

			/*!
				\brief The method polls input devices and checks up has some button been pressed

				\param[in] button A button's index
			*/

			TDE2_API bool IsButtonPressed(U8 button) override;

			/*!
				\brief The method polls input devices and checks up is some button still being pressed

				\param[in] button A button's index
			*/

			TDE2_API bool IsButton(U8 button) override;

			/*!
				\brief The method polls input devices and checks up has some button been unpressed

				\param[in] button A button's index
			*/

			TDE2_API bool IsButtonUnpressed(U8 button) override;
			
			/*!
				\brief The method returns a position of a cursor

				\return The method returns a position of a cursor
			*/

			TDE2_API TVector3 GetMousePosition() const override;

			/*!
				\brief The method returns a 3d vector that's composed from
				shift values of each mouse axis

				\return The method returns a 3d vector that's composed from
				shift values of each mouse axis
			*/

			TDE2_API TVector3 GetMouseShiftVec() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CMouse)

			TDE2_API E_RESULT_CODE _createInternalHandlers(const TInternalInputData& params) override;
		protected:
			DIMOUSESTATE2 mPrevMouseState;

			DIMOUSESTATE2 mCurrMouseState;
	};
}

#endif