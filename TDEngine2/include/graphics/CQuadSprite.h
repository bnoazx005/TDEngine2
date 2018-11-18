/*!
	\file CQuadSprite.h
	\date 18.11.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "ISprite.h"
#include "./../ecs/CBaseComponent.h"


namespace TDEngine2
{
	/*!
		interface ISprite

		\brief The interface describes a functionality of a 2D sprite component
		By now only in plans only quad sprites support
	*/

	class CQuadSprite: public ISprite, public CBaseComponent
	{
		public:
			TDE2_REGISTER_TYPE(CQuadSprite)
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(CQuadSprite)
		protected:
	};
}