/*!
	\file CBaseCubemapTexture.h
	\date 03.10.2019
	\authors Kasimov Ildar
*/

#pragma once


#include "ICubemapTexture.h"
#include "../core/CBaseObject.h"
#include "../core/CBaseResource.h"


namespace TDEngine2
{
	enum class TTextureHandleId : U32;


	TDE2_API ICubemapTexture* CreateCubemapTexture(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
		const TTexture2DParameters& params, E_RESULT_CODE& result);


	/*!
		class CBaseCubemapTexture

		\brief The interface describes a common functionality of cubemap textures
	*/

	class CBaseCubemapTexture : public ICubemapTexture, public CBaseResource
	{
		public:
			friend TDE2_API ICubemapTexture* CreateCubemapTexture(IResourceManager*, IGraphicsContext*, const std::string&, const TTexture2DParameters&, E_RESULT_CODE&);
		public:
			TDE2_REGISTER_RESOURCE_TYPE(ICubemapTexture)
			TDE2_REGISTER_TYPE(CBaseCubemapTexture)

			/*!
				\brief The method initializes an internal state of a cubemap texture. The overloaded version of
				Init method creates a new texture object within memory with given width, height and format

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\param[in] name A resource's name

				\param[in] params Additional parameters of a texture

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
										const TTexture2DParameters& params) override;

			/*!
				\brief The method binds a texture object to a given slot

				\param[in] slot Slot's index
			*/

			TDE2_API void Bind(U32 slot) override;

			/*!
				\brief The method resets current internal data of a resource

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Reset() override;

			/*!
				\brief The method writes data into a specified texture's region

				\param[in] face An index of cubemap's face
				\param[in] regionRect A region, which will be overwritten
				\param[in] pData Data that will be written into a given region

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE WriteData(E_CUBEMAP_FACE face, const TRectI32& regionRect, const U8* pData) override;

			/*!
				\brief The method sets up wrapping mode for U axis
			*/

			TDE2_API void SetUWrapMode(const E_ADDRESS_MODE_TYPE& mode) override;

			/*!
				\brief The method sets up wrapping mode for Z axis
			*/

			TDE2_API void SetVWrapMode(const E_ADDRESS_MODE_TYPE& mode) override;

			/*!
				\brief The method sets up wrapping mode for W axis
			*/

			TDE2_API void SetWWrapMode(const E_ADDRESS_MODE_TYPE& mode) override;

			/*!
				\brief The method sets up a type of filtering read samples from the texture
			*/

			TDE2_API void SetFilterType(const E_TEXTURE_FILTER_TYPE& type) override;

			/*!
				\brief The method allows to mark one side of a cubemap as loaded one. When all the sides are loaded the resource
				is marked as loaded.
			*/

			TDE2_API void MarkFaceAsLoaded(E_CUBEMAP_FACE face) override;

			/*!
				\brief The method returns a width of a texture

				\return The method returns a width of a texture
			*/

			TDE2_API U32 GetWidth() const override;

			/*!
				\brief The method returns a height of a texture

				\return The method returns a height of a texture
			*/

			TDE2_API U32 GetHeight() const override;

			/*!
				\brief The method returns current format of the texture

				\return The method returns current format of the texture
			*/

			TDE2_API E_FORMAT_TYPE GetFormat() const override;

			/*!
				\return The method returns a rect of the texture in range [0.0f, 1.0f]
			*/

			TDE2_API TRectF32 GetNormalizedTextureRect() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseCubemapTexture)

			TDE2_API const TPtr<IResourceLoader> _getResourceLoader() override;

		protected:
			IGraphicsContext*   mpGraphicsContext;

			std::atomic_uint8_t mFacesLoadingStatusBitset; /// \todo Refactor this later, each bit determines whether a face loaded or not

			TTextureSamplerDesc mTextureSamplerParams;

			TTextureSamplerId   mCurrTextureSamplerHandle = TTextureSamplerId::Invalid;
			TTextureHandleId    mCurrTextureHandle;
	};


	/*!
		\brief A factory function for creation objects of CBaseCubemapTextureLoader's type

		\param[in, out] pResourceManager A pointer to IResourceManager's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[in, out] pFileSystem A pointer to IFileSystem's implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CBaseCubemapTextureLoader's implementation
	*/

	TDE2_API IResourceLoader* CreateBaseCubemapTextureLoader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem,
															 E_RESULT_CODE& result);


	/*!
		class CBaseCubemapTextureLoader

		\brief The class is a common implementation of a 2d texture loaded
	*/

	class CBaseCubemapTextureLoader : public CBaseObject, public ICubemapTextureLoader
	{
		public:
			friend TDE2_API IResourceLoader* CreateBaseCubemapTextureLoader(IResourceManager*, IGraphicsContext*, IFileSystem*,	E_RESULT_CODE&);
		protected:
			typedef struct TCubemapMetaInfo
			{
				E_FORMAT_TYPE mFormat;

				U32           mWidth;

				U32           mHeight;

				U16           mMipLevelsCount;

				std::string   mFaceTexturePaths[6];
			} TCubemapMetaInfo;
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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseCubemapTextureLoader)

			TDE2_API TResult<TCubemapMetaInfo> _readCubemapInfo(const std::string& filename) const;

			TDE2_API E_RESULT_CODE _loadFaceTexture(ICubemapTexture* pCubemapTexture, const TCubemapMetaInfo& info, E_CUBEMAP_FACE face) const;
		protected:
			IResourceManager* mpResourceManager;

			IFileSystem*      mpFileSystem;

			IGraphicsContext* mpGraphicsContext;
	};


	TDE2_API IResourceFactory* CreateBaseCubemapTextureFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);


	/*!
		class CBaseCubemapTextureFactory

		\brief The class is an abstract factory of CBaseCubemapTexture objects that is used by a resource manager
	*/

	class CBaseCubemapTextureFactory : public CBaseObject, public ICubemapTextureFactory
	{
		public:
			friend TDE2_API IResourceFactory* CreateBaseCubemapTextureFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an internal state of a shader factory

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext) override;

			/*!
				\brief The method creates a new instance of a resource BaseCubemapd on passed parameters

				\param[in] name A name of a resource

				\param[in] params An object that contains parameters that are needed for the resource's creation

				\return A pointer to a new instance of IResource type
			*/

			TDE2_API IResource* Create(const std::string& name, const TBaseResourceParameters& params) const override;

			/*!
				\brief The method creates a new instance of a resource BaseCubemapd on passed parameters

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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseCubemapTextureFactory)
		protected:
			IResourceManager* mpResourceManager;

			IGraphicsContext* mpGraphicsContext;
	};
}
