#include "../include/CVulkanTexture.h"
#include "../include/CVulkanMappings.h"
#include "../include/CVulkanGraphicsContext.h"
#include "../include/CVulkanUtils.h"
#include "../include/CVulkanBuffer.h"
#include <core/IResourceManager.h>
#include <graphics/IGraphicsObjectManager.h>
#include <utils/Utils.h>


namespace TDEngine2
{
	struct TCreatedImageInfo
	{
		VkImage       mImage;
		VmaAllocation mAllocation;
	};


	static TResult<TCreatedImageInfo> CreateTextureResourceInternal(VmaAllocator allocator, const TInitTextureImplParams& params)
	{
		VkExtent3D imageExtent;
		imageExtent.width = params.mWidth;
		imageExtent.height = params.mHeight;
		imageExtent.depth = params.mDepth;

		VkImageCreateInfo imageInfo{};
		imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		imageInfo.extent = imageExtent;
		imageInfo.mipLevels = params.mNumOfMipLevels;
		imageInfo.format = CVulkanMappings::GetInternalFormat(params.mFormat);
		imageInfo.imageType = CVulkanMappings::GetTextureType(params.mType);
		imageInfo.arrayLayers = E_TEXTURE_IMPL_TYPE::CUBEMAP == params.mType ? 6 : params.mArraySize;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.samples = CVulkanMappings::GetSamplesCount(params.mNumOfSamples);

		VmaAllocationCreateInfo allocInfo{};
		allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
		allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

		TCreatedImageInfo output;

		VkResult result = vmaCreateImage(allocator, &imageInfo, &allocInfo, &output.mImage, &output.mAllocation, nullptr);
		if (VK_SUCCESS != result)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(CVulkanMappings::GetErrorCode(result));
		}

		return Wrench::TOkValue<TCreatedImageInfo>(output);
	}


	static TResult<VkImageView> CreateResourceViewInternal(VkDevice device, VkImage image, const TInitTextureImplParams& params)
	{
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = CVulkanMappings::GetTextureViewType(params.mType);
		viewInfo.format = CVulkanMappings::GetInternalFormat(params.mFormat);
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = E_TEXTURE_IMPL_TYPE::CUBEMAP == params.mType ? 6 : params.mArraySize;
		VkImageView textureImageView = VK_NULL_HANDLE;

		VkResult result = vkCreateImageView(device, &viewInfo, nullptr, &textureImageView);
		if (VK_SUCCESS != result)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(CVulkanMappings::GetErrorCode(result));
		}
		return Wrench::TOkValue<VkImageView>(textureImageView);
	}


	CVulkanTextureImpl::CVulkanTextureImpl() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CVulkanTextureImpl::Init(IGraphicsContext* pGraphicsContext, const TInitTextureImplParams& params)
	{
		if (!pGraphicsContext ||
			!params.mWidth ||
			!params.mHeight ||
			params.mDepth < 1 ||
			params.mNumOfMipLevels < 1 ||
			!params.mArraySize)
		{
			return RC_INVALID_ARGS;
		}

		mpGraphicsContextImpl = dynamic_cast<CVulkanGraphicsContext*>(pGraphicsContext);
		mInitParams = params;

		mAllocator = mpGraphicsContextImpl->GetAllocator();

		E_RESULT_CODE result = _onInitInternal();
		if (RC_OK != result)
		{
			return result;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CVulkanTextureImpl::Resize(U32 width, U32 height, U32 depth)
	{
		E_RESULT_CODE result = _onFreeInternal();

		mInitParams.mWidth = width;
		mInitParams.mHeight = height;
		mInitParams.mDepth = depth;

		return _onInitInternal();
	}

	VkImage CVulkanTextureImpl::GetTextureHandle()
	{
		return mInternalImageHandle;
	}

	VkImageView CVulkanTextureImpl::GetTextureViewHandle()
	{
		return mInternalImageViewHandle;
	}

	std::vector<U8> CVulkanTextureImpl::ReadBytes(U32 index)
	{
		TDE2_UNIMPLEMENTED();
		return {};
	}

	const TInitTextureParams& CVulkanTextureImpl::GetParams() const
	{
		return mInitParams;
	}

	E_RESULT_CODE CVulkanTextureImpl::_onInitInternal()
	{
		auto createResourceResult = CreateTextureResourceInternal(mAllocator, mInitParams);
		if (createResourceResult.HasError())
		{
			return createResourceResult.GetError();
		}

		auto&& createdImageInfo = createResourceResult.Get();

		mInternalImageHandle = createdImageInfo.mImage;
		mAllocation = createdImageInfo.mAllocation;

		auto createResourceViewResult = CreateResourceViewInternal(mDevice, mInternalImageHandle, mInitParams);
		if (createResourceViewResult.HasError())
		{
			return createResourceViewResult.GetError();
		}

		mInternalImageViewHandle = createResourceViewResult.Get();

		return RC_OK;
	}

	E_RESULT_CODE CVulkanTextureImpl::_onFreeInternal()
	{
		mIsInitialized = false;

		mpGraphicsContextImpl->DestroyObjectDeffered([=]
		{
			vmaDestroyImage(mAllocator, mInternalImageHandle, mAllocation);
			vkDestroyImageView(mDevice, mInternalImageViewHandle, nullptr);
		});

		return RC_OK;
	}


	TDE2_API ITextureImpl* CreateVulkanTextureImpl(IGraphicsContext* pGraphicsContext, const TInitTextureImplParams& params, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ITextureImpl, CVulkanTextureImpl, result, pGraphicsContext, params);
	}
}