/*!
	\brief CTextureAtlas.h
	\date 10.09.2019
	\authors Kasimov Ildar
*/

#pragma once


#include "ITextureAtlas.h"
#include "./../core/CBaseResource.h"
#include "./../math/TRect.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <limits>
#include <memory>


namespace TDEngine2
{
	class IFileSystem;
	class IGraphicsContext;
	class IResourceHandler;
	class ITexture2D;


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
		protected:
			struct TTextureAtlasEntry
			{
				struct TRawTextureData
				{
					const U8*     mpData;

					E_FORMAT_TYPE mFormat;
				};

				TDE2_API TTextureAtlasEntry(const std::string& name, const TRectI32& rect, const TRawTextureData& texture);

				std::string mName;
						    
				TRectI32    mRect;
						    
				bool        mIsRawData = false; ///< true means that mpData points to some object that's derived from CBaseTexture2D
						    
				union 
				{
					TRawTextureData mRawTexture;

					ITexture2D*     mpTexture;
				} mData;
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
		public:
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
				\brief The method finalizes the process of packing textures into the atlas.
				You should call it after all textures added into the atlas. True will be returned
				in case when there is no enough space for packing all textures

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Bake() override;

			/*!
				\brief The method writes the information about texture atlas's content down onto disk

				\param[in, out] pFileSystem A pointer to IFileSystem implementation
				\param[in] filename A name of a file into which the data will be written

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Serialize(IFileSystem* pFileSystem, const std::string& filename) override;

			/*!
				\brief The method restores state of the texture atlas based on information from a given file

				\param[in, out] pFileSystem A pointer to IFileSystem implementation
				\param[in] filename A name of a file into which the data will be written

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Deserialize(IFileSystem* pFileSystem, const std::string& filename) override;
						
			/*!
				\brief The method loads resource data into memory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Load() override;

			/*!
				\brief The method unloads resource data from memory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Unload() override;

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
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CTextureAtlas)
		protected:
			IGraphicsContext* mpGraphicsContext;

			IResourceHandler* mpTextureResource;

			TPendingDataArray mPendingData;

			TAtlasRegistry    mAtlasEntities;
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

	class CTextureAtlasLoader : public ITextureAtlasLoader
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
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

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

			TDE2_API U32 GetResourceTypeId() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CTextureAtlasLoader)
		protected:
			bool              mIsInitialized;

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

	class CTextureAtlasFactory : public ITextureAtlasFactory
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
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

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

			TDE2_API U32 GetResourceTypeId() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CTextureAtlasFactory)
		protected:
			bool              mIsInitialized;

			IResourceManager* mpResourceManager;

			IGraphicsContext* mpGraphicsContext;
	};
}
