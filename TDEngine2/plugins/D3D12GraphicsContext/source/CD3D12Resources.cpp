#include "../include/CD3D12Resources.h"
#include "../include/CD3D12GraphicsContext.h"
#include "../include/CD3D12Mappings.h"
#include "../deps/D3D12MemAlloc/D3D12MemAlloc.h"
#include <graphics/CBaseTexture2D.h>
#include <graphics/IGraphicsObjectManager.h>
#include <graphics/IShader.h>
#include <graphics/CVertexDeclaration.h>


#if defined(TDE2_USE_WINPLATFORM) /// Used only on Windows platform


namespace TDEngine2
{
	TDE2_DEFINE_SCOPED_PTR(CD3D12Buffer);


	struct TCreatedBufferInfo
	{
		ComPtr<ID3D12Resource> mpResource = nullptr;
		D3D12MA::Allocation* mpAllocation = nullptr;
	};


	static TResult<TCreatedBufferInfo> CreateBufferInternal(D3D12MA::Allocator* pAllocator, U32 size, E_BUFFER_TYPE type, E_BUFFER_USAGE_TYPE usageType, bool isUAVResource, std::optional<E_STRUCTURED_BUFFER_TYPE> structuredBufferType = std::nullopt)
	{
		if (!pAllocator || !size)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}

		D3D12_RESOURCE_DESC bufferDesc{};
		bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		bufferDesc.Width = size;
		bufferDesc.Height = 1;
		bufferDesc.DepthOrArraySize = 1;
		bufferDesc.MipLevels = 1;
		bufferDesc.SampleDesc.Count = 1;
		bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		if (isUAVResource)
		{
			bufferDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		}

		D3D12MA::ALLOCATION_DESC allocationDesc{};
		allocationDesc.HeapType = (E_BUFFER_TYPE::GENERIC == type && E_BUFFER_USAGE_TYPE::DYNAMIC == usageType) ? D3D12_HEAP_TYPE_READBACK : D3D12_HEAP_TYPE_DEFAULT;

		TCreatedBufferInfo output{};

		if (FAILED(pAllocator->CreateResource(&allocationDesc, &bufferDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, &output.mpAllocation, IID_PPV_ARGS(&output.mpResource))))
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		return Wrench::TOkValue<TCreatedBufferInfo>(output);
	}


	CD3D12Buffer::CD3D12Buffer() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CD3D12Buffer::Init(IGraphicsContext* pGraphicsContext, const TInitBufferParams& params)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mBufferSize = params.mTotalBufferSize;
		mBufferUsageType = params.mUsageType;
		mBufferType = params.mBufferType;

		mpGraphicsContextImpl = dynamic_cast<CD3D12GraphicsContext*>(pGraphicsContext);
		if (!mpGraphicsContextImpl)
		{
			return RC_FAIL;
		}

		mIsUnorderedAccessResource = params.mIsUnorderedAccessResource;

		E_RESULT_CODE result = _discardCurrentBuffer(mBufferSize, params.mStructuredBufferType);
		if (RC_OK != result)
		{
			return result;
		}

		mInitParams = params;

		if (params.mpDataPtr)
		{
			//result = InitBufferContent(mpGraphicsContextImpl, mInitParams, mInternalBufferHandle);
			if (RC_OK != result)
			{
				return result;
			}
		}

#if TDE2_DEBUG_MODE
		mpResource->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<U32>(strlen(params.mName)), params.mName);
#endif

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CD3D12Buffer::_discardCurrentBuffer(USIZE newSize, E_STRUCTURED_BUFFER_TYPE structuredBufferType)
	{
		if (mpResource)
		{
			if (mpMappedBufferData)
			{
				Unmap();
			}

			mpGraphicsContextImpl->DestroyObjectDeffered(mpResource, mpAllocation);
		}

		auto createBufferResult = CreateBufferInternal(mpGraphicsContextImpl->GetMemoryAllocator(), static_cast<U32>(newSize), mBufferType, mBufferUsageType, mIsUnorderedAccessResource, structuredBufferType);
		if (createBufferResult.HasError())
		{
			return createBufferResult.GetError();
		}

		const TCreatedBufferInfo& bufferInfo = createBufferResult.Get();

		//if (mIsUnorderedAccessResource)
		//{
		//	auto createBufferViewResult = CreateBufferViewInternal(mpGraphicsContextImpl, bufferInfo.mHandle, structuredBufferType);
		//	if (createBufferViewResult.HasError())
		//	{
		//		return createBufferViewResult.GetError();
		//	}

		//	mInternalBufferViewHandle = createBufferViewResult.Get();
		//}

		mpResource = bufferInfo.mpResource;
		mpAllocation = bufferInfo.mpAllocation;

		return RC_OK;
	}

	E_RESULT_CODE CD3D12Buffer::_onFreeInternal()
	{
		mpGraphicsContextImpl->DestroyObjectDeffered(mpResource, mpAllocation);

		mpResource = nullptr;
		mpAllocation = nullptr;

		return RC_OK;
	}

	E_RESULT_CODE CD3D12Buffer::Map(E_BUFFER_MAP_TYPE mapType, USIZE offset)
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

		D3D12_RANGE readRange{};
		readRange.Begin = offset;
		readRange.End = mBufferSize - offset - 1;

		mpResource->Map(0, offset > 0 ? &readRange : nullptr, &mpMappedBufferData);

#if TDE2_DEBUG_MODE
		++mLockChecker;
#endif

		return RC_OK;
	}

	void CD3D12Buffer::Unmap()
	{
#if TDE2_DEBUG_MODE
		--mLockChecker;
#endif

		mpResource->Unmap(0, nullptr);
		mpMappedBufferData = nullptr;
	}

	E_RESULT_CODE CD3D12Buffer::Write(const void* pData, USIZE size)
	{
		if (!mpMappedBufferData || size > mBufferSize)
		{
			return RC_FAIL;
		}

		memcpy(mpMappedBufferData, pData, size);

		return RC_OK;
	}

	void* CD3D12Buffer::Read()
	{
		return mpMappedBufferData;
	}

	E_RESULT_CODE CD3D12Buffer::Resize(USIZE newSize)
	{
		E_RESULT_CODE result = _discardCurrentBuffer(newSize, mInitParams.mStructuredBufferType);
		if (RC_OK != result)
		{
			return result;
		}

		mInitParams.mTotalBufferSize = newSize;
		mBufferSize = newSize;

		return RC_OK;
	}

	void* CD3D12Buffer::GetInternalData()
	{
		return reinterpret_cast<void*>(mpResource.Get());
	}

	USIZE CD3D12Buffer::GetSize() const
	{
		return mBufferSize;
	}

	ComPtr<ID3D12Resource> CD3D12Buffer::GetHandle() const
	{
		return mpResource;
	}

	D3D12_GPU_VIRTUAL_ADDRESS CD3D12Buffer::GetGPUAddress() const
	{
		return mpResource->GetGPUVirtualAddress();
	}

	const TInitBufferParams& CD3D12Buffer::GetParams() const
	{
		return mInitParams;
	}


	IBuffer* CreateD3D12Buffer(IGraphicsContext* pGraphicsContext, const TInitBufferParams& params, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IBuffer, CD3D12Buffer, result, pGraphicsContext, params);
	}


	/*!
		\brief CD3D12ShaderFactory's definition
	*/

	/*!
		class CD3D12ShaderFactory

		\brief The class is an abstract factory of CD3D12Shader objects that
		is used by a resource manager
	*/

	class CD3D12ShaderFactory : public CBaseObject, public IShaderFactory
	{
		public:
			friend IResourceFactory* CreateD3D12ShaderFactory(IResourceManager*, IGraphicsContext*, E_RESULT_CODE&);
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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CD3D12ShaderFactory)
		protected:
			IResourceManager* mpResourceManager = nullptr;
			IGraphicsContext* mpGraphicsContext = nullptr;
	};


	CD3D12Shader::CD3D12Shader() :
		CBaseShader()
	{
	}

	E_RESULT_CODE CD3D12Shader::Reset()
	{
		mIsInitialized = false;

		E_RESULT_CODE result = RC_OK;

		/*for (auto& currShaderModule : mShaderStageModules)
		{
			vkDestroyShaderModule(mDevice, currShaderModule, nullptr);
		}*/

		return RC_OK;
	}

	void CD3D12Shader::Bind()
	{
		CBaseShader::Bind();

	}

	void CD3D12Shader::Unbind()
	{
	}

	//VkPipelineShaderStageCreateInfo CD3D12Shader::GetPipelineShaderStage(E_SHADER_STAGE_TYPE stageType) const
	//{
	//	return mPipelineShaderStagesInfo[stageType];
	//}

	//VkPipelineShaderStageCreateInfo* CD3D12Shader::GetStages()
	//{
	//	return mPipelineShaderStagesInfo.data();
	//}

	//U32 CD3D12Shader::GetStagesCount() const
	//{
	//	return static_cast<U32>(std::count_if(mPipelineShaderStagesInfo.cbegin(), mPipelineShaderStagesInfo.cend(), [](const VkPipelineShaderStageCreateInfo& info) { return info.sType == VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO; }));
	//}

	//const VkPipelineLayout CD3D12Shader::GetPipelineLayout() const
	//{
	//	return mPipelineLayout;
	//}

	//const VkDescriptorSetLayout CD3D12Shader::GetDescriptorSetLayout() const
	//{
	//	return mDescriptorsSetLayout;
	//}

	const TD3D12PipelineLayoutInfo& CD3D12Shader::GetLayoutInfo() const
	{
		return mLayoutInfo;
	}

	E_RESULT_CODE CD3D12Shader::_createInternalHandlers(const TShaderCompilerOutput* pCompilerData)
	{
		if (!pCompilerData)
		{
			return RC_INVALID_ARGS;
		}

		CD3D12GraphicsContext* pD3D12ImplContext = dynamic_cast<CD3D12GraphicsContext*>(mpGraphicsContext);
		TDE2_ASSERT(pD3D12ImplContext);

		/*mDevice = pD3D12ImplContext->GetDevice();

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
		}*/

		return _createUniformBuffers(pCompilerData);
	}


	static TD3D12PipelineLayoutInfo::TBindingInfo::E_TYPE ConvertToBindingType(E_SHADER_RESOURCE_TYPE sourceType)
	{
		switch (sourceType)
		{
			case E_SHADER_RESOURCE_TYPE::SRT_TEXTURE2D:
			case E_SHADER_RESOURCE_TYPE::SRT_TEXTURE3D:
			case E_SHADER_RESOURCE_TYPE::SRT_TEXTURE2D_ARRAY:
			case E_SHADER_RESOURCE_TYPE::SRT_TEXTURECUBE:
			case E_SHADER_RESOURCE_TYPE::SRT_RW_IMAGE2D:
			case E_SHADER_RESOURCE_TYPE::SRT_RW_IMAGE3D:
				return TD3D12PipelineLayoutInfo::TBindingInfo::E_TYPE::TEXTURE;

			case E_SHADER_RESOURCE_TYPE::SRT_STRUCTURED_BUFFER:
			case E_SHADER_RESOURCE_TYPE::SRT_RW_STRUCTURED_BUFFER:
				return TD3D12PipelineLayoutInfo::TBindingInfo::E_TYPE::BUFFER;

			case E_SHADER_RESOURCE_TYPE::SRT_RAW_BUFFER:
			case E_SHADER_RESOURCE_TYPE::SRT_RW_RAW_BUFFER:
				return TD3D12PipelineLayoutInfo::TBindingInfo::E_TYPE::RAW_BUFFER;
		}

		TDE2_UNREACHABLE();
		return TD3D12PipelineLayoutInfo::TBindingInfo::E_TYPE::BUFFER;
	}


	E_RESULT_CODE CD3D12Shader::_createUniformBuffers(const TShaderCompilerOutput* pCompilerData)
	{
		//std::vector<VkDescriptorSetLayoutBinding> bindings;
		//std::unordered_set<U32> existingBindings;

		//for (const auto& currUniformBufferInfo : pCompilerData->mUniformBuffersInfo)
		//{
		//	if (existingBindings.find(currUniformBufferInfo.second.mSlot) != existingBindings.cend())
		//	{
		//		continue;
		//	}

		//	VkDescriptorSetLayoutBinding currBinding{};

		//	currBinding.binding = currUniformBufferInfo.second.mSlot;
		//	currBinding.descriptorCount = 1;
		//	currBinding.stageFlags = VK_SHADER_STAGE_ALL;
		//	currBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

		//	bindings.emplace_back(currBinding);
		//	existingBindings.emplace(currBinding.binding);

		//	mLayoutInfo.mCBVActiveSlots.push_back(currBinding.binding);
		//}

		//for (const auto& currShaderResourceInfo : pCompilerData->mShaderResourcesInfo)
		//{
		//	const U16 bindingOffset = GetBindingOffsetByResourceType(currShaderResourceInfo.second.mType);

		//	if (existingBindings.find(bindingOffset + currShaderResourceInfo.second.mSlot) != existingBindings.cend())
		//	{
		//		continue;
		//	}

		//	VkDescriptorSetLayoutBinding currBinding{};

		//	currBinding.binding = bindingOffset + currShaderResourceInfo.second.mSlot;
		//	currBinding.descriptorCount = 1;
		//	currBinding.stageFlags = VK_SHADER_STAGE_ALL;
		//	currBinding.descriptorType = CD3D12Mappings::GetDescriptorType(currShaderResourceInfo.second.mType);

		//	bindings.emplace_back(currBinding);
		//	existingBindings.emplace(currBinding.binding);

		//	switch (currShaderResourceInfo.second.mType)
		//	{
		//	case E_SHADER_RESOURCE_TYPE::SRT_TEXTURE2D:
		//	case E_SHADER_RESOURCE_TYPE::SRT_TEXTURE3D:
		//	case E_SHADER_RESOURCE_TYPE::SRT_TEXTURE2D_ARRAY:
		//	case E_SHADER_RESOURCE_TYPE::SRT_TEXTURECUBE:
		//	case E_SHADER_RESOURCE_TYPE::SRT_STRUCTURED_BUFFER:
		//	case E_SHADER_RESOURCE_TYPE::SRT_RAW_BUFFER:
		//	{
		//		auto& srvBindingInfo = mLayoutInfo.mSRVActiveSlots.emplace_back();

		//		srvBindingInfo.mSlot = static_cast<U32>(currShaderResourceInfo.second.mSlot);
		//		srvBindingInfo.mType = ConvertToBindingType(currShaderResourceInfo.second.mType);
		//	}
		//	break;

		//	case E_SHADER_RESOURCE_TYPE::SRT_SAMPLER_STATE:
		//		mLayoutInfo.mSamplersActiveSlots.push_back(currShaderResourceInfo.second.mSlot);
		//		break;

		//	case E_SHADER_RESOURCE_TYPE::SRT_RW_IMAGE2D:
		//	case E_SHADER_RESOURCE_TYPE::SRT_RW_IMAGE3D:
		//	case E_SHADER_RESOURCE_TYPE::SRT_RW_STRUCTURED_BUFFER:
		//	case E_SHADER_RESOURCE_TYPE::SRT_RW_RAW_BUFFER:
		//	{
		//		auto& srvBindingInfo = mLayoutInfo.mUAVActiveSlots.emplace_back();

		//		srvBindingInfo.mSlot = static_cast<U32>(currShaderResourceInfo.second.mSlot);
		//		srvBindingInfo.mType = ConvertToBindingType(currShaderResourceInfo.second.mType);
		//	}
		//	break;
		//	}
		//}

		//VkDescriptorSetLayoutCreateInfo shaderDescriptorSetLayoutCreateInfo{};
		//shaderDescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		//shaderDescriptorSetLayoutCreateInfo.bindingCount = static_cast<U32>(bindings.size());
		//shaderDescriptorSetLayoutCreateInfo.pBindings = bindings.data();
		//shaderDescriptorSetLayoutCreateInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;

		//VK_SAFE_CALL(vkCreateDescriptorSetLayout(mDevice, &shaderDescriptorSetLayoutCreateInfo, nullptr, &mDescriptorsSetLayout));

		//VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		//pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		//pipelineLayoutCreateInfo.setLayoutCount = 1;
		//pipelineLayoutCreateInfo.pSetLayouts = &mDescriptorsSetLayout;
		//pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
		//pipelineLayoutCreateInfo.pPushConstantRanges = 0;

		//VK_SAFE_CALL(vkCreatePipelineLayout(mDevice, &pipelineLayoutCreateInfo, nullptr, &mPipelineLayout));

		//auto uniformBuffersInfo = pCompilerData->mUniformBuffersInfo;

		//TUniformBufferDesc currDesc;

		//E_RESULT_CODE result = RC_OK;

		//mUniformBuffers.resize(uniformBuffersInfo.size() - TotalNumberOfInternalConstantBuffers);

		//auto pGraphicsObjectManager = mpGraphicsContext->GetGraphicsObjectManager();

		///// here only user uniforms buffers are created
		//for (auto iter = uniformBuffersInfo.cbegin(); iter != uniformBuffersInfo.cend(); ++iter)
		//{
		//	currDesc = (*iter).second;

		//	/// skip internal buffers, because they are created separately by IGlobalShaderProperties implementation
		//	if ((currDesc.mFlags & E_UNIFORM_BUFFER_DESC_FLAGS::UBDF_INTERNAL) == E_UNIFORM_BUFFER_DESC_FLAGS::UBDF_INTERNAL)
		//	{
		//		continue;
		//	}

		//	auto createBufferResult = pGraphicsObjectManager->CreateBuffer({ E_BUFFER_USAGE_TYPE::DYNAMIC, E_BUFFER_TYPE::CONSTANT, currDesc.mSize, nullptr });
		//	if (createBufferResult.HasError())
		//	{
		//		return createBufferResult.GetError();
		//	}

		//	const U32 index = static_cast<U32>(iter->second.mSlot - TotalNumberOfInternalConstantBuffers);
		//	TDE2_ASSERT(index >= 0 && index <= 1024);

		//	/// \note Ensure that we compute correct size of the constant buffer. We use ID3D11ShaderReflection to retrieve accurate information

		//	mUniformBuffers[index] = createBufferResult.Get(); // the offset is used because the shaders doesn't store internal buffer by themselves
		//}

		return RC_OK;
	}


	IShader* CreateD3D12Shader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IShader, CD3D12Shader, result, pResourceManager, pGraphicsContext, name);
	}


	CD3D12ShaderFactory::CD3D12ShaderFactory() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CD3D12ShaderFactory::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext)
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

	IResource* CD3D12ShaderFactory::Create(const std::string& name, const TBaseResourceParameters& params) const
	{
		return nullptr;
	}

	IResource* CD3D12ShaderFactory::CreateDefault(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		return dynamic_cast<IResource*>(CreateD3D12Shader(mpResourceManager, mpGraphicsContext, name, result));
	}

	TypeId CD3D12ShaderFactory::GetResourceTypeId() const
	{
		return IShader::GetTypeId();
	}


	IResourceFactory* CreateD3D12ShaderFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IResourceFactory, CD3D12ShaderFactory, result, pResourceManager, pGraphicsContext);
	}


	/*!
		\brief CD3D12TextureImpl's definition
	*/

	TDE2_DEFINE_SCOPED_PTR(CD3D12TextureImpl)


	/*!
		\brief CVulkanTextureImpl's definition
	*/

	struct TCreatedImageInfo
	{
		ComPtr<ID3D12Resource> mpTexture = nullptr;
		D3D12MA::Allocation*   mpAllocation = nullptr;
	};


	static D3D12_RESOURCE_DIMENSION GetInternalTextureType(E_TEXTURE_IMPL_TYPE type)
	{
		switch (type)
		{
			case E_TEXTURE_IMPL_TYPE::TEXTURE_2D:
				return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			case E_TEXTURE_IMPL_TYPE::TEXTURE_3D:
				return D3D12_RESOURCE_DIMENSION_TEXTURE3D;
		}

		TDE2_UNREACHABLE();
		return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	}


	static TResult<TCreatedImageInfo> CreateTextureResourceInternal(D3D12MA::Allocator* pAllocator, const TInitTextureImplParams& params)
	{
		D3D12_RESOURCE_DESC textureDesc{};
		textureDesc.Dimension        = GetInternalTextureType(params.mType);
		textureDesc.Width            = params.mWidth;
		textureDesc.Height           = params.mHeight;
		textureDesc.DepthOrArraySize = E_TEXTURE_IMPL_TYPE::TEXTURE_3D == params.mType ? params.mDepth : params.mArraySize;
		textureDesc.MipLevels        = params.mNumOfMipLevels;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Format           = (E_BIND_GRAPHICS_TYPE::BIND_DEPTH_BUFFER == (params.mBindFlags & E_BIND_GRAPHICS_TYPE::BIND_DEPTH_BUFFER)) ?
			CD3D12Mappings::GetTypelessVersionOfFormat(params.mFormat) : CD3D12Mappings::GetDXGIFormat(params.mFormat);

		if (E_BIND_GRAPHICS_TYPE::BIND_RENDER_TARGET == (params.mBindFlags & E_BIND_GRAPHICS_TYPE::BIND_RENDER_TARGET))
		{
			textureDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		}
		else if (E_BIND_GRAPHICS_TYPE::BIND_DEPTH_BUFFER == (params.mBindFlags & E_BIND_GRAPHICS_TYPE::BIND_DEPTH_BUFFER))
		{
			textureDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		}
		else if (E_BIND_GRAPHICS_TYPE::BIND_UNORDERED_ACCESS == (params.mBindFlags & E_BIND_GRAPHICS_TYPE::BIND_UNORDERED_ACCESS))
		{
			textureDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		}

		D3D12MA::ALLOCATION_DESC allocationDesc{};
		allocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

		TCreatedImageInfo output{};

		if (FAILED(pAllocator->CreateResource(&allocationDesc, &textureDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, nullptr, &output.mpAllocation, IID_PPV_ARGS(&output.mpTexture))))
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		/*VkExtent3D imageExtent;
		imageExtent.width = params.mWidth;
		imageExtent.height = params.mHeight;
		imageExtent.depth = params.mDepth;

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		imageInfo.extent = imageExtent;
		imageInfo.mipLevels = params.mNumOfMipLevels;
		imageInfo.format = CVulkanMappings::GetInternalFormat(params.mFormat);
		imageInfo.imageType = CVulkanMappings::GetTextureType(params.mType);
		imageInfo.arrayLayers = E_TEXTURE_IMPL_TYPE::CUBEMAP == params.mType ? 6 : params.mArraySize;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.samples = CVulkanMappings::GetSamplesCount(params.mNumOfSamples);
		imageInfo.flags = E_TEXTURE_IMPL_TYPE::CUBEMAP == params.mType ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0x0;

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
		allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;*/


		//VkResult result = vmaCreateImage(allocator, &imageInfo, &allocInfo, &output.mImage, &output.mAllocation, nullptr);
		//if (VK_SUCCESS != result)
		//{
		//	return Wrench::TErrValue<E_RESULT_CODE>(CVulkanMappings::GetErrorCode(result));
		//}

		return Wrench::TOkValue<TCreatedImageInfo>(output);
	}

	/*
	static TResult<VkImageView> CreateResourceViewInternal(VkDevice device, VkImage image, const TInitTextureImplParams& params)
	{
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = CVulkanMappings::GetTextureViewType(params.mType);
		viewInfo.format = CVulkanMappings::GetInternalFormat(params.mFormat);
		viewInfo.subresourceRange.aspectMask = E_FORMAT_TYPE::FT_D32 == params.mFormat ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = params.mNumOfMipLevels;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = E_TEXTURE_IMPL_TYPE::CUBEMAP == params.mType ? 6 : params.mArraySize;
		VkImageView textureImageView = VK_NULL_HANDLE;

		VkResult result = vkCreateImageView(device, &viewInfo, nullptr, &textureImageView);
		if (VK_SUCCESS != result)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(CVulkanMappings::GetErrorCode(result));
		}
		return Wrench::TOkValue<VkImageView>(textureImageView);
	}*/


	CD3D12TextureImpl::CD3D12TextureImpl() :
		CBaseObject(), mCurrLayout(E_RESOURCE_LAYOUT::UNDEFINED), mHandle(TTextureHandleId::Invalid)
	{
	}

	E_RESULT_CODE CD3D12TextureImpl::Init(IGraphicsContext* pGraphicsContext, const TInitTextureImplParams& params)
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

		mpGraphicsContextImpl = dynamic_cast<CD3D12GraphicsContext*>(pGraphicsContext);
		mInitParams = params;

		E_RESULT_CODE result = _onInitInternal();
		if (RC_OK != result)
		{
			return result;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CD3D12TextureImpl::Resize(U32 width, U32 height, U32 depth)
	{
		E_RESULT_CODE result = _onFreeInternal();

		mInitParams.mWidth  = width;
		mInitParams.mHeight = height;
		mInitParams.mDepth  = depth;

		return _onInitInternal();
	}

	E_RESULT_CODE CD3D12TextureImpl::Transition(E_RESOURCE_LAYOUT newLayout)
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

	E_RESULT_CODE CD3D12TextureImpl::SetHandle(TTextureHandleId handle, const CPassKey<CBaseGraphicsObjectManager>& passkey)
	{
		if (TTextureHandleId::Invalid == handle)
		{
			return RC_INVALID_ARGS;
		}

		mHandle = handle;

		return RC_OK;
	}

	ComPtr<ID3D12Resource> CD3D12TextureImpl::GetHandle() const
	{
		return mpResource;
	}

	//VkImageView CD3D12TextureImpl::GetTextureViewHandle()
	//{
	//	return mInternalImageViewHandle;
	//}

	std::vector<U8> CD3D12TextureImpl::ReadBytes(U32 index)
	{
		TDE2_ASSERT(E_TEXTURE_IMPL_USAGE_TYPE::DYNAMIC == mInitParams.mUsageType && mpReadbackBuffer);

		std::vector<U8> outputBytes(static_cast<size_t>(mInitParams.mWidth * mInitParams.mHeight * mInitParams.mDepth * CFormatUtils::GetFormatSize(mInitParams.mFormat)));

		//mpGraphicsContextImpl->ExecuteImmediate([this, &outputBytes](VkCommandBuffer commandBuffer)
		//	{
		//		VkBufferImageCopy regionsInfo{};
		//		regionsInfo.imageSubresource.aspectMask = CD3D12Mappings::IsDepthTextureFormat(mInitParams.mFormat) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		//		regionsInfo.imageSubresource.layerCount = 1;
		//		regionsInfo.imageExtent.width = mInitParams.mWidth;
		//		regionsInfo.imageExtent.height = mInitParams.mHeight;
		//		regionsInfo.imageExtent.depth = mInitParams.mDepth;

		//		const E_RESOURCE_LAYOUT currSourceLayout = GetLayout();

		//		VkImageMemoryBarrier2 imageMemoryBarrier{};
		//		imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
		//		imageMemoryBarrier.image = mInternalImageHandle;
		//		imageMemoryBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		//		imageMemoryBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
		//		imageMemoryBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
		//		imageMemoryBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
		//		imageMemoryBarrier.oldLayout = CD3D12Mappings::GetResourceLayout(currSourceLayout);
		//		imageMemoryBarrier.newLayout = CD3D12Mappings::GetResourceLayout(E_RESOURCE_LAYOUT::COPY_SRC);
		//		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		//		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		//		imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
		//		imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
		//		imageMemoryBarrier.subresourceRange.aspectMask = regionsInfo.imageSubresource.aspectMask;
		//		imageMemoryBarrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
		//		imageMemoryBarrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;

		//		VkDependencyInfo dependencyInfo{};
		//		dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		//		dependencyInfo.imageMemoryBarrierCount = 1;
		//		dependencyInfo.pImageMemoryBarriers = &imageMemoryBarrier;

		//		vkCmdPipelineBarrier2(commandBuffer, &dependencyInfo);
		//		vkCmdCopyImageToBuffer(commandBuffer, mInternalImageHandle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, mpReadbackBuffer->GetD3D12Handle(), 1, &regionsInfo);

		//		imageMemoryBarrier.oldLayout = CD3D12Mappings::GetResourceLayout(E_RESOURCE_LAYOUT::COPY_SRC);
		//		imageMemoryBarrier.newLayout = CD3D12Mappings::GetResourceLayout(currSourceLayout);

		//		vkCmdPipelineBarrier2(commandBuffer, &dependencyInfo);

		//		mpReadbackBuffer->Map(E_BUFFER_MAP_TYPE::BMT_READ, 0);
		//		memcpy(outputBytes.data(), mpReadbackBuffer->Read(), outputBytes.size());
		//		mpReadbackBuffer->Unmap();
		//	});

		return outputBytes;
	}

	E_RESOURCE_LAYOUT CD3D12TextureImpl::GetLayout() const
	{
		return mCurrLayout;
	}

	const TInitTextureParams& CD3D12TextureImpl::GetParams() const
	{
		return mInitParams;
	}

	E_RESULT_CODE CD3D12TextureImpl::_onInitInternal()
	{
		auto createResourceResult = CreateTextureResourceInternal(mpGraphicsContextImpl->GetMemoryAllocator(), mInitParams);
		if (createResourceResult.HasError())
		{
			return createResourceResult.GetError();
		}

		auto&& createdImageInfo = createResourceResult.Get();

		mpResource   = createdImageInfo.mpTexture;
		mpAllocation = createdImageInfo.mpAllocation;
		/*
		auto createResourceViewResult = CreateResourceViewInternal(mDevice, mInternalImageHandle, mInitParams);
		if (createResourceViewResult.HasError())
		{
			return createResourceViewResult.GetError();
		}

		mInternalImageViewHandle = createResourceViewResult.Get();*/

		E_RESULT_CODE result = RC_OK;

		if (E_TEXTURE_IMPL_USAGE_TYPE::DYNAMIC == mInitParams.mUsageType)
		{
			mpReadbackBuffer = DynamicPtrCast<CD3D12Buffer>(TPtr<IBuffer>(CreateD3D12Buffer(mpGraphicsContextImpl,
				{
					E_BUFFER_USAGE_TYPE::DYNAMIC,
					E_BUFFER_TYPE::GENERIC,
					static_cast<USIZE>(mInitParams.mWidth * mInitParams.mHeight * mInitParams.mDepth * CFormatUtils::GetFormatSize(mInitParams.mFormat)),
					nullptr
				}, result)));
		}

#if TDE2_DEBUG_MODE
		mpResource->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<U32>(strlen(mInitParams.mName)), mInitParams.mName);
#endif

		return result;
	}

	E_RESULT_CODE CD3D12TextureImpl::_onFreeInternal()
	{
		mIsInitialized = false;

		mpGraphicsContextImpl->DestroyObjectDeffered(mpResource, mpAllocation);

		mpResource   = nullptr;
		mpAllocation = nullptr;

		/*
		mpGraphicsContextImpl->DestroyObjectDeffered(mInternalImageViewHandle);*/

		return RC_OK;
	}


	ITextureImpl* CreateD3D12TextureImpl(IGraphicsContext* pGraphicsContext, const TInitTextureImplParams& params, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ITextureImpl, CD3D12TextureImpl, result, pGraphicsContext, params);
	}


	/*!
		class CD3D12VertexDeclaration

		\brief The class implements a vertex declaration for D3D12
	*/

	class CD3D12VertexDeclaration : public CVertexDeclaration
	{
		public:
			friend IVertexDeclaration* CreateD3D12VertexDeclaration(E_RESULT_CODE& result);
		public:
			void Bind(IGraphicsContext* pGraphicsContext, const CStaticArray<TBufferHandleId>& pVertexBuffersArray, IShader* pShader) override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CD3D12VertexDeclaration)
	};


	CD3D12VertexDeclaration::CD3D12VertexDeclaration() :
		CVertexDeclaration()
	{
	}

	void CD3D12VertexDeclaration::Bind(IGraphicsContext* pGraphicsContext, const CStaticArray<TBufferHandleId>& pVertexBuffersArray, IShader* pShader)
	{
	}


	IVertexDeclaration* CreateD3D12VertexDeclaration(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IVertexDeclaration, CD3D12VertexDeclaration, result);
	}
}

#endif