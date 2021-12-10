/*!
	\file ITrigger2D.h
	\date 18.11.2019
	\authors Kasimov Ildar
*/

#pragma once


#include "../../utils/Types.h"
#include "../../utils/Utils.h"
#include "../../ecs/IComponentFactory.h"
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
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ITrigger2D)
	};
}
