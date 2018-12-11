/*!
	\file IRenderable.h
	\date 07.12.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Utils.h"
#include "./../utils/Types.h"
#include "./../core/IBaseObject.h"


namespace TDEngine2
{
	/*!
		interface IRenderable

		\brief The interface describes a functionality of a renderable
		entities
	*/

	class IRenderable: public virtual IBaseObject
	{
		public:
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IRenderable)
	};
}