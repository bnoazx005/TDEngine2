/*!
	\file IBaseObject.h
	\date 28.09.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Types.h"
#include "./../utils/Utils.h"


namespace TDEngine2
{
	/*!
		interface IBaseObject

		\brief The interface describes a functionality of 
		a base object used in the engine
	*/

	class IBaseObject
	{
		public:
			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Free() = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IBaseObject)
	};
}