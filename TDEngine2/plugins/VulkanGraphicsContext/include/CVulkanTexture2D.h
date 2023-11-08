/*!
	\file CVulkanTexture2D.h
	\date 08.11.2023
	\authors Kasimov Ildar
*/

#pragma once


#include <core/CBaseObject.h>
#include <graphics/CBaseTexture2D.h>
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan_core.h>
#include "vk_mem_alloc.h"


namespace TDEngine2
{
	class CVulkanGraphicsContext;


	/*!
		\brief A factory function for creation objects of CVulkanTexture2D's type

		\param[in, out] pResourceManager A pointer to IGraphicsContext's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[in] name A resource's name
		
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CVulkanTexture2D's implementation
	*/

	TDE2_API ITexture2D* CreateVulkanTexture2D(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, E_RESULT_CODE& result);

	/*!
		\brief A factory function for creation objects of CVulkanTexture2D's type

		\param[in, out] pResourceManager A pointer to IGraphicsContext's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[in] name A resource's name

		\param[in] params Additional parameters of a texture

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CVulkanTexture2D's implementation
	*/

	TDE2_API ITexture2D* CreateVulkanTexture2D(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
											const TTexture2DParameters& params, E_RESULT_CODE& result);


	/*!
		class CVulkanTexture2D

		\brief The class represents an implementation of a 2d texture for Vulkan GAPI
	*/

	class CVulkanTexture2D : public CBaseTexture2D
	{
		public:
			friend TDE2_API ITexture2D* CreateVulkanTexture2D(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, E_RESULT_CODE& result);

			friend TDE2_API ITexture2D* CreateVulkanTexture2D(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
														   const TTexture2DParameters& params, E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_TYPE(CVulkanTexture2D)

			/*!
				\brief The method binds a texture object to a given slot

				\param[in] slot Slot's index
			*/

			TDE2_API void Bind(U32 slot) override;

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
				\brief The method writes data into a specified texture's region

				\param[in] regionRect A region, which will be overwritten

				\param[in] pData Data that will be written into a given region

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE WriteData(const TRectI32& regionRect, const U8* pData) override;

			/*!
				\brief The method returns an internal data that the texture stores. The returned data is allocated
				on heap so should be manually deleted later

				\return A pointer to texture's memory, which size equals to width * height * bytes_per_channel
			*/

			TDE2_API std::vector<U8> GetInternalData() override;

		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CVulkanTexture2D)

			TDE2_API E_RESULT_CODE _createInternalTextureHandler(IGraphicsContext* pGraphicsContext, U32 width, U32 height, E_FORMAT_TYPE format,
																 U32 mipLevelsCount, U32 samplesCount, U32 samplingQuality, bool isWriteable) override;
		protected:
			CVulkanGraphicsContext* mpGraphicsContextImpl;

			VkImage                 mInternalImageHandle = VK_NULL_HANDLE;
			VkImageView             mInternalImageViewHandle = VK_NULL_HANDLE;

			VmaAllocator            mAllocator;
			VmaAllocation           mAllocation;

			VkDevice                mDevice;

	};


	/*!
		\brief A factory function for creation objects of CVulkanTexture2DFactory's type

		\param[in, out] pResourceManager A pointer to IResourceManager's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CVulkanTexture2DFactory's implementation
	*/

	TDE2_API IResourceFactory* CreateVulkanTexture2DFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);


	/*!
		class CVulkanTexture2DFactory

		\brief The class is an abstract factory of CVulkanTexture2D objects that
		is used by a resource manager
	*/

	class CVulkanTexture2DFactory : public CBaseObject, public ITexture2DFactory
	{
		public:
			friend TDE2_API IResourceFactory* CreateVulkanTexture2DFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);
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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CVulkanTexture2DFactory)
		protected:
			IResourceManager* mpResourceManager;

			IGraphicsContext* mpGraphicsContext;
	};
}