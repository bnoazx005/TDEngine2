/*!
	\file IRaycastContext.h
	\date 23.02.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Types.h"
#include "./../utils/Config.h"
#include "./../core/IBaseObject.h"


namespace TDEngine2
{
	/*!
		interface IRaycastContext

		\brief The interface describes functionality of world ray casting
	*/

	class IRaycastContext: public virtual IBaseObject
	{
		public:
			//TDE2_API virtual 
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IRaycastContext)
	};
}