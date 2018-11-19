/*!
	\file ISprite.h
	\date 18.11.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Types.h"
#include "./../utils/Utils.h"
#include "./../ecs/IComponentFactory.h"


namespace TDEngine2
{
	/*!
		interface ISprite

		\brief The interface describes a functionality of a 2D sprite component
		By now only in plans only quad sprites support
	*/

	class ISprite
	{
		public:
			/*!
				\brief The method initializes an internal state of a quad sprite

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init() = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ISprite)
	};


	/*!
		interface ISpriteFactory

		\brief The interface represents a functionality of a factory of ISprite objects
	*/

	class ISpriteFactory : public IComponentFactory
	{
		public:
			/*!
				\brief The method initializes an internal state of a factory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init() = 0;
		protected:
	};
}