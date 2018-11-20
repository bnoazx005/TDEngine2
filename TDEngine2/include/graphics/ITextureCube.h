/*!
	\file ITextureCube.h
	\date 20.11.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "ITexture.h"


namespace TDEngine2
{
	/*!
		interface ITextureCube

		\brief The interface describes a common functionality of cubemaps
	*/

	class ITextureCube : public ITexture
	{
		public:
			/*!
				\brief The method initializes an internal state of a cubempa texture

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init() = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ITextureCube)
	};


	/*!
		interface ITextureCubeLoader

		\brief The interface describes a functionality of a cubemap texture loader
	*/

	class ITextureCubeLoader : public IResourceLoader
	{
		public:
			/*!
			\brief The method initializes an inner state of an object

			\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init() = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ITextureCubeLoader)
	};
}
