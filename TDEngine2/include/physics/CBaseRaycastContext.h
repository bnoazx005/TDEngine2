/*!
	\file CBaseRaycastContext.h
	\date 23.02.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "./../core/CBaseObject.h"
#include "IRaycastContext.h"


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CBaseRaycastContext's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CBaseRaycastContext's implementation
	*/

	TDE2_API IRaycastContext* CreateBaseRaycastContext(E_RESULT_CODE& result);


	/*!
		class CBaseRaycastContext

		\brief The class implements world ray casting via 2D and 3D physics engines Box2D and Bullet3 respectively
	*/

	class CBaseRaycastContext : public CBaseObject, public IRaycastContext
	{
		public:
			friend TDE2_API IRaycastContext* CreateBaseRaycastContext(E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an internal state of a context

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init();

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			//TDE2_API virtual 
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseRaycastContext)
		protected:
	};
}