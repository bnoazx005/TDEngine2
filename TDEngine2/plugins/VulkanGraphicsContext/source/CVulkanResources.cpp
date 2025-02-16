#include "../include/CVulkanResources.h"
#include "../include/CVulkanMappings.h"
#include "../include/CVulkanUtils.h"
#include "../include/CVulkanGraphicsObjectManager.h"
#include "../include/CVulkanGraphicsContext.h"
#include "../include/CVulkanShaderCompiler.h"
#include <core/IResourceManager.h>
#include <graphics/CBaseShader.h>
#include <graphics/BasePipelines.h>
#include <unordered_set>
#include "deferOperation.hpp"
#include "vk_mem_alloc.h"


namespace TDEngine2
{
	static VkBufferUsageFlagBits GetBufferType(E_BUFFER_TYPE type)
	{
		switch (type)
		{
			case E_BUFFER_TYPE::VERTEX:
				return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
			case E_BUFFER_TYPE::INDEX:
				return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
			case E_BUFFER_TYPE::CONSTANT:
				return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			case E_BUFFER_TYPE::STRUCTURED:
				return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
			case E_BUFFER_TYPE::GENERIC:
				return VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		}

		TDE2_UNREACHABLE();
		return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	}


	static VmaAllocationCreateFlags GetFlagsFromType(E_BUFFER_USAGE_TYPE type)
	{
		switch (type)
		{
			case E_BUFFER_USAGE_TYPE::DYNAMIC:
				return VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
			case E_BUFFER_USAGE_TYPE::DEFAULT:
			case E_BUFFER_USAGE_TYPE::STATIC:
				return VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
		}

		TDE2_UNREACHABLE();
		return VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
	}


	struct TCreatedBufferInfo
	{
		VkBuffer      mHandle = VK_NULL_HANDLE;
		VmaAllocation mAllocation = VK_NULL_HANDLE;
	};


	static TResult<TCreatedBufferInfo> CreateBufferInternal(VmaAllocator allocator, U32 size, E_BUFFER_TYPE type, E_BUFFER_USAGE_TYPE usageType, std::optional<E_STRUCTURED_BUFFER_TYPE> structuredBufferType = std::nullopt)
	{
		VkBufferCreateInfo bufferCreateInfo{};
		bufferCreateInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.size        = size;
		bufferCreateInfo.usage       = GetBufferType(type) | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; //mIsUnorderedAccessResource ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;

		if (structuredBufferType)
		{
			switch (*structuredBufferType)
			{
				case E_STRUCTURED_BUFFER_TYPE::INDIRECT_DRAW_BUFFER:
					bufferCreateInfo.usage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
					break;
				default:
					break;
			}
		}

		VmaAllocationCreateInfo allocInfo{};
		allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
		allocInfo.flags = GetFlagsFromType(usageType);

		TCreatedBufferInfo createResult{};

		VK_SAFE_TRESULT_CALL(vmaCreateBuffer(allocator, &bufferCreateInfo, &allocInfo, &createResult.mHandle, &createResult.mAllocation, 0));

		return Wrench::TOkValue<TCreatedBufferInfo>(createResult);
	}


	static E_RESULT_CODE InitBufferContent(CVulkanGraphicsContext* pGraphicsContext, const TInitBufferParams& params, VkBuffer destBufferHandle)
	{
		VmaAllocator allocator = pGraphicsContext->GetAllocator();

		auto createStagingBufferResult = CreateBufferInternal(allocator, params.mDataSize, E_BUFFER_TYPE::GENERIC, E_BUFFER_USAGE_TYPE::DYNAMIC, params.mStructuredBufferType);
		if (createStagingBufferResult.HasError())
		{
			return createStagingBufferResult.GetError();
		}

		const TCreatedBufferInfo& stagingBufferInfo = createStagingBufferResult.Get();

		defer([=] { pGraphicsContext->DestroyObjectDeffered(stagingBufferInfo.mHandle, stagingBufferInfo.mAllocation); });

		void* pStagingBufferMappedData = nullptr;

		VK_SAFE_CALL(vmaMapMemory(allocator, stagingBufferInfo.mAllocation, &pStagingBufferMappedData));
		memcpy(pStagingBufferMappedData, params.mpDataPtr, params.mDataSize);
		vmaUnmapMemory(allocator, stagingBufferInfo.mAllocation);

		pGraphicsContext->ExecuteImmediate([&stagingBufferInfo, &params, destBufferHandle](VkCommandBuffer cmdBuffer)
			{
				VkBufferCopy bufferCopy{};
				bufferCopy.dstOffset = 0;
				bufferCopy.srcOffset = 0;
				bufferCopy.size = params.mDataSize;

				vkCmdCopyBuffer(cmdBuffer, stagingBufferInfo.mHandle, destBufferHandle, 1, &bufferCopy);
			});

		return RC_OK;
	}



	CVulkanBuffer::CVulkanBuffer() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CVulkanBuffer::Init(IGraphicsContext* pGraphicsContext, const TInitBufferParams& params)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mBufferSize = params.mTotalBufferSize;
		mBufferUsageType = params.mUsageType;
		mBufferType = params.mBufferType;

		mpGraphicsContextImpl = dynamic_cast<CVulkanGraphicsContext*>(pGraphicsContext);
		if (!mpGraphicsContextImpl)
		{
			return RC_FAIL;
		}

		mDevice = mpGraphicsContextImpl->GetDevice();
 		mAllocator = mpGraphicsContextImpl->GetAllocator();

		mIsUnorderedAccessResource = params.mIsUnorderedAccessResource;
		
		E_RESULT_CODE result = _discardCurrentBuffer(mBufferSize, params.mStructuredBufferType);
		if (RC_OK != result)
		{
			return result;
		}

		mInitParams = params;
		
		if (params.mpDataPtr)
		{
			result = InitBufferContent(mpGraphicsContextImpl, mInitParams, mInternalBufferHandle);
			if (RC_OK != result)
			{
				return result;
			}
		}

#if TDE2_DEBUG_MODE
		VkDebugUtilsObjectNameInfoEXT debugNameInfo{};
		debugNameInfo.sType        = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		debugNameInfo.objectType   = VK_OBJECT_TYPE_BUFFER;
		debugNameInfo.objectHandle = reinterpret_cast<uint64_t>(mInternalBufferHandle);
		debugNameInfo.pObjectName  = params.mName;

		VK_SAFE_CALL(vkSetDebugUtilsObjectNameEXT(mDevice, &debugNameInfo));
#endif

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CVulkanBuffer::_discardCurrentBuffer(USIZE newSize, E_STRUCTURED_BUFFER_TYPE structuredBufferType)
	{
		if (VK_NULL_HANDLE != mInternalBufferHandle)
		{
			if (mpMappedBufferData)
			{
				Unmap();
			}

			mpGraphicsContextImpl->DestroyObjectDeffered(mInternalBufferHandle, mAllocation);
		}

		auto createBufferResult = CreateBufferInternal(mAllocator, newSize, mBufferType, mBufferUsageType, structuredBufferType);
		if (createBufferResult.HasError())
		{
			return createBufferResult.GetError();
		}

		const TCreatedBufferInfo& bufferInfo = createBufferResult.Get();

		mInternalBufferHandle = bufferInfo.mHandle;
		mAllocation = bufferInfo.mAllocation;

		return RC_OK;
	}

	E_RESULT_CODE CVulkanBuffer::_onFreeInternal()
	{
		mpGraphicsContextImpl->DestroyObjectDeffered(mInternalBufferHandle, mAllocation);

		return RC_OK;
	}

	E_RESULT_CODE CVulkanBuffer::Map(E_BUFFER_MAP_TYPE mapType, USIZE offset)
	{
		if (E_BUFFER_MAP_TYPE::BMT_WRITE_DISCARD == mapType && mBufferUsageType != E_BUFFER_USAGE_TYPE::DYNAMIC)
		{
			TDE2_ASSERT(false);
			return RC_FAIL;
		}

		if (E_BUFFER_MAP_TYPE::BMT_WRITE_DISCARD == mapType)
		{
			_discardCurrentBuffer(mBufferSize, mInitParams.mStructuredBufferType);
		}

		VK_SAFE_CALL(vmaMapMemory(mAllocator, mAllocation, &mpMappedBufferData));
		mpMappedBufferData = reinterpret_cast<void*>(reinterpret_cast<U8*>(mpMappedBufferData) + offset);

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

		return RC_OK;
	}

	void* CVulkanBuffer::Read()
	{
		return mpMappedBufferData;
	}

	E_RESULT_CODE CVulkanBuffer::Resize(USIZE newSize)
	{
		E_RESULT_CODE result = _discardCurrentBuffer(mBufferSize, mInitParams.mStructuredBufferType);
		if (RC_OK != result)
		{
			return result;
		}

		if (mpMappedBufferData)
		{
			Unmap();
		}

		mpGraphicsContextImpl->DestroyObjectDeffered(mInternalBufferHandle, mAllocation);

		mInitParams.mTotalBufferSize = newSize;
		mBufferSize = newSize;

		return RC_OK;
	}

	void* CVulkanBuffer::GetInternalData() 
	{
		return reinterpret_cast<void*>(&mInternalBufferHandle);
	}

	USIZE CVulkanBuffer::GetSize() const
	{
		return mBufferSize;
	}

	VkBuffer CVulkanBuffer::GetVulkanHandle()
	{
		return mInternalBufferHandle;
	}

	const TInitBufferParams& CVulkanBuffer::GetParams() const
	{
		return mInitParams;
	}


	IBuffer* CreateVulkanBuffer(IGraphicsContext* pGraphicsContext, const TInitBufferParams& params, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IBuffer, CVulkanBuffer, result, pGraphicsContext, params);
	}


	/*!
		\brief CVulkanShaderFactory's definition
	*/

	/*!
		class CVulkanShaderFactory

		\brief The class is an abstract factory of CVulkanShader objects that
		is used by a resource manager
	*/

	class CVulkanShaderFactory : public CBaseObject, public IShaderFactory
	{
		public:
			friend IResourceFactory* CreateVulkanShaderFactory(IResourceManager*, IGraphicsContext*, E_RESULT_CODE&);
		public:
			/*!
				\brief The method initializes an internal state of a shader factory

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext) override;

			/*!
				\brief The method creates a new instance of a resource based on passed parameters

				\param[in] name A name of a resource

				\param[in] params An object that contains parameters that are needed for the resource's creation

				\return A pointer to a new instance of IResource type
			*/

			IResource* Create(const std::string& name, const TBaseResourceParameters& params) const override;

			/*!
				\brief The method creates a new instance of a resource based on passed parameters

				\param[in] name A name of a resource

				\param[in] params An object that contains parameters that are needed for the resource's creation

				\return A pointer to a new instance of IResource type
			*/

			IResource* CreateDefault(const std::string& name, const TBaseResourceParameters& params) const override;

			/*!
				\brief The method returns an identifier of a resource's type, which
				the factory serves

				\return The method returns an identifier of a resource's type, which
				the factory serves
			*/

			TypeId GetResourceTypeId() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CVulkanShaderFactory)
		protected:
			IResourceManager* mpResourceManager;

			IGraphicsContext* mpGraphicsContext;
	};

	CVulkanShader::CVulkanShader() :
		CBaseShader()
	{
	}

	E_RESULT_CODE CVulkanShader::Reset()
	{
		mIsInitialized = false;

		E_RESULT_CODE result = RC_OK;

		for (auto& currShaderModule : mShaderStageModules)
		{
			vkDestroyShaderModule(mDevice, currShaderModule, nullptr);
		}

		return RC_OK;
	}

	void CVulkanShader::Bind()
	{
		CBaseShader::Bind();

	}

	void CVulkanShader::Unbind()
	{
	}

	VkPipelineShaderStageCreateInfo CVulkanShader::GetPipelineShaderStage(E_SHADER_STAGE_TYPE stageType) const
	{
		return mPipelineShaderStagesInfo[stageType];
	}

	VkPipelineShaderStageCreateInfo* CVulkanShader::GetStages()
	{
		return mPipelineShaderStagesInfo.data();
	}

	U32 CVulkanShader::GetStagesCount() const
	{
		return static_cast<U32>(std::count_if(mPipelineShaderStagesInfo.cbegin(), mPipelineShaderStagesInfo.cend(), [](const VkPipelineShaderStageCreateInfo& info) { return info.sType == VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO; }));
	}

	const VkPipelineLayout CVulkanShader::GetPipelineLayout() const
	{
		return mPipelineLayout;
	}

	const VkDescriptorSetLayout CVulkanShader::GetDescriptorSetLayout() const
	{
		return mDescriptorsSetLayout;
	}

	const TVulkanPipelineLayoutInfo& CVulkanShader::GetLayoutInfo() const
	{
		return mLayoutInfo;
	}

	static TResult<VkShaderModule> CreateShaderModule(VkDevice device, E_SHADER_STAGE_TYPE stageType, const TShaderCompilerOutput* pCompilerData)
	{
		VkShaderModule shaderModule = VK_NULL_HANDLE;

		auto it = pCompilerData->mStagesInfo.find(stageType);
		if (it == pCompilerData->mStagesInfo.end())
		{
			return Wrench::TOkValue<VkShaderModule>(shaderModule);
		}

		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = it->second.mBytecode.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(it->second.mBytecode.data());

		VkResult result = vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule);
		if (VK_SUCCESS != result)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(CVulkanMappings::GetErrorCode(result));
		}

		return Wrench::TOkValue<VkShaderModule>(shaderModule);
	}


	static VkPipelineShaderStageCreateInfo CreatePipelineShaderStageInfo(VkDevice device, E_SHADER_STAGE_TYPE stageType, VkShaderModule shaderModule, const TShaderCompilerOutput* pCompilerData)
	{
		VkPipelineShaderStageCreateInfo shaderStageInfo{};
		shaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStageInfo.stage  = CVulkanMappings::GetShaderStageType(stageType);
		shaderStageInfo.module = shaderModule;
		shaderStageInfo.pName  = pCompilerData->mStagesInfo.at(stageType).mEntrypointName.c_str();

		return shaderStageInfo;
	}


	E_RESULT_CODE CVulkanShader::_createInternalHandlers(const TShaderCompilerOutput* pCompilerData)
	{
		if (!pCompilerData)
		{
			return RC_INVALID_ARGS;
		}

		CVulkanGraphicsContext* pVulkanImplContext = dynamic_cast<CVulkanGraphicsContext*>(mpGraphicsContext);
		TDE2_ASSERT(pVulkanImplContext);

		mDevice = pVulkanImplContext->GetDevice();

		for (U32 stageIndex = SST_VERTEX; stageIndex < SST_NONE; stageIndex++)
		{
			auto creationResult = CreateShaderModule(mDevice, static_cast<E_SHADER_STAGE_TYPE>(stageIndex), pCompilerData);
			if (creationResult.HasError())
			{
				return creationResult.GetError();
			}

			mShaderStageModules[stageIndex] = creationResult.Get();

			if (VK_NULL_HANDLE == mShaderStageModules[stageIndex])
			{
				continue;
			}

			mPipelineShaderStagesInfo[stageIndex] = CreatePipelineShaderStageInfo(mDevice, static_cast<E_SHADER_STAGE_TYPE>(stageIndex), mShaderStageModules[stageIndex], pCompilerData);
		}

		return _createUniformBuffers(pCompilerData);
	}


	static TVulkanPipelineLayoutInfo::TBindingInfo::E_TYPE ConvertToBindingType(E_SHADER_RESOURCE_TYPE sourceType)
	{
		switch (sourceType)
		{
			case E_SHADER_RESOURCE_TYPE::SRT_TEXTURE2D:
			case E_SHADER_RESOURCE_TYPE::SRT_TEXTURE3D:
			case E_SHADER_RESOURCE_TYPE::SRT_TEXTURE2D_ARRAY:
			case E_SHADER_RESOURCE_TYPE::SRT_TEXTURECUBE:
			case E_SHADER_RESOURCE_TYPE::SRT_RW_IMAGE2D:
			case E_SHADER_RESOURCE_TYPE::SRT_RW_IMAGE3D:
				return TVulkanPipelineLayoutInfo::TBindingInfo::E_TYPE::TEXTURE;

			case E_SHADER_RESOURCE_TYPE::SRT_STRUCTURED_BUFFER:
			case E_SHADER_RESOURCE_TYPE::SRT_RW_STRUCTURED_BUFFER:
			case E_SHADER_RESOURCE_TYPE::SRT_RAW_BUFFER:
			case E_SHADER_RESOURCE_TYPE::SRT_RW_RAW_BUFFER:
				return TVulkanPipelineLayoutInfo::TBindingInfo::E_TYPE::BUFFER;
		}

		TDE2_UNREACHABLE();
		return TVulkanPipelineLayoutInfo::TBindingInfo::E_TYPE::BUFFER;
	}


	E_RESULT_CODE CVulkanShader::_createUniformBuffers(const TShaderCompilerOutput* pCompilerData)
	{
		std::vector<VkDescriptorSetLayoutBinding> bindings;
		std::unordered_set<U32> existingBindings;

		for (const auto& currUniformBufferInfo : pCompilerData->mUniformBuffersInfo)
		{
			if (existingBindings.find(currUniformBufferInfo.second.mSlot) != existingBindings.cend())
			{
				continue;
			}

			VkDescriptorSetLayoutBinding currBinding{};

			currBinding.binding         = currUniformBufferInfo.second.mSlot;
			currBinding.descriptorCount = 1;
			currBinding.stageFlags      = VK_SHADER_STAGE_ALL;
			currBinding.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

			bindings.emplace_back(currBinding);
			existingBindings.emplace(currBinding.binding);

			mLayoutInfo.mCBVActiveSlots.push_back(currBinding.binding);
		}

		for (const auto& currShaderResourceInfo : pCompilerData->mShaderResourcesInfo)
		{
			const U16 bindingOffset = GetBindingOffsetByResourceType(currShaderResourceInfo.second.mType);

			if (existingBindings.find(bindingOffset + currShaderResourceInfo.second.mSlot) != existingBindings.cend())
			{
				continue;
			}

			VkDescriptorSetLayoutBinding currBinding{};

			currBinding.binding         = bindingOffset + currShaderResourceInfo.second.mSlot;
			currBinding.descriptorCount = 1;
			currBinding.stageFlags      = VK_SHADER_STAGE_ALL;
			currBinding.descriptorType  = CVulkanMappings::GetDescriptorType(currShaderResourceInfo.second.mType);

			bindings.emplace_back(currBinding);
			existingBindings.emplace(currBinding.binding);

			switch (currShaderResourceInfo.second.mType)
			{
				case E_SHADER_RESOURCE_TYPE::SRT_TEXTURE2D:
				case E_SHADER_RESOURCE_TYPE::SRT_TEXTURE3D:
				case E_SHADER_RESOURCE_TYPE::SRT_TEXTURE2D_ARRAY:
				case E_SHADER_RESOURCE_TYPE::SRT_TEXTURECUBE:
				case E_SHADER_RESOURCE_TYPE::SRT_STRUCTURED_BUFFER:
				case E_SHADER_RESOURCE_TYPE::SRT_RAW_BUFFER:
					{
						auto& srvBindingInfo = mLayoutInfo.mSRVActiveSlots.emplace_back();

						srvBindingInfo.mSlot = static_cast<U32>(currShaderResourceInfo.second.mSlot);
						srvBindingInfo.mType = ConvertToBindingType(currShaderResourceInfo.second.mType);
					}
					break;

				case E_SHADER_RESOURCE_TYPE::SRT_SAMPLER_STATE:
					mLayoutInfo.mSamplersActiveSlots.push_back(currShaderResourceInfo.second.mSlot);
					break;

				case E_SHADER_RESOURCE_TYPE::SRT_RW_IMAGE2D:
				case E_SHADER_RESOURCE_TYPE::SRT_RW_IMAGE3D:
				case E_SHADER_RESOURCE_TYPE::SRT_RW_STRUCTURED_BUFFER:
				case E_SHADER_RESOURCE_TYPE::SRT_RW_RAW_BUFFER:
					{
						auto& srvBindingInfo = mLayoutInfo.mUAVActiveSlots.emplace_back();

						srvBindingInfo.mSlot = static_cast<U32>(currShaderResourceInfo.second.mSlot);
						srvBindingInfo.mType = ConvertToBindingType(currShaderResourceInfo.second.mType);
					}
					break;
			}
		}

		VkDescriptorSetLayoutCreateInfo shaderDescriptorSetLayoutCreateInfo{};
		shaderDescriptorSetLayoutCreateInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		shaderDescriptorSetLayoutCreateInfo.bindingCount = static_cast<U32>(bindings.size());
		shaderDescriptorSetLayoutCreateInfo.pBindings    = bindings.data();
		shaderDescriptorSetLayoutCreateInfo.flags        = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;

		VK_SAFE_CALL(vkCreateDescriptorSetLayout(mDevice, &shaderDescriptorSetLayoutCreateInfo, nullptr, &mDescriptorsSetLayout));

		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		pipelineLayoutCreateInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.setLayoutCount         = 1;
		pipelineLayoutCreateInfo.pSetLayouts            = &mDescriptorsSetLayout;
		pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
		pipelineLayoutCreateInfo.pPushConstantRanges    = 0;

		VK_SAFE_CALL(vkCreatePipelineLayout(mDevice, &pipelineLayoutCreateInfo, nullptr, &mPipelineLayout));

		auto uniformBuffersInfo = pCompilerData->mUniformBuffersInfo;

		TUniformBufferDesc currDesc;

		E_RESULT_CODE result = RC_OK;

		mUniformBuffers.resize(uniformBuffersInfo.size() - TotalNumberOfInternalConstantBuffers);

		auto pGraphicsObjectManager = mpGraphicsContext->GetGraphicsObjectManager();

		/// here only user uniforms buffers are created
		for (auto iter = uniformBuffersInfo.cbegin(); iter != uniformBuffersInfo.cend(); ++iter)
		{
			currDesc = (*iter).second;

			/// skip internal buffers, because they are created separately by IGlobalShaderProperties implementation
			if ((currDesc.mFlags & E_UNIFORM_BUFFER_DESC_FLAGS::UBDF_INTERNAL) == E_UNIFORM_BUFFER_DESC_FLAGS::UBDF_INTERNAL)
			{
				continue;
			}

			auto createBufferResult = pGraphicsObjectManager->CreateBuffer({ E_BUFFER_USAGE_TYPE::DYNAMIC, E_BUFFER_TYPE::CONSTANT, currDesc.mSize, nullptr });
			if (createBufferResult.HasError())
			{
				return createBufferResult.GetError();
			}

			const U32 index = static_cast<U32>(iter->second.mSlot - TotalNumberOfInternalConstantBuffers);
			TDE2_ASSERT(index >= 0 && index <= 1024);

			/// \note Ensure that we compute correct size of the constant buffer. We use ID3D11ShaderReflection to retrieve accurate information

			mUniformBuffers[index] = createBufferResult.Get(); // the offset is used because the shaders doesn't store internal buffer by themselves
		}

		return RC_OK;
	}


	IShader* CreateVulkanShader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IShader, CVulkanShader, result, pResourceManager, pGraphicsContext, name);
	}


	CVulkanShaderFactory::CVulkanShaderFactory() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CVulkanShaderFactory::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext)
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

	IResource* CVulkanShaderFactory::Create(const std::string& name, const TBaseResourceParameters& params) const
	{
		return nullptr;
	}

	IResource* CVulkanShaderFactory::CreateDefault(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		return dynamic_cast<IResource*>(CreateVulkanShader(mpResourceManager, mpGraphicsContext, name, result));
	}

	TypeId CVulkanShaderFactory::GetResourceTypeId() const
	{
		return IShader::GetTypeId();
	}


	IResourceFactory* CreateVulkanShaderFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IResourceFactory, CVulkanShaderFactory, result, pResourceManager, pGraphicsContext);
	}


	/*!
		\brief CVulkanTextureImpl's definition
	*/

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
		imageInfo.sType       = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.usage       = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		imageInfo.extent      = imageExtent;
		imageInfo.mipLevels   = params.mNumOfMipLevels;
		imageInfo.format      = CVulkanMappings::GetInternalFormat(params.mFormat);
		imageInfo.imageType   = CVulkanMappings::GetTextureType(params.mType);
		imageInfo.arrayLayers = E_TEXTURE_IMPL_TYPE::CUBEMAP == params.mType ? 6 : params.mArraySize;
		imageInfo.tiling      = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.samples     = CVulkanMappings::GetSamplesCount(params.mNumOfSamples);
		imageInfo.flags       = E_TEXTURE_IMPL_TYPE::CUBEMAP == params.mType ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0x0;

		if (E_BIND_GRAPHICS_TYPE::BIND_RENDER_TARGET == (params.mBindFlags & E_BIND_GRAPHICS_TYPE::BIND_RENDER_TARGET))
		{
			imageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		}
		else if (E_BIND_GRAPHICS_TYPE::BIND_DEPTH_BUFFER == (params.mBindFlags & E_BIND_GRAPHICS_TYPE::BIND_DEPTH_BUFFER))
		{
			imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		}
		else if (E_BIND_GRAPHICS_TYPE::BIND_UNORDERED_ACCESS == (params.mBindFlags & E_BIND_GRAPHICS_TYPE::BIND_UNORDERED_ACCESS))
		{
			imageInfo.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
		}

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
		viewInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image                           = image;
		viewInfo.viewType                        = CVulkanMappings::GetTextureViewType(params.mType);
		viewInfo.format                          = CVulkanMappings::GetInternalFormat(params.mFormat);
		viewInfo.subresourceRange.aspectMask     = E_FORMAT_TYPE::FT_D32 == params.mFormat ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel   = 0;
		viewInfo.subresourceRange.levelCount     = params.mNumOfMipLevels;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount     = E_TEXTURE_IMPL_TYPE::CUBEMAP == params.mType ? 6 : params.mArraySize;
		VkImageView textureImageView             = VK_NULL_HANDLE;

		VkResult result = vkCreateImageView(device, &viewInfo, nullptr, &textureImageView);
		if (VK_SUCCESS != result)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(CVulkanMappings::GetErrorCode(result));
		}
		return Wrench::TOkValue<VkImageView>(textureImageView);
	}


	CVulkanTextureImpl::CVulkanTextureImpl() :
		CBaseObject(), mCurrLayout(E_RESOURCE_LAYOUT::UNDEFINED)
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
		mDevice = mpGraphicsContextImpl->GetDevice();

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

	E_RESULT_CODE CVulkanTextureImpl::Transition(E_RESOURCE_LAYOUT newLayout)
	{
		if (mCurrLayout == newLayout)
		{
			return RC_OK;
		}

		TTextureTransitionBarrierInfo barrierInfo{};
		barrierInfo.mCurrLayout = mCurrLayout;
		barrierInfo.mNewLayout  = newLayout;
		barrierInfo.mHandle     = mHandle;

		mpGraphicsContextImpl->TransitionBarrier(barrierInfo);
		mCurrLayout = newLayout;

		return RC_OK;
	}

	E_RESULT_CODE CVulkanTextureImpl::SetHandle(TTextureHandleId handle, const CPassKey<CBaseGraphicsObjectManager>& passkey)
	{
		if (TTextureHandleId::Invalid == handle)
		{
			return RC_INVALID_ARGS;
		}

		mHandle = handle;

		return RC_OK;
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
		TDE2_ASSERT(E_TEXTURE_IMPL_USAGE_TYPE::DYNAMIC == mInitParams.mUsageType && mpReadbackBuffer);
		
		std::vector<U8> outputBytes(static_cast<size_t>(mInitParams.mWidth * mInitParams.mHeight * mInitParams.mDepth * CFormatUtils::GetFormatSize(mInitParams.mFormat)));

		mpGraphicsContextImpl->ExecuteImmediate([this, &outputBytes](VkCommandBuffer commandBuffer)
			{
				VkBufferImageCopy regionsInfo{};
				regionsInfo.imageSubresource.aspectMask = CVulkanMappings::IsDepthTextureFormat(mInitParams.mFormat) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
				regionsInfo.imageSubresource.layerCount = 1;
				regionsInfo.imageExtent.width           = mInitParams.mWidth;
				regionsInfo.imageExtent.height          = mInitParams.mHeight;
				regionsInfo.imageExtent.depth           = mInitParams.mDepth;

				const E_RESOURCE_LAYOUT currSourceLayout = GetLayout();

				VkImageMemoryBarrier2 imageMemoryBarrier{};
				imageMemoryBarrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
				imageMemoryBarrier.image                           = mInternalImageHandle;
				imageMemoryBarrier.srcStageMask                    = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
				imageMemoryBarrier.srcAccessMask                   = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
				imageMemoryBarrier.dstStageMask                    = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
				imageMemoryBarrier.dstAccessMask                   = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
				imageMemoryBarrier.oldLayout                       = CVulkanMappings::GetResourceLayout(currSourceLayout);
				imageMemoryBarrier.newLayout                       = CVulkanMappings::GetResourceLayout(E_RESOURCE_LAYOUT::COPY_SRC);
				imageMemoryBarrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
				imageMemoryBarrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
				imageMemoryBarrier.subresourceRange.baseMipLevel   = 0;
				imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
				imageMemoryBarrier.subresourceRange.aspectMask     = regionsInfo.imageSubresource.aspectMask;
				imageMemoryBarrier.subresourceRange.layerCount     = VK_REMAINING_ARRAY_LAYERS;
				imageMemoryBarrier.subresourceRange.levelCount     = VK_REMAINING_MIP_LEVELS;

				VkDependencyInfo dependencyInfo{};
				dependencyInfo.sType                   = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
				dependencyInfo.imageMemoryBarrierCount = 1;
				dependencyInfo.pImageMemoryBarriers    = &imageMemoryBarrier;

				vkCmdPipelineBarrier2(commandBuffer, &dependencyInfo);
				vkCmdCopyImageToBuffer(commandBuffer, mInternalImageHandle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, mpReadbackBuffer->GetVulkanHandle(), 1, &regionsInfo);

				imageMemoryBarrier.oldLayout = CVulkanMappings::GetResourceLayout(E_RESOURCE_LAYOUT::COPY_SRC);
				imageMemoryBarrier.newLayout = CVulkanMappings::GetResourceLayout(currSourceLayout);

				vkCmdPipelineBarrier2(commandBuffer, &dependencyInfo);

				mpReadbackBuffer->Map(E_BUFFER_MAP_TYPE::BMT_READ, 0);
				memcpy(outputBytes.data(), mpReadbackBuffer->Read(), outputBytes.size());
				mpReadbackBuffer->Unmap();
			});

		return outputBytes;
	}

	E_RESOURCE_LAYOUT CVulkanTextureImpl::GetLayout() const
	{
		return mCurrLayout;
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

		E_RESULT_CODE result = RC_OK;

		if (E_TEXTURE_IMPL_USAGE_TYPE::DYNAMIC == mInitParams.mUsageType)
		{
			mpReadbackBuffer = DynamicPtrCast<CVulkanBuffer>(TPtr<IBuffer>(CreateVulkanBuffer(mpGraphicsContextImpl,
				{
					E_BUFFER_USAGE_TYPE::DYNAMIC,
					E_BUFFER_TYPE::GENERIC,
					static_cast<USIZE>(mInitParams.mWidth * mInitParams.mHeight * mInitParams.mDepth * CFormatUtils::GetFormatSize(mInitParams.mFormat)),
					nullptr
				}, result)));
		}

#if TDE2_DEBUG_MODE
		VkDebugUtilsObjectNameInfoEXT debugNameInfo{};
		debugNameInfo.sType        = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		debugNameInfo.objectType   = VK_OBJECT_TYPE_IMAGE;
		debugNameInfo.objectHandle = reinterpret_cast<uint64_t>(mInternalImageHandle);
		debugNameInfo.pObjectName  = mInitParams.mName;

		VK_SAFE_CALL(vkSetDebugUtilsObjectNameEXT(mDevice, &debugNameInfo));
#endif

		return result;
	}

	E_RESULT_CODE CVulkanTextureImpl::_onFreeInternal()
	{
		mIsInitialized = false;

		mpGraphicsContextImpl->DestroyObjectDeffered(mInternalImageHandle, mAllocation);
		mpGraphicsContextImpl->DestroyObjectDeffered(mInternalImageViewHandle);

		return RC_OK;
	}


	ITextureImpl* CreateVulkanTextureImpl(IGraphicsContext* pGraphicsContext, const TInitTextureImplParams& params, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ITextureImpl, CVulkanTextureImpl, result, pGraphicsContext, params);
	}


	/*!
		class CVulkanVertexDeclaration

		\brief The class implements a vertex declaration for Vulkan
	*/

	class CVulkanVertexDeclaration : public CVertexDeclaration
	{
		public:
			friend IVertexDeclaration* CreateVulkanVertexDeclaration(E_RESULT_CODE& result);
		public:
			/*!
				\brief The method creates an internal handlers for a vertex declaration and binds it
				to a rendering pipeline

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext implementation

				\param[in, out] pVertexBuffersArray An array of  IVertexBuffer implementations

				\param[in, out] pShader A pointer to IShader implementation
			*/

			void Bind(IGraphicsContext* pGraphicsContext, const CStaticArray<TBufferHandleId>& pVertexBuffersArray, IShader* pShader) override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CVulkanVertexDeclaration)

			E_RESULT_CODE _compile();
		private:
			std::vector<VkVertexInputBindingDescription> mInputBindings;
			std::vector<VkVertexInputAttributeDescription> mAttributeDescs;
			VkPipelineVertexInputStateCreateInfo mVertexFormatInfo;
	};


	CVulkanVertexDeclaration::CVulkanVertexDeclaration() :
		CVertexDeclaration()
	{
	}

	void CVulkanVertexDeclaration::Bind(IGraphicsContext* pGraphicsContext, const CStaticArray<TBufferHandleId>& pVertexBuffersArray, IShader* pShader)
	{
	}

	E_RESULT_CODE CVulkanVertexDeclaration::_compile()
	{
		E_RESULT_CODE result = RC_OK;

		if (!mAttributeDescs.empty())
		{
			return result;
		}

		U32 currInstancingElementIndex = (std::numeric_limits<U32>::max)();
		U32 currInstancesPerData = 0;

		TInstancingInfoArray::const_iterator instancingIter = mInstancingInfo.cbegin();

		if (!mInstancingInfo.empty())
		{
			std::tie(currInstancingElementIndex, currInstancesPerData) = *instancingIter;
		}

		VkVertexInputAttributeDescription currAttributeDesc;

		U32 currIndex = 0;
		U32 currOffset = 0;

		/// fill in elements vector
		for (auto iter = mElements.cbegin(); iter != mElements.cend(); ++iter, ++currIndex)
		{
			if (currIndex == currInstancingElementIndex) /// a new instancing division has found
			{
				currOffset = 0;
				currIndex = 0;

				VkVertexInputBindingDescription inputBindings{};
				inputBindings.binding = currInstancingElementIndex;
				inputBindings.stride = GetStrideSize(iter->mSource);
				inputBindings.inputRate = iter->mIsPerInstanceData ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX;

				mInputBindings.emplace_back(inputBindings);

				if (instancingIter + 1 == mInstancingInfo.cend())
				{
					currInstancingElementIndex = (std::numeric_limits<U32>::max)();
				}
				else
				{
					std::tie(currInstancingElementIndex, currInstancesPerData) = *(++instancingIter); /// retrieve next division's info
				}
			}

			currAttributeDesc = {};

			currAttributeDesc.binding = static_cast<U32>(currInstancingElementIndex);
			currAttributeDesc.location = currIndex;
			currAttributeDesc.format = CVulkanMappings::GetInternalFormat(iter->mFormatType);
			currAttributeDesc.offset = currOffset;

			mAttributeDescs.emplace_back(currAttributeDesc);

			currOffset += CFormatUtils::GetFormatSize((*iter).mFormatType);
		}

		mVertexFormatInfo = {};
		mVertexFormatInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		mVertexFormatInfo.pVertexAttributeDescriptions = mAttributeDescs.data();
		mVertexFormatInfo.vertexAttributeDescriptionCount = static_cast<U32>(mAttributeDescs.size());

		mVertexFormatInfo.pVertexBindingDescriptions = mInputBindings.data();
		mVertexFormatInfo.vertexBindingDescriptionCount = static_cast<U32>(mInputBindings.size());

		return result;
	}


	IVertexDeclaration* CreateVulkanVertexDeclaration(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IVertexDeclaration, CVulkanVertexDeclaration, result);
	}


	static const VkPipelineViewportStateCreateInfo* GetDefaultViewport()
	{
		static const VkViewport DEFAULT_VIEWPORT { 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f };
		static const VkRect2D DEFAULT_SCISSOR_RECT { { 0, 0 }, { 1, 1 }	};

		static VkPipelineViewportStateCreateInfo viewportStateInfo{};
		viewportStateInfo.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportStateInfo.scissorCount = 1;
		viewportStateInfo.pScissors     = &DEFAULT_SCISSOR_RECT;
		viewportStateInfo.viewportCount = 1;
		viewportStateInfo.pViewports    = &DEFAULT_VIEWPORT;

		return &viewportStateInfo;
	}


	static const VkPipelineMultisampleStateCreateInfo* GetDefaultMsaaState()
	{
		static VkPipelineMultisampleStateCreateInfo msaaStateInfo{};
		msaaStateInfo.sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		msaaStateInfo.sampleShadingEnable  = VK_FALSE;
		msaaStateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		return &msaaStateInfo;
	}


	/*!
		\brief CVulkanBasePipeline's definition
	*/

	CVulkanBasePipeline::CVulkanBasePipeline()
	{
	}

	E_RESULT_CODE CVulkanBasePipeline::Init(IGraphicsContext* pGraphicsContext)
	{
		mpVulkanGraphicsContext = dynamic_cast<CVulkanGraphicsContext*>(pGraphicsContext);

		CVulkanGraphicsContext* pVulkanGraphicsContext = dynamic_cast<CVulkanGraphicsContext*>(pGraphicsContext);
		mpVulkanGraphicsObjectManagerImpl = dynamic_cast<CVulkanGraphicsObjectManager*>(pVulkanGraphicsContext->GetGraphicsObjectManager());

		return RC_OK;
	}

	VkPipelineLayout CVulkanBasePipeline::GetPipelineLayout() const
	{
		return mCachedPipelineLayoutHandle;
	}

	const TVulkanPipelineLayoutInfo& CVulkanBasePipeline::GetLayoutInfo() const
	{
		return mLayoutInfo;
	}

	VkPipeline CVulkanBasePipeline::GetBasePipelineHandle() const
	{
		return mBasePipelineHandle;
	}


	CVulkanGraphicsPipeline::CVulkanGraphicsPipeline() :
		CBaseGraphicsPipeline()
	{
	}

	E_RESULT_CODE CVulkanGraphicsPipeline::Init(IGraphicsContext* pGraphicsContext, IResourceManager* pResourceManager, const TGraphicsPipelineConfigDesc& pipelineConfig)
	{
		E_RESULT_CODE result = CBaseGraphicsPipeline::Init(pGraphicsContext, pResourceManager, pipelineConfig);
		if (RC_OK != result)
		{
			return result;
		}

		result = CVulkanBasePipeline::Init(pGraphicsContext);
		if (RC_OK != result)
		{
			return result;
		}

		const TResourceId shaderHandle = pResourceManager->Load<IShader>(pipelineConfig.mShaderIdStr);
		if (TResourceId::Invalid == shaderHandle)
		{
			return RC_FAIL;
		}
		
		TPtr<CVulkanShader> pShader = pResourceManager->GetResource<CVulkanShader>(shaderHandle);

		// \note Prepare basic pipeline that will be derived in runtime to override attachments

		const VkFormat defaultColorAttachmentFormat = CVulkanMappings::GetInternalFormat(E_FORMAT_TYPE::FT_NORM_BYTE4);

		VkPipelineRenderingCreateInfo renderingInfo{};
		renderingInfo.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		renderingInfo.colorAttachmentCount    = 1;
		renderingInfo.pColorAttachmentFormats = &defaultColorAttachmentFormat;
		renderingInfo.depthAttachmentFormat   = pipelineConfig.mDepthStencilStateParams.mIsDepthWritingEnabled ? CVulkanMappings::GetInternalFormat(E_FORMAT_TYPE::FT_D32) : VK_FORMAT_UNDEFINED;
		renderingInfo.stencilAttachmentFormat = pipelineConfig.mDepthStencilStateParams.mIsStencilTestEnabled ? VK_FORMAT_S8_UINT : VK_FORMAT_UNDEFINED;
		
		VkPipelineVertexInputStateCreateInfo vertexInputStateInfo{};
		vertexInputStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
		inputAssemblyInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyInfo.primitiveRestartEnable = false;
		inputAssemblyInfo.topology               = CVulkanMappings::GetPrimitiveTopology(mConfig.mTopology);

		VkPipelineMultisampleStateCreateInfo multisamplingInfo{};
		multisamplingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;

		const VkPipelineColorBlendAttachmentState& colorBlendAttachment = CVulkanMappings::GetBlendState(pipelineConfig.mBlendStateParams);

		VkPipelineColorBlendStateCreateInfo colorBlendingInfo {};
		colorBlendingInfo.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendingInfo.logicOpEnable     = VK_FALSE;
		colorBlendingInfo.logicOp           = VK_LOGIC_OP_COPY;
		colorBlendingInfo.attachmentCount   = 1; // \todo For now only single target rendering is supported
		colorBlendingInfo.pAttachments      = &colorBlendAttachment;
		colorBlendingInfo.blendConstants[0] = 0.0f;

		const VkPipelineRasterizationStateCreateInfo& rasterizationInfo    = CVulkanMappings::GetRasterizerState(pipelineConfig.mRasterizerStateParams);
		const VkPipelineDepthStencilStateCreateInfo& depthStencilStateInfo = CVulkanMappings::GetDepthStencilState(pipelineConfig.mDepthStencilStateParams);

		const std::array<VkDynamicState, 3> dynamicStates
		{ 
			VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY
		};

		VkPipelineDynamicStateCreateInfo dynamicInfo{};
		dynamicInfo.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicInfo.pDynamicStates    = dynamicStates.data();
		dynamicInfo.dynamicStateCount = static_cast<U32>(dynamicStates.size());

		mCachedPipelineLayoutHandle = pShader->GetPipelineLayout();

		mBasePipelineConfig.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		mBasePipelineConfig.pNext               = &renderingInfo;
		mBasePipelineConfig.renderPass          = VK_NULL_HANDLE;
		mBasePipelineConfig.basePipelineHandle  = VK_NULL_HANDLE;
		mBasePipelineConfig.stageCount          = pShader->GetStagesCount();
		mBasePipelineConfig.pStages             = pShader->GetStages();
		mBasePipelineConfig.pViewportState      = GetDefaultViewport();
		mBasePipelineConfig.pVertexInputState   = &vertexInputInfo;
		mBasePipelineConfig.pInputAssemblyState = &inputAssemblyInfo;
		mBasePipelineConfig.pMultisampleState   = GetDefaultMsaaState();
		mBasePipelineConfig.pColorBlendState    = &colorBlendingInfo;
		mBasePipelineConfig.pRasterizationState = &rasterizationInfo;
		mBasePipelineConfig.pDepthStencilState  = &depthStencilStateInfo;
		mBasePipelineConfig.layout              = mCachedPipelineLayoutHandle;
		mBasePipelineConfig.pDynamicState       = &dynamicInfo;

		// \todo Add pipeline cache's support
		VK_SAFE_CALL(vkCreateGraphicsPipelines(mpVulkanGraphicsContext->GetDevice(), VK_NULL_HANDLE, 1, &mBasePipelineConfig, nullptr, &mBasePipelineHandle));

		mConfigHash = ComputeStateDescHash(mConfig);
		mLayoutInfo = pShader->GetLayoutInfo();

		return RC_OK;
	}

	E_RESULT_CODE CVulkanGraphicsPipeline::Bind()
	{
		if (!mpGraphicsObjectManager)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = CBaseGraphicsPipeline::Bind();
		result = result | mpVulkanGraphicsContext->BindPipelineState(this);

		return result;
	}

	VkPipeline CVulkanGraphicsPipeline::GetPipelineForRenderPass(const TRenderPassInfo& renderPassInfo)
	{
		VkPipeline derivedPipeline = VK_NULL_HANDLE;

		std::array<VkFormat, RENDER_TARGETS_MAX_COUNT> colorAttachments;
		U32 colorAttachmentsCount = 0;

		for (USIZE i = 0; i < renderPassInfo.mRenderTargetFormats.size(); ++i)
		{
			if (E_FORMAT_TYPE::FT_UNKNOWN == renderPassInfo.mRenderTargetFormats[i])
			{
				break;
			}

			colorAttachments[i] = CVulkanMappings::GetInternalFormat(renderPassInfo.mRenderTargetFormats[i]);
			++colorAttachmentsCount;
		}

		const TDepthStencilStateDesc& depthStencilDesc = mConfig.mDepthStencilStateParams;

		VkPipelineRenderingCreateInfo renderingInfo{};
		renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		renderingInfo.colorAttachmentCount    = colorAttachmentsCount;
		renderingInfo.pColorAttachmentFormats = colorAttachments.data();
		renderingInfo.depthAttachmentFormat   = depthStencilDesc.mIsDepthWritingEnabled || depthStencilDesc.mIsDepthTestEnabled ? CVulkanMappings::GetInternalFormat(E_FORMAT_TYPE::FT_D32) : VK_FORMAT_UNDEFINED;
		renderingInfo.stencilAttachmentFormat = depthStencilDesc.mIsStencilTestEnabled ? VK_FORMAT_S8_UINT : VK_FORMAT_UNDEFINED;

		VkPipelineVertexInputStateCreateInfo vertexInputStateInfo{};
		vertexInputStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
		inputAssemblyInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyInfo.primitiveRestartEnable = false;
		inputAssemblyInfo.topology               = CVulkanMappings::GetPrimitiveTopology(mConfig.mTopology);

		VkPipelineMultisampleStateCreateInfo multisamplingInfo{};
		multisamplingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;

		const VkPipelineColorBlendAttachmentState& colorBlendAttachment = CVulkanMappings::GetBlendState(mConfig.mBlendStateParams);

		VkPipelineColorBlendStateCreateInfo colorBlendingInfo{};
		colorBlendingInfo.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendingInfo.logicOpEnable     = VK_FALSE;
		colorBlendingInfo.logicOp           = VK_LOGIC_OP_COPY;
		colorBlendingInfo.attachmentCount   = 1; // \todo For now only single target rendering is supported
		colorBlendingInfo.pAttachments      = &colorBlendAttachment;
		colorBlendingInfo.blendConstants[0] = 0.0f;

		const VkPipelineRasterizationStateCreateInfo& rasterizationInfo    = CVulkanMappings::GetRasterizerState(mConfig.mRasterizerStateParams);
		const VkPipelineDepthStencilStateCreateInfo& depthStencilStateInfo = CVulkanMappings::GetDepthStencilState(mConfig.mDepthStencilStateParams);

		const std::array<VkDynamicState, 3> dynamicStates
		{
			VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY
		};

		VkPipelineDynamicStateCreateInfo dynamicInfo{};
		dynamicInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicInfo.pDynamicStates = dynamicStates.data();
		dynamicInfo.dynamicStateCount = static_cast<U32>(dynamicStates.size());

		VkGraphicsPipelineCreateInfo derivedPipelineCreateInfo = mBasePipelineConfig;
		derivedPipelineCreateInfo.basePipelineHandle  = mBasePipelineHandle;
		derivedPipelineCreateInfo.pNext               = &renderingInfo;
		derivedPipelineCreateInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		derivedPipelineCreateInfo.stageCount          = mBasePipelineConfig.stageCount;
		derivedPipelineCreateInfo.pStages             = mBasePipelineConfig.pStages;
		derivedPipelineCreateInfo.pViewportState      = GetDefaultViewport();
		derivedPipelineCreateInfo.pVertexInputState   = &vertexInputInfo;
		derivedPipelineCreateInfo.pInputAssemblyState = &inputAssemblyInfo;
		derivedPipelineCreateInfo.pMultisampleState   = GetDefaultMsaaState();
		derivedPipelineCreateInfo.pColorBlendState    = &colorBlendingInfo;
		derivedPipelineCreateInfo.pRasterizationState = &rasterizationInfo;
		derivedPipelineCreateInfo.pDepthStencilState  = &depthStencilStateInfo;
		derivedPipelineCreateInfo.layout              = mCachedPipelineLayoutHandle;
		derivedPipelineCreateInfo.pDynamicState       = &dynamicInfo;

		// \todo Add pipeline cache's support
		VK_SAFE_VOID_CALL(vkCreateGraphicsPipelines(mpVulkanGraphicsContext->GetDevice(), VK_NULL_HANDLE, 1, &derivedPipelineCreateInfo, nullptr, &derivedPipeline));

		return derivedPipeline;
	}

	U32 CVulkanGraphicsPipeline::GetHash() const
	{
		return mConfigHash;
	}

	TDE2_DEFINE_SCOPED_PTR(CVulkanGraphicsPipeline);


	IGraphicsPipeline* CreateVulkanGraphicsPipeline(IGraphicsContext* pGraphicsContext, IResourceManager* pResourceManager, const TGraphicsPipelineConfigDesc& config, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IGraphicsPipeline, CVulkanGraphicsPipeline, result, pGraphicsContext, pResourceManager, config);
	}


	/*!
		\brief CVulkanComputePipeline's definition
	*/

	CVulkanComputePipeline::CVulkanComputePipeline() :
		CBaseComputePipeline()
	{
	}

	E_RESULT_CODE CVulkanComputePipeline::Init(IGraphicsContext* pGraphicsContext, IResourceManager* pResourceManager, const std::string& shaderId)
	{
		E_RESULT_CODE result = CBaseComputePipeline::Init(pGraphicsContext, pResourceManager, shaderId);
		if (RC_OK != result)
		{
			return result;
		}

		result = CVulkanBasePipeline::Init(pGraphicsContext);
		if (RC_OK != result)
		{
			return result;
		}

		const TResourceId shaderHandle = pResourceManager->Load<IShader>(shaderId);
		if (TResourceId::Invalid == shaderHandle)
		{
			return RC_FAIL;
		}

		TPtr<CVulkanShader> pShader = pResourceManager->GetResource<CVulkanShader>(shaderHandle);

		mCachedPipelineLayoutHandle = pShader->GetPipelineLayout();

		VkComputePipelineCreateInfo computePipelineCreateInfo{};
		computePipelineCreateInfo.sType              = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		computePipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
		computePipelineCreateInfo.stage              = pShader->GetPipelineShaderStage(E_SHADER_STAGE_TYPE::SST_COMPUTE);
		computePipelineCreateInfo.layout             = pShader->GetPipelineLayout();

		VK_SAFE_CALL(vkCreateComputePipelines(mpVulkanGraphicsContext->GetDevice(), VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &mBasePipelineHandle));

		mConfigHash = TDE2_STRING_ID(mShaderIdStr.c_str());
		mLayoutInfo = pShader->GetLayoutInfo();

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CVulkanComputePipeline::Bind()
	{
		if (!mpGraphicsObjectManager)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = CBaseComputePipeline::Bind();
		result = result | mpVulkanGraphicsContext->BindPipelineState(this);

		return result;
	}

	U32 CVulkanComputePipeline::GetHash() const
	{
		return mConfigHash;
	}


	TDE2_DEFINE_SCOPED_PTR(CVulkanComputePipeline);


	IComputePipeline* CreateVulkanComputePipeline(IGraphicsContext* pGraphicsContext, IResourceManager* pResourceManager, const std::string& shaderId, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComputePipeline, CVulkanComputePipeline, result, pGraphicsContext, pResourceManager, shaderId);
	}
}