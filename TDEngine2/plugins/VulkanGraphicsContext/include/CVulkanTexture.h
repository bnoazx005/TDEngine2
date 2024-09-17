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


	ITextureImpl* CreateVulkanTextureImpl(IGraphicsContext* pGraphicsContext, const TInitTextureImplParams& params, E_RESULT_CODE& result);


	/*!
		class CVulkanTextureImpl
	*/

	class CVulkanTextureImpl : public virtual ITextureImpl, public CBaseObject
	{
		public:
			friend ITextureImpl* CreateVulkanTextureImpl(IGraphicsContext*, const TInitTextureImplParams&, E_RESULT_CODE&);
		public:
			/*!
				\brief The method initializes an initial state of a texture

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			E_RESULT_CODE Init(IGraphicsContext* pGraphicsContext, const TInitTextureImplParams& params) override;

			E_RESULT_CODE Resize(U32 width, U32 height, U32 depth = 1) override;

			VkImage GetTextureHandle();
			VkImageView GetTextureViewHandle();

			std::vector<U8> ReadBytes(U32 index) override;

			const TInitTextureParams& GetParams() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CVulkanTextureImpl)

			E_RESULT_CODE _onInitInternal();
			E_RESULT_CODE _onFreeInternal() override;
		protected:
			CVulkanGraphicsContext* mpGraphicsContextImpl = nullptr;

			TInitTextureImplParams  mInitParams;

			VkImage                 mInternalImageHandle = VK_NULL_HANDLE;
			VkImageView             mInternalImageViewHandle = VK_NULL_HANDLE;

			VmaAllocator            mAllocator;
			VmaAllocation           mAllocation;

			VkDevice               mDevice;
	};
}