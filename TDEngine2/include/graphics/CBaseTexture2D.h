/*!
	\file CBaseTexture2D.h
	\date 20.11.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "ITexture2D.h"
#include "../core/CBaseObject.h"
#include "../core/CBaseResource.h"


namespace TDEngine2
{
	enum class TTextureHandleId : U32;


	TDE2_API ITexture2D* CreateTexture2D(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
		const TTexture2DParameters& params, E_RESULT_CODE& result);

	/*!
		class CBaseTexture2D

		\brief The interface describes a common functionality of 2D textures
	*/

	class CBaseTexture2D : public ITexture2D, public CBaseResource
	{
		public:
			friend TDE2_API ITexture2D* CreateTexture2D(IResourceManager*, IGraphicsContext*, const std::string&, const TTexture2DParameters&, E_RESULT_CODE&);
		public:
			TDE2_REGISTER_RESOURCE_TYPE(CBaseTexture2D)
			TDE2_REGISTER_TYPE(CBaseTexture2D)

			/*!
				\brief The method initializes an internal state of a 2d texture. The overloaded version of
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

				\param[in] regionRect A region, which will be overwritten

				\param[in] pData Data that will be written into a given region

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE WriteData(const TRectI32& regionRect, const U8* pData) override;

			/*!
				\brief The method allows to resize internal resources that corresponds to the given texture
			*/

			TDE2_API E_RESULT_CODE Resize(U32 width, U32 height) override;

			TDE2_API void SetWriteable(bool value) override;

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

			TDE2_API bool IsWriteable() const override;

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

			/*!
				\brief The method returns an internal data that the texture stores. (The returned data is allocated
				on heap so should be manually deleted later) For now we use std::unique_ptr instead

				\return A pointer to texture's memory, which size equals to width * height * bytes_per_channel
			*/

			TDE2_API std::vector<U8> GetInternalData() override;

			TDE2_API TTextureHandleId GetHandle() const override;

			static TDE2_API TTextureSamplerId GetTextureSampleHandle(IGraphicsContext* pGraphicsContext, const TTextureSamplerDesc& params);
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseTexture2D)

			TDE2_API const TPtr<IResourceLoader> _getResourceLoader() override;
		protected:
			IGraphicsContext*   mpGraphicsContext;

			TTextureSamplerDesc mTextureSamplerParams;

			TTextureSamplerId   mCurrTextureSamplerHandle = TTextureSamplerId::Invalid;
			TTextureHandleId    mCurrTextureHandle;

			bool                mIsWriteEnabled = false;
	};


	/*!
		\brief A factory function for creation objects of CBaseTexture2DLoader's type

		\param[in, out] pResourceManager A pointer to IResourceManager's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[in, out] pFileSystem A pointer to IFileSystem's implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CBaseTexture2DLoader's implementation
	*/

	TDE2_API IResourceLoader* CreateBaseTexture2DLoader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem,
													    E_RESULT_CODE& result);


	/*!
		class CBaseTexture2DLoader

		\brief The class is a common implementation of a 2d texture loaded
	*/

	class CBaseTexture2DLoader : public CBaseObject, public ITexture2DLoader
	{
		public:
			friend TDE2_API IResourceLoader* CreateBaseTexture2DLoader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem,
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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseTexture2DLoader)
		protected:
			IResourceManager* mpResourceManager;

			IFileSystem*      mpFileSystem;

			IGraphicsContext* mpGraphicsContext;
	};


	TDE2_API IResourceFactory* CreateBaseTexture2DFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);


	/*!
		class CBaseTexture2DFactory

		\brief The class is an abstract factory of CBaseTexture2D objects that is used by a resource manager
	*/

	class CBaseTexture2DFactory : public CBaseObject, public ITexture2DFactory
	{
		public:
			friend TDE2_API IResourceFactory* CreateBaseTexture2DFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);
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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseTexture2DFactory)
		protected:
			IResourceManager* mpResourceManager;

			IGraphicsContext* mpGraphicsContext;
	};
}
