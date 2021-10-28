/*!
	\brief CAtlasSubTexture.h
	\date 18.04.2021
	\authors Kasimov Ildar
*/

#pragma once


#include "IAtlasSubTexture.h"
#include "../core/CBaseResource.h"
#include "../math/TRect.h"


namespace TDEngine2
{
	class IFileSystem;


	/*!
		\brief A factory function for creation objects of CAtlasSubTexture's type

		\param[in, out] pResourceManager A pointer to IResourceManager's implementation
		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation
		\param[in] name A resource's name
		\param[in] params A parameters of created texture
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CAtlasSubTexture's implementation
	*/

	TDE2_API IAtlasSubTexture* CreateAtlasSubTexture(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
		const TAtlasSubTextureParameters& params, E_RESULT_CODE& result);


	/*!
		interface CAtlasSubTexture

		\brief The implementation represents a basic functionality of a texture that's stored within some texture atlas
	*/

	class CAtlasSubTexture : public IAtlasSubTexture, public CBaseResource
	{
		public:
			friend TDE2_API IAtlasSubTexture* CreateAtlasSubTexture(IResourceManager*, IGraphicsContext*, const std::string&, const TAtlasSubTextureParameters&, E_RESULT_CODE&);
		public:
			TDE2_REGISTER_RESOURCE_TYPE(CAtlasSubTexture)
			TDE2_REGISTER_TYPE(CAtlasSubTexture)

			/*!
				\brief The method initializes a state of a brand new sub-texture.

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation
				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation
				\param[in] name A resource's name
				\param[in] params Additional parameters of a texture

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, const TAtlasSubTextureParameters& params) override;

			/*!
				\brief The method resets current internal data of a resource

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Reset() override;

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
				\return The method returns a width of a texture
			*/

			TDE2_API U32 GetWidth() const override;

			/*!
				\return The method returns a height of a texture
			*/

			TDE2_API U32 GetHeight() const override;

			/*!
				\return The method returns current format of the texture
			*/

			TDE2_API E_FORMAT_TYPE GetFormat() const override;

			/*!
				\return The method returns a rect of the texture in range [0.0f, 1.0f]
			*/

			TDE2_API TRectF32 GetNormalizedTextureRect() const override;

			static TDE2_API TTextureSamplerId GetTextureSampleHandle(IGraphicsContext* pGraphicsContext, const TTextureSamplerDesc& params);
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CAtlasSubTexture)

			TDE2_API const TPtr<IResourceLoader> _getResourceLoader() override;
		protected:
			IGraphicsContext*   mpGraphicsContext;

			TResourceId         mOwnerAtlasResourceHandle;

			TTextureSamplerDesc mTextureSamplerParams;

			TTextureSamplerId   mCurrTextureSamplerHandle = TTextureSamplerId::Invalid;

			TRectI32            mTextureRectInfo;
	};


	/*!
		\brief A factory function for creation objects of CAtlasSubTextureLoader's type

		\param[in, out] pResourceManager A pointer to IResourceManager's implementation
		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation
		\param[in, out] pFileSystem A pointer to IFileSystem's implementation
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CAtlasSubTextureLoader's implementation
	*/

	TDE2_API IResourceLoader* CreateAtlasSubTextureLoader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem,
		E_RESULT_CODE& result);


	/*!
		class CAtlasSubTextureLoader

		\brief The class is an implementation of a loader of texture atlases
	*/

	class CAtlasSubTextureLoader : public CBaseObject, public IAtlasSubTextureLoader
	{
		public:
			friend TDE2_API IResourceLoader* CreateAtlasSubTextureLoader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem,
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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CAtlasSubTextureLoader)
		protected:
			bool              mIsInitialized;

			IResourceManager* mpResourceManager;

			IFileSystem*      mpFileSystem;

			IGraphicsContext* mpGraphicsContext;
	};


	/*!
		\brief A factory function for creation objects of CAtlasSubTextureFactory's type

		\param[in, out] pResourceManager A pointer to IResourceManager's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CAtlasSubTextureFactory's implementation
	*/

	TDE2_API IResourceFactory* CreateAtlasSubTextureFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);


	/*!
		class CAtlasSubTextureFactory

		\brief The class is an implementation of a factory of texture atlases
	*/

	class CAtlasSubTextureFactory : public CBaseObject, public IAtlasSubTextureFactory
	{
		public:
			friend TDE2_API IResourceFactory* CreateAtlasSubTextureFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);
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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CAtlasSubTextureFactory)
		protected:
			bool              mIsInitialized;

			IResourceManager* mpResourceManager;

			IGraphicsContext* mpGraphicsContext;
	};
}
