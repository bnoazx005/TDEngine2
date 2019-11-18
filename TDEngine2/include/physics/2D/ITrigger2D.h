/*!
	\file ITrigger2D.h
	\date 18.11.2019
	\authors Kasimov Ildar
*/

#pragma once


#include "./../../utils/Types.h"
#include "./../../utils/Utils.h"
#include "./../../ecs/IComponentFactory.h"
#include "Box2D.h"


namespace TDEngine2
{
	/*!
		interface ITrigger2D

		\brief The interface represents a functionality of a 2D trigger
	*/

	class ITrigger2D
	{
		public:
			/*!
				\brief The method initializes an internal state of a component

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init() = 0;

		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ITrigger2D)
	};


	/*!
		interface ITrigger2DFactory

		\brief The interface represents a functionality of a factory of ITrigger2D objects
	*/

	class ITrigger2DFactory : public IComponentFactory
	{
		public:
			/*!
				\brief The method initializes an internal state of a factory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init() = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ITrigger2DFactory)
	};
}
