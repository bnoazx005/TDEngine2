#include "../include/CVulkanTexture2D.h"
#include "../include/CVulkanMappings.h"
#include "../include/CVulkanGraphicsContext.h"
#include "../include/CVulkanUtils.h"
#include "../include/CVulkanBuffer.h"
#include <core/IResourceManager.h>
#include <graphics/IGraphicsObjectManager.h>
#include <utils/Utils.h>


namespace TDEngine2
{
	CVulkanTexture2D::CVulkanTexture2D() :
		CBaseTexture2D()
	{
	}

	void CVulkanTexture2D::Bind(U32 slot)
	{
		CBaseTexture2D::Bind(slot);

		/*GL_SAFE_VOID_CALL(glActiveTexture(GL_TEXTURE0 + slot));
		GL_SAFE_VOID_CALL(glBindTexture(GL_TEXTURE_2D, mTextureHandler));*/
	}

	E_RESULT_CODE CVulkanTexture2D::Load()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		auto&& pResourceLoader = mpResourceManager->GetResourceLoader<ITexture2D>();

		if (!pResourceLoader)
		{
			return RC_FAIL;
		}

		return pResourceLoader->LoadResource(this);
	}

	E_RESULT_CODE CVulkanTexture2D::Unload()
	{
		return RC_NOT_IMPLEMENTED_YET;
	}

	E_RESULT_CODE CVulkanTexture2D::Reset()
	{
		mIsInitialized = false;

		mpGraphicsContextImpl->DestroyObjectDeffered([=]
		{
			vmaDestroyImage(mAllocator, mInternalImageHandle, mAllocation);
			vkDestroyImageView(mDevice, mInternalImageViewHandle, nullptr);
		});

		return RC_OK;
	}

	E_RESULT_CODE CVulkanTexture2D::WriteData(const TRectI32& regionRect, const U8* pData)
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		const USIZE textureSize = static_cast<USIZE>(mWidth * mHeight * CFormatUtils::GetFormatSize(mFormat));

		auto pGraphicsObjectManager = mpGraphicsContext->GetGraphicsObjectManager();

		TPtr<IBuffer> pStagingBuffer = pGraphicsObjectManager->GetBufferPtr(
			pGraphicsObjectManager->CreateBuffer({ E_BUFFER_USAGE_TYPE::DYNAMIC, E_BUFFER_TYPE::BT_GENERIC, textureSize, nullptr}).Get());

		if (RC_OK != result || !pStagingBuffer)
		{
			return result;
		}

		result = pStagingBuffer->Map(E_BUFFER_MAP_TYPE::BMT_WRITE);
		if (RC_OK != result)
		{
			return result;
		}

		result = pStagingBuffer->Write(pData, textureSize);
		if (RC_OK != result)
		{
			return result;
		}

		pStagingBuffer->Unmap();

		auto pStagingBufferImpl = DynamicPtrCast<CVulkanBuffer>(pStagingBuffer);

		result = mpGraphicsContextImpl->ExecuteCopyImmediate([=](VkCommandBuffer cmdBuffer)
		{
			VkImageSubresourceRange range;
			range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			range.baseMipLevel = 0;
			range.levelCount = 1;
			range.baseArrayLayer = 0;
			range.layerCount = 1;

			VkImageMemoryBarrier imageBarrierToTransfer = {};
			imageBarrierToTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

			imageBarrierToTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageBarrierToTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imageBarrierToTransfer.image = mInternalImageHandle;
			imageBarrierToTransfer.subresourceRange = range;

			imageBarrierToTransfer.srcAccessMask = 0;
			imageBarrierToTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			//barrier the image into the transfer-receive layout
			vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrierToTransfer);

			VkBufferImageCopy copyRegion = {};
			copyRegion.bufferOffset = 0;
			copyRegion.bufferRowLength = 0;
			copyRegion.bufferImageHeight = 0;

			VkExtent3D imageExtent;
			imageExtent.width = mWidth;
			imageExtent.height = mHeight;
			imageExtent.depth = 1;

			copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copyRegion.imageSubresource.mipLevel = 0;
			copyRegion.imageSubresource.baseArrayLayer = 0;
			copyRegion.imageSubresource.layerCount = 1;
			copyRegion.imageExtent = imageExtent;

			//copy the buffer into the image
			vkCmdCopyBufferToImage(cmdBuffer, pStagingBufferImpl->GetBufferImpl(), mInternalImageHandle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

			VkImageMemoryBarrier imageBarrierToReadable = imageBarrierToTransfer;

			imageBarrierToReadable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imageBarrierToReadable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			imageBarrierToReadable.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			imageBarrierToReadable.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			//barrier the image into the shader readable layout
			vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrierToReadable);
		});

		if (RC_OK != result)
		{
			return result;
		}

		return RC_OK;
	}

	std::vector<U8> CVulkanTexture2D::GetInternalData()
	{
		//std::vector<U8> pPixelData(mWidth * mHeight * CVulkanMappings::GetFormatSize(mFormat));

		/*Bind(0);
		GL_SAFE_VOID_CALL(glGetTexImage(GL_TEXTURE_2D, 0, CVulkanMappings::GetPixelDataFormat(mFormat), CVulkanMappings::GetBaseTypeOfFormat(mFormat), reinterpret_cast<void*>(&pPixelData[0])));
		
		if (CVulkanMappings::GetErrorCode(glGetError()) != RC_OK)
		{
			return {};
		}

		glBindTexture(GL_TEXTURE_2D, 0);*/

		//return std::move(pPixelData);
		return {};
	}


	struct TCreatedImageInfo
	{
		VkImage mImage;
		VmaAllocation mAllocation;
	};


	static TResult<TCreatedImageInfo> CreateImageResourceInternal(VmaAllocator allocator,
		U32 width, U32 height, E_FORMAT_TYPE format, U32 mipLevelsCount, U32 samplesCount, U32 samplingQuality, bool isWriteable)
	{
		VkExtent3D imageExtent;
		imageExtent.width = width;
		imageExtent.height = height;
		imageExtent.depth = 1;

		VkImageCreateInfo imageInfo{};
		imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		imageInfo.extent = imageExtent;
		imageInfo.mipLevels = mipLevelsCount;
		imageInfo.format = CVulkanMappings::GetInternalFormat(format);
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.arrayLayers = 1;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.samples = CVulkanMappings::GetSamplesCount(samplesCount);
		
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


	static TResult<VkImageView> CreateResourceViewInternal(VkDevice device, VkImage image, E_FORMAT_TYPE format)
	{
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = CVulkanMappings::GetInternalFormat(format);
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		VkImageView textureImageView = VK_NULL_HANDLE;
		
		VkResult result = vkCreateImageView(device, &viewInfo, nullptr, &textureImageView);
		if (VK_SUCCESS != result)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(CVulkanMappings::GetErrorCode(result));
		}

		return Wrench::TOkValue<VkImageView>(textureImageView);
	}


	E_RESULT_CODE CVulkanTexture2D::_createInternalTextureHandler(IGraphicsContext* pGraphicsContext, U32 width, U32 height, E_FORMAT_TYPE format,
																U32 mipLevelsCount, U32 samplesCount, U32 samplingQuality, bool isWriteable)
	{
		mpGraphicsContextImpl = dynamic_cast<CVulkanGraphicsContext*>(pGraphicsContext);
		
		mAllocator = mpGraphicsContextImpl->GetAllocator();
		mDevice = mpGraphicsContextImpl->GetDevice();

		// create a resource
		auto createImageResourceResult = CreateImageResourceInternal(mAllocator, width, height, format, mipLevelsCount, samplesCount, samplingQuality, isWriteable);
		if (createImageResourceResult.HasError())
		{
			return createImageResourceResult.GetError();
		}

		auto&& allocatedImage = createImageResourceResult.Get();
		mInternalImageHandle = allocatedImage.mImage;
		mAllocation = allocatedImage.mAllocation;

		// create a view
		auto createViewResult = CreateResourceViewInternal(mDevice, mInternalImageHandle, mFormat);
		if (createViewResult.HasError())
		{
			return createViewResult.GetError();
		}

		mInternalImageViewHandle = createViewResult.Get();

		// \todo Add support of UAV resources

		return RC_OK;
	}
	

	TDE2_API ITexture2D* CreateVulkanTexture2D(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ITexture2D, CVulkanTexture2D, result, pResourceManager, pGraphicsContext, name);
	}

	TDE2_API ITexture2D* CreateVulkanTexture2D(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
											const TTexture2DParameters& params, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ITexture2D, CVulkanTexture2D, result, pResourceManager, pGraphicsContext, name, params);
	}


	CVulkanTexture2DFactory::CVulkanTexture2DFactory() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CVulkanTexture2DFactory::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pGraphicsContext || !pResourceManager)
		{
			return RC_INVALID_ARGS;
		}

		mpResourceManager = pResourceManager;

		mpGraphicsContext = pGraphicsContext;

		mIsInitialized = true;

		return RC_OK;
	}

	IResource* CVulkanTexture2DFactory::Create(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		const TTexture2DParameters& texParams = static_cast<const TTexture2DParameters&>(params);

		return dynamic_cast<IResource*>(CreateVulkanTexture2D(mpResourceManager, mpGraphicsContext, name, texParams, result));
	}

	IResource* CVulkanTexture2DFactory::CreateDefault(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		static const TTexture2DParameters defaultTextureParams{ 2, 2, FT_NORM_UBYTE4, 1, 1, 0 };

		TTexture2DParameters currParams = defaultTextureParams;
		currParams.mLoadingPolicy = params.mLoadingPolicy;

		// create blank texture, which sizes equals to 2 x 2 pixels of RGBA format
		return dynamic_cast<IResource*>(CreateVulkanTexture2D(mpResourceManager, mpGraphicsContext, name, currParams, result));
	}

	TypeId CVulkanTexture2DFactory::GetResourceTypeId() const
	{
		return ITexture2D::GetTypeId();
	}


	TDE2_API IResourceFactory* CreateVulkanTexture2DFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IResourceFactory, CVulkanTexture2DFactory, result, pResourceManager, pGraphicsContext);
	}
}