/*!
	\file CBaseCubemapTexture.h
	\date 03.10.2019
	\authors Kasimov Ildar
*/

#pragma once


#include "ICubemapTexture.h"
#include "./../core/CBaseObject.h"
#include "./../core/CBaseResource.h"


namespace TDEngine2
{
	/*!
		class CBaseCubemapTexture

		\brief The interface describes a common functionality of cubemap textures
		that should be expanded via GAPI plugins
	*/

	class CBaseCubemapTexture : public ICubemapTexture, public CBaseResource
	{
		public:
			TDE2_REGISTER_RESOURCE_TYPE(CBaseCubemapTexture)
			TDE2_REGISTER_TYPE(CBaseCubemapTexture)

			/*!
				\brief The method initializes an internal state of a cubemap texture. The method
				is used when we want just to load texture's data from some storage. In this
				case all the parameters will be executed automatically. To create a new blank
				texture object use overloaded version of Init()

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\param[in] name A resource's name

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name) override;

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

			TDE2_API void SetFilterType(const E_FILTER_TYPE& type) override;

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
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseCubemapTexture)

			TDE2_API virtual E_RESULT_CODE _createInternalTextureHandler(IGraphicsContext* pGraphicsContext, U32 width, U32 height, E_FORMAT_TYPE format,
																		 U32 mipLevelsCount, U32 samplesCount, U32 samplingQuality) = 0;
		protected:
			IGraphicsContext*   mpGraphicsContext;

			U32                 mWidth;

			U32                 mHeight;

			E_FORMAT_TYPE       mFormat;

			U32                 mNumOfMipLevels;

			U32                 mNumOfSamples;

			U32                 mSamplingQuality;

			TTextureSamplerDesc mTextureSamplerParams;

			TTextureSamplerId   mCurrTextureSamplerHandle = TTextureSamplerId::Invalid;
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
			friend TDE2_API IResourceLoader* CreateBaseCubemapTextureLoader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem,
																			E_RESULT_CODE& result);
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
}
