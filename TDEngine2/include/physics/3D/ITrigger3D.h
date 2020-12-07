/*!
	\file ITrigger3D.h
	\date 07.12.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "../../utils/Types.h"
#include "../../utils/Utils.h"
#include "../../ecs/IComponentFactory.h"


namespace TDEngine2
{
	/*!
		interface ITrigger3D

		\brief The interface represents a functionality of a 3D trigger
	*/

	class ITrigger3D
	{
	public:
		/*!
			\brief The method initializes an internal state of a component

			\return RC_OK if everything went ok, or some other code, which describes an error
		*/

		TDE2_API virtual E_RESULT_CODE Init() = 0;

	protected:
		DECLARE_INTERFACE_PROTECTED_MEMBERS(ITrigger3D)
	};


	/*!
		interface ITrigger3DFactory

		\brief The interface represents a functionality of a factory of ITrigger3D objects
	*/

	class ITrigger3DFactory : public IComponentFactory
	{
	public:
		/*!
			\brief The method initializes an internal state of a factory

			\return RC_OK if everything went ok, or some other code, which describes an error
		*/

		TDE2_API virtual E_RESULT_CODE Init() = 0;
	protected:
		DECLARE_INTERFACE_PROTECTED_MEMBERS(ITrigger3DFactory)
	};
}
