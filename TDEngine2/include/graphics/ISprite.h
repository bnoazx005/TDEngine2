/*!
	\file ISprite.h
	\date 18.11.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Types.h"
#include "../utils/Utils.h"
#include "../ecs/IComponentFactory.h"
#include "../graphics/IRenderable.h"
#include "../utils/Color.h"
#include <string>


namespace TDEngine2
{
	/*!
		interface ISprite

		\brief The interface describes a functionality of a 2D sprite component
		By now only in plans only quad sprites support
	*/

	class ISprite: public IRenderable
	{
		public:
			/*!
				\brief The method initializes an internal state of a quad sprite

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init() = 0;

			/*!
				\brief The method sets up an identifier fo a material that will be used for the sprite

				\param[in] materialName A string that contains material's name

				\param[in] materialId An identifier of a material 
			*/

			TDE2_API virtual void SetMaterialName(const std::string& materialName) = 0;

			/*!
				\brief The method sets up a color for a sprite

				\param[in] color A color that will be specified for a sprite
			*/

			TDE2_API virtual void SetColor(const TColor32F& color) = 0;

			/*!
				\brief The method returns an identifier of used material

				\return The method returns an identifier of used material
			*/

			TDE2_API virtual const std::string& GetMaterialName() const = 0;

			/*!
				\brief The method returns a color of a sprite

				\return The method returns a color of a sprite
			*/

			TDE2_API virtual const TColor32F& GetColor() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ISprite)
	};


	class ISpriteFactory: public IGenericComponentFactory<> {};
}