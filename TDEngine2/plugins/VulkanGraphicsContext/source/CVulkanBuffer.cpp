#include "../include/CVulkanBuffer.h"
#include "../include/CVulkanMappings.h"
#include "../include/CVulkanUtils.h"
#include "../include/CVulkanGraphicsObjectManager.h"
#include "../include/CVulkanGraphicsContext.h"


namespace TDEngine2
{
	static VkBufferUsageFlagBits GetBufferType(E_BUFFER_TYPE type)
	{
		switch (type)
		{
		case E_BUFFER_TYPE::BT_VERTEX_BUFFER:
			return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		case E_BUFFER_TYPE::BT_INDEX_BUFFER:
			return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		case E_BUFFER_TYPE::BT_CONSTANT_BUFFER:
			return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		case E_BUFFER_TYPE::BT_STRUCTURED_BUFFER:
			return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		}

		TDE2_UNREACHABLE();
		return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	}


	static VmaAllocationCreateFlags GetFlagsFromType(E_BUFFER_USAGE_TYPE type)
	{
		switch (type)
		{
			case E_BUFFER_USAGE_TYPE::BUT_DYNAMIC:
				return VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
			case E_BUFFER_USAGE_TYPE::BUT_DEFAULT:
			case E_BUFFER_USAGE_TYPE::BUT_STATIC:
				return VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
		}

		TDE2_UNREACHABLE();
		return VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
	}


	CVulkanBuffer::CVulkanBuffer() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CVulkanBuffer::Init(const TInitBufferParams& params)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mBufferSize = params.mTotalBufferSize;
		mUsedBytesSize = 0;

		mBufferUsageType = params.mUsageType;

		mBufferType = params.mBufferType;

		mpGraphicsContextImpl = dynamic_cast<CVulkanGraphicsContext*>(params.mpGraphicsContext);
		if (!mpGraphicsContextImpl)
		{
			return RC_FAIL;
		}

		auto pGraphicsObjectManager = dynamic_cast<CVulkanGraphicsObjectManager*>(params.mpGraphicsContext->GetGraphicsObjectManager());
		if (!pGraphicsObjectManager)
		{
			return RC_FAIL;
		}

		mDevice = mpGraphicsContextImpl->GetDevice();
		mAllocator = pGraphicsObjectManager->GetAllocator();

		mIsUnorderedAccessResource = params.mIsUnorderedAccessResource;
		
		E_RESULT_CODE result = _discardCurrentBuffer();
		if (RC_OK != result)
		{
			return result;
		}
		
		// \todo Add buffer's memory initialization

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CVulkanBuffer::_discardCurrentBuffer()
	{
		if (VK_NULL_HANDLE != mInternalBufferHandle)
		{
			if (mpMappedBufferData)
			{
				Unmap();
			}

			mpGraphicsContextImpl->DestroyObjectDeffered([=]
			{
				vmaDestroyBuffer(mAllocator, mInternalBufferHandle, mAllocation);
			});
		}

		VkBufferCreateInfo bufferCreateInfo{};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.size = mBufferSize;
		bufferCreateInfo.usage = GetBufferType(mBufferType) | ((mBufferUsageType == E_BUFFER_USAGE_TYPE::BUT_DYNAMIC) ? VK_BUFFER_USAGE_TRANSFER_DST_BIT : 0x0);
		bufferCreateInfo.sharingMode = mIsUnorderedAccessResource ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
		allocInfo.flags = GetFlagsFromType(mBufferUsageType);

		VK_SAFE_CALL(vmaCreateBuffer(mAllocator, &bufferCreateInfo, &allocInfo, &mInternalBufferHandle, &mAllocation, 0));

		return RC_OK;
	}

	E_RESULT_CODE CVulkanBuffer::_onFreeInternal()
	{
		mpGraphicsContextImpl->DestroyObjectDeffered([=]
		{
			vmaDestroyBuffer(mAllocator, mInternalBufferHandle, mAllocation);
		});

		return RC_OK;
	}

	E_RESULT_CODE CVulkanBuffer::Map(E_BUFFER_MAP_TYPE mapType)
	{
		if (E_BUFFER_MAP_TYPE::BMT_WRITE_DISCARD == mapType && mBufferUsageType != E_BUFFER_USAGE_TYPE::BUT_DYNAMIC)
		{
			TDE2_ASSERT(false);
			return RC_FAIL;
		}

		VK_SAFE_CALL(vmaMapMemory(mAllocator, mAllocation, &mpMappedBufferData));

#if TDE2_DEBUG_MODE
		++mLockChecker;
#endif

		return RC_OK;
	}

	void CVulkanBuffer::Unmap()
	{
#if TDE2_DEBUG_MODE
		--mLockChecker;
#endif

		vmaUnmapMemory(mAllocator, mAllocation);
		mpMappedBufferData = nullptr;
	}

	E_RESULT_CODE CVulkanBuffer::Write(const void* pData, USIZE size)
	{
		if (!mpMappedBufferData || size > mBufferSize)
		{
			return RC_FAIL;
		}

		memcpy(mpMappedBufferData, pData, size);

		mUsedBytesSize += size;

		return RC_OK;
	}

	void* CVulkanBuffer::Read()
	{
		return mpMappedBufferData;
	}

	const TBufferInternalData& CVulkanBuffer::GetInternalData() const
	{
		return mBufferInternalData;
	}

	USIZE CVulkanBuffer::GetSize() const
	{
		return mBufferSize;
	}

	USIZE CVulkanBuffer::GetUsedSize() const
	{
		return mUsedBytesSize;
	}


	TDE2_API IBuffer* CreateVulkanBuffer(const TInitBufferParams& params, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IBuffer, CVulkanBuffer, result, params);
	}
}