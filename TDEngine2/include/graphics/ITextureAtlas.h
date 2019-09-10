/*!
	\brief ITextureAtlas.h
	\date 10.09.2019
	\authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Types.h"
#include "./../utils/Config.h"
#include "./../core/IResource.h"
#include "./../core/IResourceLoader.h"
#include "./../core/IResourceFactory.h"
#include "ITexture.h"


namespace TDEngine2
{
	class IResourceManager;
	class IGraphicsContext;
	class IFileSystem;

	struct TTexture2DParameters;


	/*!
		interface ITextureAtlas

		\brief The interface describes a common functionality of a texture atlases
	*/

	class ITextureAtlas: public ITexture
	{
		public:
			/*!
				\brief The method initializes an internal state of a texture atlas. 

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\param[in] name A resource's name

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name) = 0;

			/*!
				\brief The method initializes a state of a brand new texture atlas.

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\param[in] name A resource's name

				\param[in] params Additional parameters of a texture

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
												const TTexture2DParameters& params) = 0;

		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ITextureAtlas)
	};


	/*!
		interface ITextureAtlasLoader

		\brief The interface describes a functionality of a loaded of texture atlases
	*/

	class ITextureAtlasLoader : public IResourceLoader
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
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ITextureAtlasLoader)
	};


	/*!
		interface ITextureAtlasFactory

		\brief The interface describes a functionality of a factory of texture atlases
	*/

	class ITextureAtlasFactory : public IResourceFactory
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
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ITextureAtlasFactory)
	};
}
