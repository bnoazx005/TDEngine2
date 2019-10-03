/*!
	\file ICubemapTexture.h
	\date 20.11.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "ITexture.h"
#include "../core/IResourceLoader.h"
#include "../core/IResourceFactory.h"
#include <string>


namespace TDEngine2
{
	class IResourceManager;
	class IGraphicsContext;

	/*!
		interface ICubemapTexture

		\brief The interface describes a common functionality of cubemaps
	*/

	class ICubemapTexture : public ITexture
	{
		public:
			/*!
				\brief The method initializes an internal state of a 2d texture. The method
				is used when we want just to load texture's data from some storage. In this
				case all the parameters will be executed automatically. To create a new blank
				texture object use overloaded version of Init()

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\param[in] name A resource's name

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ICubemapTexture)
	};


	/*!
		interface ICubemapTextureLoader

		\brief The interface describes a functionality of a cubemap texture loader
	*/

	class ICubemapTextureLoader : public IResourceLoader
	{
		public:
			/*!
				\brief The method initializes an inner state of an object

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\param[in, out] pFileSystem A pointer to IFileSystem's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ICubemapTextureLoader)
	};


	/*!
		interface ICubemapTextureFactory

		\brief The interface describes a functionality of a Cube texture factory
	*/

	class ICubemapTextureFactory: public IResourceFactory
	{
		public:
			/*!
				\brief The method initializes an internal state of a shader factory

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ICubemapTextureFactory)
	};
}
