/*!
	\file CBaseObject.h
	\date 28.09.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "IBaseObject.h"


namespace TDEngine2
{
	/*!
		class CBaseObject
		
		\brief The class implements a base object for the engine.
	*/

	class CBaseObject: public virtual IBaseObject
	{
		public:
			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Free() = 0;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseObject)
		protected:
			bool mIsInitialized;
	};
}