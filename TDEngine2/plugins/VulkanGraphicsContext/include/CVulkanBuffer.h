/*!
	\file CVulkanBuffer.h
	\date 07.11.2023
	\authors Kasimov Ildar
*/

#pragma once


#include <graphics/IBuffer.h>
#include <core/CBaseObject.h>
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan_core.h>
#include "vk_mem_alloc.h"


namespace TDEngine2
{
	class CVulkanGraphicsContext;


	/*!
		class CVulkanBuffer

		\brief The class is an implementation of a common Vulkan buffer, which can be used in different situations like
		vertex / index / uniform / storage / etc
	*/

	class CVulkanBuffer : public CBaseObject, public IBuffer
	{			
		public:
			friend TDE2_API IBuffer* CreateVulkanBuffer(IGraphicsContext* pGraphicsContext, const TInitBufferParams&, E_RESULT_CODE&);
		public:
			/*!
				\brief The method initializes an initial state of a buffer

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IGraphicsContext* pGraphicsContext, const TInitBufferParams& params);

			/*!
				\brief The method locks a buffer to provide safe data reading/writing

				\param[in] mapType A way the data should be processed during a lock

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Map(E_BUFFER_MAP_TYPE mapType) override;

			/*!
				\brief The method unlocks a buffer, so GPU can access to it after that operation
			*/

			TDE2_API void Unmap() override;

			/*!
				\brief The method writes data into a buffer

				\param[in] pData A pointer to data, which should be written
				\param[in] count A size of data in bytes

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Write(const void* pData, USIZE size) override;

			/*!
				\brief The method returns a pointer to buffer's data

				\return The method returns a pointer to buffer's data
			*/

			TDE2_API void* Read() override;

			/*!
				\brief The method recreates a buffer with a new specified size all previous data will be discarded
			*/

			TDE2_API E_RESULT_CODE Resize(USIZE newSize) override;

			/*!
				\brief The method returns an internal data of a buffer, which
				contains low-level platform specific buffer's handlers

				\return The method returns an internal data of a buffer, which
				contains low-level platform specific buffer's handlers
			*/

			TDE2_API void* GetInternalData() override;

			/*!
				\brief The method returns buffer's size in bytes

				\return The method returns buffer's size in bytes
			*/

			TDE2_API USIZE GetSize() const override;

			/*!
				\return The method returns an amount of occupied buffer's bytes
			*/

			TDE2_API USIZE GetUsedSize() const override;

			TDE2_API VkBuffer GetBufferImpl();

			TDE2_API const TInitBufferParams& GetParams() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CVulkanBuffer)

			E_RESULT_CODE _discardCurrentBuffer(USIZE newSize);
			TDE2_API E_RESULT_CODE _onFreeInternal() override;			
		protected:
			USIZE                    mBufferSize;
			USIZE                    mUsedBytesSize;

			E_BUFFER_USAGE_TYPE      mBufferUsageType;

			E_BUFFER_TYPE            mBufferType;

			void*                    mpMappedBufferData;

			TBufferInternalData      mBufferInternalData;

#if TDE2_DEBUG_MODE
			U8                       mLockChecker = 0;
#endif

			CVulkanGraphicsContext*  mpGraphicsContextImpl;

			VkBuffer                 mInternalBufferHandle = VK_NULL_HANDLE;
			VmaAllocator             mAllocator;
			VmaAllocation            mAllocation;
			VkDevice                 mDevice;

			bool                     mIsUnorderedAccessResource = false;

			TInitBufferParams        mInitParams;
	};


	/*!
		\brief A factory function for creation objects of CVulkanGraphicsContext's type

		\return A pointer to CVulkanGraphicsContext's implementation
	*/

	TDE2_API IBuffer* CreateVulkanBuffer(IGraphicsContext* pGraphicsContext, const TInitBufferParams& params, E_RESULT_CODE& result);
}