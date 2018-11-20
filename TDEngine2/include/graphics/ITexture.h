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
				\brief The method returns a width of a texture

				\return The method returns a width of a texture
			*/

			TDE2_API virtual U32 GetWidth() const = 0;

			/*!
				\brief The method returns a height of a texture

				\return The method returns a height of a texture
			*/

			TDE2_API virtual U32 GetHeight() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ITexture)
	};
}
