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
#include "./../math/TRect.h"


namespace TDEngine2
{
	class IResourceManager;
	class IGraphicsContext;
	class IFileSystem;
	class ITexture2D;

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

			/*!
				\brief The method adds raw image data into the atlas. A raw data means that we use raw byte representation instead
				of high-level texture API

				\param[in] name An identifier of an image
				\param[in] width A width of an image
				\param[in] height A height of an image
				\param[in] format A format of a single image's pixel
				\param[in] pData A pointer to image's data
				
				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE AddRawTexture(const std::string& name, U32 width, U32 height, E_FORMAT_TYPE format, const U8* pData) = 0;

			/*!
				\brief The method finalizes the process of packing textures into the atlas.
				You should call it after all textures added into the atlas. True will be returned
				in case when there is no enough space for packing all textures

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Bake() = 0;

			/*!
				\brief The method writes the information about texture atlas's content down onto disk

				\param[in, out] pFileSystem A pointer to IFileSystem implementation
				\param[in] filename A name of a file into which the data will be written

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Serialize(IFileSystem* pFileSystem, const std::string& filename) = 0;

			/*!
				\brief The method returns a pointer to texture that is used with texture atlas

				\return The method returns a pointer to texture that is used with texture atlas
			*/

			TDE2_API virtual ITexture2D* GetTexture() const = 0;
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
