/*!
	\file IComponent.h
	\date 28.09.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "../core/CBaseObject.h"
#include "../core/Serialization.h"
#include "../utils/Utils.h"


namespace TDEngine2
{
	/*!
		interface IComponent

		\brief The interface describes a functionality of a component

		\note The interface is empty by now
	*/

	class IComponent: public virtual IBaseObject, public ISerializable
	{
		public:
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IComponent)
	};
}