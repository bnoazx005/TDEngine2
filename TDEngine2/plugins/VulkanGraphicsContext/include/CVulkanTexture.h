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


	TDE2_API ITextureImpl* CreateVulkanTextureImpl(IGraphicsContext* pGraphicsContext, const TInitTextureImplParams& params, E_RESULT_CODE& result);


	/*!
		class CVulkanTextureImpl
	*/

	class CVulkanTextureImpl : public virtual ITextureImpl, public CBaseObject
	{
		public:
			friend TDE2_API ITextureImpl* CreateVulkanTextureImpl(IGraphicsContext*, const TInitTextureImplParams&, E_RESULT_CODE&);
		public:
			/*!
				\brief The method initializes an initial state of a texture

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IGraphicsContext* pGraphicsContext, const TInitTextureImplParams& params) override;

			TDE2_API E_RESULT_CODE Resize(U32 width, U32 height, U32 depth = 1) override;

			TDE2_API VkImage GetTextureHandle();
			TDE2_API VkImageView GetTextureViewHandle();

			TDE2_API std::vector<U8> ReadBytes(U32 index) override;

			TDE2_API const TInitTextureParams& GetParams() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CVulkanTextureImpl)

			TDE2_API E_RESULT_CODE _onInitInternal();
			TDE2_API E_RESULT_CODE _onFreeInternal() override;
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