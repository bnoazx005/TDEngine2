/*!
	\file IComponent.h
	\date 28.09.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../core/CBaseObject.h"


namespace TDEngine2
{
	/*!
		interface IComponent

		\brief The interface describes a functionality of a component

		\note The interface is empty by now
	*/

	class IComponent: public virtual IBaseObject
	{
		public:
		protected:
			TDE2_API IComponent() = default;
			TDE2_API virtual ~IComponent() = default;
			TDE2_API IComponent(const IComponent& component) = delete;
			TDE2_API virtual IComponent& operator=(const IComponent& component) = delete;
	};
}