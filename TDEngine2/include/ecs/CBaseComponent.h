/*!
	\file CBaseComponent.h
	\date 28.09.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "IComponent.h"
#include "./../core/CBaseObject.h"


namespace TDEngine2
{
	/*!
		class CBaseComponent

		\brief The class is a base class for all the components, which can be
		declared.

		The example of custom component declaration:

		class CCustomComponent: public CBaseComponent<CCustomComponent>
		{
			//...
		};

		The following statement is needed to implement proper registration of component's type
	*/

	class CBaseComponent: public IComponent, public CBaseObject
	{
		public:
			TDE2_REGISTER_TYPE(CBaseComponent)

			/*!
				\brief The method initializes an entity

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init();

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseComponent)
	};
}