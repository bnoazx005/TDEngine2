/*!
	/file CDirectInputMappings.h
	/date 16.12.2018
	/authors Kasimov Ildar
*/

#pragma once


#include <utils/Types.h>
#include "CWindowsInputContext.h"


#if defined(TDE2_USE_WIN32PLATFORM)

namespace TDEngine2
{
	/*!
		class CDirectInputMappings

		\brief The static class contains a set of helper functions to
		provide mappings from the internal enumerations and formats into
		DirectInput API specific ones
	*/

	class CDirectInputMappings
	{
		public:
			/*!
				\brief The method converts the internal format's values into DirectInput specific ones

				\param[in] keyCode A value of a key code

				\return A value of key that is specific for DirectInput library
			*/

			TDE2_API static U16 GetKeyCode(E_KEYCODES keyCode);

			/*!
				\brief The method converts the internal value of E_GAMEPAD_BUTTONS into XInput specific one

				\param[in] button A button's index
				
				\return A value of a button that is specific for XInput library
			*/

			TDE2_API static U16 GetGamepadButton(E_GAMEPAD_BUTTONS button);
	};
}

#endif
