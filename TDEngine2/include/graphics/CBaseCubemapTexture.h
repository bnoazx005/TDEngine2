/*!
	\file CBaseCubemapTexture.h
	\date 03.10.2019
	\authors Kasimov Ildar
*/

#pragma once


#include "ICubemapTexture.h"
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
		TDE2_REGISTER_TYPE(CBaseCubemapTexture)

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

		TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name) override;

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
		IGraphicsContext* mpGraphicsContext;

		U32               mWidth;

		U32               mHeight;

		E_FORMAT_TYPE     mFormat;

		U32               mNumOfMipLevels;

		U32               mNumOfSamples;

		U32               mSamplingQuality;
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

	class CBaseCubemapTextureLoader : public ICubemapTextureLoader
	{
		public:
			friend TDE2_API IResourceLoader* CreateBaseCubemapTextureLoader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem,
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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseCubemapTextureLoader)
		protected:
			bool              mIsInitialized;

			IResourceManager* mpResourceManager;

			IFileSystem*      mpFileSystem;

			IGraphicsContext* mpGraphicsContext;
	};
}
