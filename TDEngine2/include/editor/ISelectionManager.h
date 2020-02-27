/*!
	\file ISelectionManager.h
	\date 27.02.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Config.h"
#include "../utils/Types.h"
#include "../core/IBaseObject.h"


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	/*!
		interface ISelectionManager

		\brief The interface describes a functionality of a manager that stores information
		about selected objects
	*/
	
	class ISelectionManager: public virtual IBaseObject
	{
		public:
			/*!
				\brief The method initializes the internal state of the manager

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init() = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ISelectionManager)
	};
}

#endif