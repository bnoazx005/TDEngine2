/*!
	\brief ITextureAtlas.h
	\date 10.09.2019
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Types.h"
#include "../utils/Config.h"
#include "../core/IResource.h"
#include "../core/IResourceLoader.h"
#include "../core/IResourceFactory.h"
#include "../core/Serialization.h"
#include "../math/TRect.h"
#include "ITexture.h"
#include <vector>
#include <string>


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

	class ITextureAtlas: public ISerializable, public virtual IBaseObject
	{
		public:
			enum class E_TEXTURE_ATLAS_ENTITY_ORDER : U8
			{
				SIZE, ///< When ITextureAtlas::Bake is called all entities will be sorted from biggest textures to little ones
				ID,   ///< Entities will be added into the texture atlas by lexicographical order of their identifiers
			};
		public:
			TDE2_REGISTER_TYPE(ITextureAtlas);

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
				\brief The method adds a specified texture into the atlas

				\param[in] textureHandle A handle of texture's resource

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE AddTexture(TResourceId textureHandle) = 0;

			TDE2_API virtual E_RESULT_CODE RemoveTexture(const std::string& name) = 0;

			/*!
				\brief The method finalizes the process of packing textures into the atlas.
				You should call it after all textures added into the atlas. True will be returned
				in case when there is no enough space for packing all textures

				\param[in] entitiesOrder Determines how the entities will be ordered before they will be added into the atlas

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Bake(E_TEXTURE_ATLAS_ENTITY_ORDER entitiesOrder = E_TEXTURE_ATLAS_ENTITY_ORDER::SIZE) = 0;

			/*!
				\brief The method returns a pointer to texture that is used with texture atlas

				\return The method returns a pointer to texture that is used with texture atlas
			*/

			TDE2_API virtual ITexture2D* GetTexture() const = 0;

			/*!
				\brief The method returns non-normalized uv coordinates of a texture based on its name

				\param[in] textureName A texture's name

				\return Either an object that contains coordinates or an error code
			*/

			TDE2_API virtual TResult<TRectI32> GetTextureRect(const std::string& textureName) const = 0;

			/*!
				\brief The method returns normalized uv coordinates of a texture based on its name,
				each coordinate lies in range of 0 and 1

				\param[in] textureName A texture's name

				\return Either an object that contains coordinates or an error code
			*/

			TDE2_API virtual TResult<TRectF32> GetNormalizedTextureRect(const std::string& textureName) const = 0;

			/*!
				\return The vector that contains an identifiers of included texture items
			*/

			TDE2_API virtual std::vector<std::string> GetTexturesIdentifiersList() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ITextureAtlas)
	};


	class ITextureAtlasLoader : public IGenericResourceLoader<IResourceManager*, IGraphicsContext*, IFileSystem*> {};
	class ITextureAtlasFactory : public IGenericResourceFactory<IResourceManager*, IGraphicsContext*> {};
}
