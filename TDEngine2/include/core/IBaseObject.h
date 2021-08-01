/*!
	\file IBaseObject.h
	\date 28.09.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Types.h"
#include "../utils/Utils.h"


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
				\brief The method increment a value of the internal reference counter. You can ignore
				this method if you work in single-threaded manner. If you want to use an object from
				another thread you should increment its counter to prevent non thread-safe deletion by
				some of running threads.
			*/

			TDE2_API virtual void AddRef() = 0;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Free() = 0;

			/*!
				\return The method returns a number of strong references that points to the object
			*/

			TDE2_API virtual U32 GetRefCount() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IBaseObject)
	};
}