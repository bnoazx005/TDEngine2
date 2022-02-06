/*!
	\brief CTextureAtlas.h
	\date 10.09.2019
	\authors Kasimov Ildar
*/

#pragma once


#include "ITextureAtlas.h"
#include "../core/CBaseResource.h"
#include "../math/TRect.h"
#include "../../deps/stb/stb_rect_pack.h"
#include "variant.hpp"
#include <string>
#include <unordered_map>
#include <vector>
#include <limits>
#include <memory>


namespace TDEngine2
{
	class IFileSystem;
	class IGraphicsContext;
	class ITexture2D;
	class IAtlasSubTexture;


	TDE2_DECLARE_SCOPED_PTR(ITexture2D)


	/*!
		\brief A factory function for creation objects of CTextureAtlas's type

		\param[in, out] pResourceManager A pointer to IGraphicsContext's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[in] name A resource's name

		\param[in] params A parameters of created material

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CTextureAtlas's implementation
	*/

	TDE2_API ITextureAtlas* CreateTextureAtlas(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
											   const TTexture2DParameters& params, E_RESULT_CODE& result);

	/*!
		\brief A factory function for creation objects of CTextureAtlas's type

		\param[in, out] pResourceManager A pointer to IGraphicsContext's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[in] name A resource's name

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CTextureAtlas's implementation
	*/

	TDE2_API ITextureAtlas* CreateTextureAtlas(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
											   E_RESULT_CODE& result);



	/*!
		interface CTextureAtlas

		\brief The implementation represents a basic functionality of a texture atlases
	*/

	class CTextureAtlas : public ITextureAtlas, public CBaseResource
	{
		public:
			friend TDE2_API ITextureAtlas* CreateTextureAtlas(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
															  const TTexture2DParameters& params, E_RESULT_CODE& result);
			friend TDE2_API ITextureAtlas* CreateTextureAtlas(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
															  E_RESULT_CODE& result);
		public:
			struct TTextureAtlasEntry
			{
				struct TRawTextureData
				{
					const U8*     mpData;

					E_FORMAT_TYPE mFormat;
				};

				TDE2_API TTextureAtlasEntry(const std::string& name, const TRectI32& rect, const TRawTextureData& texture);
				TDE2_API TTextureAtlasEntry(const std::string& name, const TRectI32& rect, TPtr<ITexture2D> pTexture);

				std::string mName;
						    
				TRectI32    mRect;
						    
				bool        mIsRawData = false; ///< true means that mpData points to some object that's derived from CBaseTexture2D
						    
				Wrench::Variant<TRawTextureData, TPtr<ITexture2D>> mData;
			};

			struct TAtlasAreaEntry
			{
				TRectI32                         mBounds;

				U32                              mTextureEntryId = (std::numeric_limits<U32>::max)();

				/// \todo: Replace this unique_ptr with custom allocator and raw pointers later
				std::unique_ptr<TAtlasAreaEntry> mpLeft;

				std::unique_ptr<TAtlasAreaEntry> mpRight;
			};
		public:
			typedef std::vector<TTextureAtlasEntry>           TPendingDataArray;

			typedef std::unordered_map<std::string, TRectI32> TAtlasRegistry;

			typedef std::vector<IAtlasSubTexture*>            TAtlasSubTexturesArray;

			typedef std::vector<stbrp_node>                   TAtlasNodesArray;

			typedef std::unique_ptr<stbrp_context>            TAtlasContextPtr;

		public:
			TDE2_REGISTER_RESOURCE_TYPE(CTextureAtlas)
			TDE2_REGISTER_TYPE(CTextureAtlas)

			/*!
				\brief The method initializes an internal state of a texture atlas.

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\param[in] name A resource's name

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name) override;

			/*!
				\brief The method initializes a state of a brand new texture atlas.

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\param[in] name A resource's name

				\param[in] params Additional parameters of a texture

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
										const TTexture2DParameters& params) override;
		
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

			TDE2_API E_RESULT_CODE AddRawTexture(const std::string& name, U32 width, U32 height, E_FORMAT_TYPE format, const U8* pData) override;

			/*!
				\brief The method adds a specified texture into the atlas

				\param[in] textureHandle A handle of texture's resource

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE AddTexture(TResourceId textureHandle) override;

			TDE2_API E_RESULT_CODE RemoveTexture(const std::string& name) override;

			/*!
				\brief The method deserializes object's state from given reader

				\param[in, out] pReader An input stream of data that contains information about the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Load(IArchiveReader* pReader) override;

			/*!
				\brief The method serializes object's state into given stream

				\param[in, out] pWriter An output stream of data that writes information about the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Save(IArchiveWriter* pWriter) override;

			/*!
				\brief The method writes the information about texture atlas's content down onto disk

				\param[in, out] pFileSystem A pointer to IFileSystem implementation
				\param[in] filename A name of a file into which the data will be written

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API static E_RESULT_CODE Serialize(IFileSystem* pFileSystem, ITextureAtlas* pTextureAtlas, const std::string& filename);


			/*!
				\brief The method resets current internal data of a resource

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Reset() override;
			
			/*!
				\brief The method returns a pointer to texture that is used with texture atlas

				\return The method returns a pointer to texture that is used with texture atlas
			*/

			TDE2_API ITexture2D* GetTexture() const override;

			/*!
				\brief The method returns non-normalized uv coordinates of a texture based on its name

				\param[in] textureName A texture's name

				\return Either an object that contains coordinates or an error code
			*/

			TDE2_API TResult<TRectI32> GetTextureRect(const std::string& textureName) const override;

			/*!
				\brief The method returns normalized uv coordinates of a texture based on its name,
				each coordinate lies in range of 0 and 1

				\param[in] textureName A texture's name

				\return Either an object that contains coordinates or an error code
			*/

			TDE2_API TResult<TRectF32> GetNormalizedTextureRect(const std::string& textureName) const override;

			/*!
				\return The vector that contains an identifiers of included texture items
			*/

			TDE2_API std::vector<std::string> GetTexturesIdentifiersList() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CTextureAtlas)

			TDE2_API void _updateAtlasSizes(IResource* pTexture);

			TDE2_API TResult<TResourceId> _createSubTexture(const std::string& id, const TRectI32& rect);

			TDE2_API const TPtr<IResourceLoader> _getResourceLoader() override;

			/// The context is lazy evaluated entity, so always use the method to access it
			stbrp_context& _getAtlasEntitiesContext();

			E_RESULT_CODE _addNewEntityToAtlas(TTextureAtlasEntry&& entity);
		protected:
			IGraphicsContext*      mpGraphicsContext;

			TResourceId            mTextureResourceHandle;

			TAtlasContextPtr       mpAtlasRectsContext;
			
			TAtlasNodesArray       mAtlasNodes;

			TAtlasRegistry         mAtlasEntities;
			
			TAtlasSubTexturesArray mpSubTextures;

			U32                    mWidth;

			U32                    mHeight;
	};


	/*!
		\brief A factory function for creation objects of CTextureAtlasLoader's type

		\param[in, out] pResourceManager A pointer to IResourceManager's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[in, out] pFileSystem A pointer to IFileSystem's implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CTextureAtlasLoader's implementation
	*/

	TDE2_API IResourceLoader* CreateTextureAtlasLoader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem,
													   E_RESULT_CODE& result);


	/*!
		class CTextureAtlasLoader

		\brief The class is an implementation of a loader of texture atlases
	*/

	class CTextureAtlasLoader : public CBaseObject, public ITextureAtlasLoader
	{
		public:
			friend TDE2_API IResourceLoader* CreateTextureAtlasLoader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem,
																	  E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an inner state of an object

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\param[in, out] pFileSystem A pointer to IFileSystem's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem) override;

			/*!
				\brief The method loads data into the specified resource based on its
				internal information

				\param[in, out] pResource A pointer to an allocated resource

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE LoadResource(IResource* pResource) const override;

			/*!
				\brief The method returns an identifier of a resource's type, which
				the loader serves

				\return The method returns an identifier of a resource's type, which
				the loader serves
			*/

			TDE2_API TypeId GetResourceTypeId() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CTextureAtlasLoader)
		protected:
			IResourceManager* mpResourceManager;

			IFileSystem*      mpFileSystem;

			IGraphicsContext* mpGraphicsContext;
	};


	/*!
		\brief A factory function for creation objects of CTextureAtlasFactory's type

		\param[in, out] pResourceManager A pointer to IResourceManager's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CTextureAtlasFactory's implementation
	*/

	TDE2_API IResourceFactory* CreateTextureAtlasFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);


	/*!
		class CTextureAtlasFactory

		\brief The class is an implementation of a factory of texture atlases
	*/

	class CTextureAtlasFactory : public CBaseObject, public ITextureAtlasFactory
	{
		public:
			friend TDE2_API IResourceFactory* CreateTextureAtlasFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an internal state of a shader factory

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext) override;

			/*!
				\brief The method creates a new instance of a resource based on passed parameters

				\param[in] name A name of a resource

				\param[in] params An object that contains parameters that are needed for the resource's creation

				\return A pointer to a new instance of IResource type
			*/

			TDE2_API IResource* Create(const std::string& name, const TBaseResourceParameters& params) const override;

			/*!
				\brief The method creates a new instance of a resource based on passed parameters

				\param[in] name A name of a resource

				\param[in] params An object that contains parameters that are needed for the resource's creation

				\return A pointer to a new instance of IResource type
			*/

			TDE2_API IResource* CreateDefault(const std::string& name, const TBaseResourceParameters& params) const override;

			/*!
				\brief The method returns an identifier of a resource's type, which
				the factory serves

				\return The method returns an identifier of a resource's type, which
				the factory serves
			*/

			TDE2_API TypeId GetResourceTypeId() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CTextureAtlasFactory)
		protected:
			IResourceManager* mpResourceManager;

			IGraphicsContext* mpGraphicsContext;
	};
}
