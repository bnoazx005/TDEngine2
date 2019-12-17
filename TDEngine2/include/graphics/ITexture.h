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
				\brief The method sets up wrapping mode for U axis
			*/

			TDE2_API virtual void SetUWrapMode(const E_ADDRESS_MODE_TYPE& mode) = 0;
			
			/*!
				\brief The method sets up wrapping mode for Z axis
			*/
			
			TDE2_API virtual void SetVWrapMode(const E_ADDRESS_MODE_TYPE& mode) = 0;
			
			/*!
				\brief The method sets up wrapping mode for W axis
			*/

			TDE2_API virtual void SetWWrapMode(const E_ADDRESS_MODE_TYPE& mode) = 0;

			/*!
				\brief The method sets up a type of filtering read samples from the texture
			*/

			TDE2_API virtual void SetFilterType(const E_FILTER_TYPE& type) = 0;

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
