/*!
	\file ITexture.h
	\date 20.11.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Types.h"
#include "./../utils/Utils.h"


namespace TDEngine2
{
	/*!
		interface ITexture

		\brief The interface describes a common functionality that
		all texture types should provide within the engine
	*/

	class ITexture
	{
		public:
			/*!
				\brief The method binds a texture object to a given slot

				\param[in] slot Slot's index
			*/

			TDE2_API virtual void Bind(U32 slot) = 0;

			/*!
				\brief The method returns a width of a texture

				\return The method returns a width of a texture
			*/

			TDE2_API virtual U32 GetWidth() const = 0;

			/*!
				\brief The method returns a height of a texture

				\return The method returns a height of a texture
			*/

			TDE2_API virtual U32 GetHeight() const = 0;

			/*!
				\brief The method returns current format of the texture

				\return The method returns current format of the texture
			*/

			TDE2_API virtual E_FORMAT_TYPE GetFormat() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ITexture)
	};
}
