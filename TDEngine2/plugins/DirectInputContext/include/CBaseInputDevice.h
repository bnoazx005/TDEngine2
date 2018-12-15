/*!
	/file CBaseInputDevice.h
	/date 16.12.2018
	/authors Kasimov Ildar
*/

#pragma once


#include "IInputDevice.h"
#include <core/CBaseObject.h>


#if defined (TDE2_USE_WIN32PLATFORM)

namespace TDEngine2
{
	/*!
		class CBaseInputDevice

		\brief The class implements a functionality of a system keyboard
	*/

	class CBaseInputDevice : public virtual IInputDevice, public CBaseObject
	{
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
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseInputDevice)

			TDE2_API E_RESULT_CODE _acquireDevice();

			TDE2_API virtual E_RESULT_CODE _createInternalHandlers(const TInternalInputData& params) = 0;
		protected:
			IDirectInputDevice8* mpInputDevice;
	};
}

#endif