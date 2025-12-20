#include "../include/CD3D12Resources.h"
#include "../include/CD3D12GraphicsContext.h"
#include "../include/CD3D12Mappings.h"
#include "../include/CD3D12GraphicsObjectManager.h"
#include "../deps/D3D12MemAlloc/D3D12MemAlloc.h"
#include <graphics/CBaseTexture2D.h>
#include <graphics/IGraphicsObjectManager.h>
#include <graphics/IShader.h>
#include <graphics/CVertexDeclaration.h>
#include <utils/CFileLogger.h>
#define DEFER_IMPLEMENTATION
#include "deferOperation.hpp"
#include <unordered_set>


#if defined(TDE2_USE_WINPLATFORM) /// Used only on Windows platform

#include "../deps/dx12/d3dx12_root_signature.h"


namespace TDEngine2
{
	static constexpr I32 CBV_SIZE_ALIGNMENT = 256;


	TDE2_DEFINE_SCOPED_PTR(CD3D12Buffer);


	struct TCreatedBufferInfo
	{
		ComPtr<ID3D12Resource> mpResource = nullptr;
		D3D12MA::Allocation* mpAllocation = nullptr;
	};


	static TResult<TCreatedBufferInfo> CreateBufferInternal(D3D12MA::Allocator* pAllocator, USIZE size, E_BUFFER_TYPE type, E_BUFFER_USAGE_TYPE usageType, bool isUAVResource, std::optional<E_STRUCTURED_BUFFER_TYPE> structuredBufferType = std::nullopt)
	{
		if (!pAllocator || !size)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}

		D3D12_RESOURCE_DESC bufferDesc{};
		bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		bufferDesc.Width = E_BUFFER_TYPE::CONSTANT == type ? static_cast<U32>(Align(static_cast<I32>(size), CBV_SIZE_ALIGNMENT)) : static_cast<U32>(size);
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
		allocationDesc.HeapType = (/*E_BUFFER_TYPE::GENERIC == type && */E_BUFFER_USAGE_TYPE::DYNAMIC == usageType) ? D3D12_HEAP_TYPE_UPLOAD : D3D12_HEAP_TYPE_DEFAULT;

		TCreatedBufferInfo output{};

		if (FAILED(pAllocator->CreateResource(&allocationDesc, &bufferDesc, (allocationDesc.HeapType == D3D12_HEAP_TYPE_UPLOAD) ? D3D12_RESOURCE_STATE_GENERIC_READ : D3D12_RESOURCE_STATE_COMMON, nullptr, &output.mpAllocation, IID_PPV_ARGS(&output.mpResource))))
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		return Wrench::TOkValue<TCreatedBufferInfo>(output);
	}


	static E_RESULT_CODE InitBufferContent(CD3D12GraphicsContext* pGraphicsContext, const TInitBufferParams& params, ComPtr<ID3D12Resource> destBufferHandle)
	{
		D3D12MA::Allocator* pAllocator = pGraphicsContext->GetMemoryAllocator();

		auto createStagingBufferResult = CreateBufferInternal(pAllocator, params.mDataSize, E_BUFFER_TYPE::GENERIC, E_BUFFER_USAGE_TYPE::DYNAMIC, params.mIsUnorderedAccessResource, params.mStructuredBufferType);
		if (createStagingBufferResult.HasError())
		{
			return createStagingBufferResult.GetError();
		}

		const TCreatedBufferInfo& stagingBufferInfo = createStagingBufferResult.Get();

		defer([=] { pGraphicsContext->DestroyObjectDeffered(stagingBufferInfo.mpResource, stagingBufferInfo.mpAllocation); });

		void* pStagingBufferMappedData = nullptr;

		if (FAILED(stagingBufferInfo.mpResource->Map(0, nullptr, &pStagingBufferMappedData)))
		{
			return RC_FAIL;
		}

		memcpy(pStagingBufferMappedData, params.mpDataPtr, params.mDataSize);

		stagingBufferInfo.mpResource->Unmap(0, nullptr);

		// \note Copy data from staging buffer into destination
		pGraphicsContext->ExecuteImmediate([=](ID3D12GraphicsCommandList* pCommandList)
			{
				pCommandList->CopyResource(destBufferHandle.Get(), stagingBufferInfo.mpResource.Get());
			});

		return RC_OK;
	}


	CD3D12Buffer::CD3D12Buffer() :
		CBaseObject(), mCurrLayout(E_RESOURCE_LAYOUT::SHADER_RESOURCE)
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

		E_RESULT_CODE result = _discardCurrentBuffer(mBufferSize, params.mStructuredBufferType, params.mElementStrideSize);
		if (RC_OK != result)
		{
			return result;
		}

		mInitParams = params;

		if (params.mpDataPtr)
		{
			result = InitBufferContent(mpGraphicsContextImpl, mInitParams, mpResource);
			if (RC_OK != result)
			{
				return result;
			}
		}

#if TDE2_DEBUG_MODE
		if (params.mName)
		{
			mpResource->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<U32>(strlen(params.mName)), params.mName);
		}
#endif

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CD3D12Buffer::_discardCurrentBuffer(USIZE newSize, E_STRUCTURED_BUFFER_TYPE structuredBufferType, USIZE elementStrideSize)
	{
		if (mpResource)
		{
			if (mpMappedBufferData)
			{
				Unmap();
			}

			mpGraphicsContextImpl->DestroyObjectDeffered(mpResource, mpAllocation);
		}

		auto createBufferResult = CreateBufferInternal(mpGraphicsContextImpl->GetMemoryAllocator(), newSize, mBufferType, mBufferUsageType, mIsUnorderedAccessResource, structuredBufferType);
		if (createBufferResult.HasError())
		{
			return createBufferResult.GetError();
		}

		const TCreatedBufferInfo& bufferInfo = createBufferResult.Get();

		if (E_BUFFER_TYPE::CONSTANT == mBufferType)
		{
			mConstantBufferView = mpGraphicsContextImpl->GetDescriptorsAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)->AllocDescriptor();

			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc{};
			cbvDesc.BufferLocation = bufferInfo.mpResource->GetGPUVirtualAddress();
			cbvDesc.SizeInBytes = static_cast<U32>(Align(static_cast<I32>(newSize), CBV_SIZE_ALIGNMENT));

			mpGraphicsContextImpl->GetDeviceContext()->CreateConstantBufferView(&cbvDesc, mConstantBufferView.mCPUHandle);
		}

		if (E_BUFFER_TYPE::STRUCTURED == mBufferType && E_STRUCTURED_BUFFER_TYPE::INDIRECT_DRAW_BUFFER != structuredBufferType)
		{
			mShaderResourceView = mpGraphicsContextImpl->GetDescriptorsAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)->AllocDescriptor();

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
			srvDesc.Format = DXGI_FORMAT_UNKNOWN;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			srvDesc.Buffer.FirstElement = 0;
			srvDesc.Buffer.NumElements = static_cast<U32>(newSize / elementStrideSize);
			srvDesc.Buffer.StructureByteStride = static_cast<U32>(elementStrideSize);
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

			mpGraphicsContextImpl->GetDeviceContext()->CreateShaderResourceView(bufferInfo.mpResource.Get(), &srvDesc, mShaderResourceView.mCPUHandle);
		}

		if (mIsUnorderedAccessResource)
		{
			mUnorderedAccessView = mpGraphicsContextImpl->GetDescriptorsAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)->AllocDescriptor();

			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
			uavDesc.Format = DXGI_FORMAT_UNKNOWN;
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
			uavDesc.Buffer.FirstElement = 0;
			uavDesc.Buffer.StructureByteStride = static_cast<U32>(elementStrideSize);
			uavDesc.Buffer.NumElements = static_cast<U32>(newSize / elementStrideSize);

			mpGraphicsContextImpl->GetDeviceContext()->CreateUnorderedAccessView(bufferInfo.mpResource.Get(), nullptr, &uavDesc, mUnorderedAccessView.mCPUHandle);
		}

		mpResource = bufferInfo.mpResource;
		mpAllocation = bufferInfo.mpAllocation;

#if TDE2_DEBUG_MODE
		if (mInitParams.mName)
		{
			mpResource->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<U32>(strlen(mInitParams.mName)), mInitParams.mName);
		}
#endif

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
			_discardCurrentBuffer(mBufferSize, mInitParams.mStructuredBufferType, mInitParams.mElementStrideSize);
		}

		D3D12_RANGE readRange{};
		readRange.Begin = offset;
		readRange.End = mBufferSize - offset - 1;

		if (FAILED(mpResource->Map(0, offset > 0 ? &readRange : nullptr, &mpMappedBufferData)))
		{
			return RC_FAIL;
		}

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

	E_RESULT_CODE CD3D12Buffer::Transition(E_RESOURCE_LAYOUT newLayout)
	{
		if (mCurrLayout == newLayout)
		{
			return RC_OK;
		}

		TBufferTransitionBarrierInfo barrierInfo{};
		barrierInfo.mCurrLayout = mCurrLayout;
		barrierInfo.mNewLayout  = newLayout;
		barrierInfo.mHandle     = mHandle;

		mpGraphicsContextImpl->TransitionBarrier(barrierInfo);
		mCurrLayout = newLayout;

		return RC_OK;
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
		E_RESULT_CODE result = _discardCurrentBuffer(newSize, mInitParams.mStructuredBufferType, mInitParams.mElementStrideSize);
		if (RC_OK != result)
		{
			return result;
		}

		mInitParams.mTotalBufferSize = newSize;
		mBufferSize = newSize;

		return RC_OK;
	}

	E_RESULT_CODE CD3D12Buffer::SetHandle(TBufferHandleId handle, const CPassKey<CBaseGraphicsObjectManager>& passkey)
	{
		if (TBufferHandleId::Invalid == handle)
		{
			return RC_INVALID_ARGS;
		}

		mHandle = handle;

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

	const TD3D12ResourceDescriptor& CD3D12Buffer::GetUnorderedAccessViewHandle() const
	{
		return mUnorderedAccessView;
	}

	const TD3D12ResourceDescriptor& CD3D12Buffer::GetShaderResourceViewHandle() const
	{
		return mShaderResourceView;
	}

	const TInitBufferParams& CD3D12Buffer::GetParams() const
	{
		return mInitParams;
	}

	E_RESOURCE_LAYOUT CD3D12Buffer::GetLayout() const
	{
		return mCurrLayout;
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


	CD3D12ShaderImpl::CD3D12ShaderImpl() :
		CBaseShaderImpl()
	{
	}

	const D3D12_SHADER_BYTECODE& CD3D12ShaderImpl::GetPipelineShaderStage(E_SHADER_STAGE_TYPE stageType) const
	{
		return mStagesBytecode[static_cast<U32>(stageType)];
	}

	ComPtr<ID3D12RootSignature> CD3D12ShaderImpl::GetRootSignature() const
	{
		return mpRootSignature;
	}

	const TD3D12PipelineLayoutInfo& CD3D12ShaderImpl::GetLayoutInfo() const
	{
		return mLayoutInfo;
	}

	E_RESULT_CODE CD3D12ShaderImpl::_createInternalHandlers(const TShaderCompilerOutput* pCompilerData)
	{
		if (!pCompilerData)
		{
			return RC_INVALID_ARGS;
		}

		CD3D12GraphicsContext* pD3D12ImplContext = dynamic_cast<CD3D12GraphicsContext*>(mpGraphicsContext);
		TDE2_ASSERT(pD3D12ImplContext);

		for (U32 stageIndex = SST_VERTEX; stageIndex < SST_NONE; stageIndex++)
		{
			D3D12_SHADER_BYTECODE& currBytecodeEntity = mStagesBytecode[stageIndex];

			auto&& it = pCompilerData->mStagesInfo.find(static_cast<E_SHADER_STAGE_TYPE>(stageIndex));
			if (it == pCompilerData->mStagesInfo.cend())
			{
				continue;
			}

			currBytecodeEntity.pShaderBytecode = it->second.mBytecode.data();
			currBytecodeEntity.BytecodeLength  = it->second.mBytecode.size();
		}

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


	static U16 GetBindingOffsetByResourceType(E_SHADER_RESOURCE_TYPE type)
	{
		switch (type)
		{
			case E_SHADER_RESOURCE_TYPE::SRT_TEXTURE2D:
			case E_SHADER_RESOURCE_TYPE::SRT_TEXTURE3D:
			case E_SHADER_RESOURCE_TYPE::SRT_TEXTURE2D_ARRAY:
			case E_SHADER_RESOURCE_TYPE::SRT_TEXTURECUBE:
			case E_SHADER_RESOURCE_TYPE::SRT_STRUCTURED_BUFFER:
			case E_SHADER_RESOURCE_TYPE::SRT_RAW_BUFFER:
				return 1000;

			case E_SHADER_RESOURCE_TYPE::SRT_SAMPLER_STATE:
				return 2000;

			case E_SHADER_RESOURCE_TYPE::SRT_RW_IMAGE2D:
			case E_SHADER_RESOURCE_TYPE::SRT_RW_IMAGE3D:
			case E_SHADER_RESOURCE_TYPE::SRT_RW_STRUCTURED_BUFFER:
			case E_SHADER_RESOURCE_TYPE::SRT_RW_RAW_BUFFER:
				return 3000;
		}

		TDE2_UNREACHABLE();
		return 0;
	}


	template <typename TIter, typename TKeyFunctor>
	std::vector<std::tuple<U32, U32>> GetConsecutiveRanges(TIter first, TIter last, const TKeyFunctor& getKey)
	{
		std::vector<std::tuple<U32, U32>> output{};

		if (first == last)
		{
			return output;
		}

		TIter prevIt = first;
		U32 length = 1;

		for (TIter it = first + 1; it != last; ++it)
		{
			if (std::abs(getKey(*it) - getKey(*(it - 1))) <= 1)
			{
				++length;
				continue;
			}

			output.emplace_back(static_cast<U32>(std::distance(first, prevIt)), length);
			prevIt = it;

			length = 1;
		}

		output.emplace_back(static_cast<U32>(std::distance(first, prevIt)), length);

		return output;
	}


	E_RESULT_CODE CD3D12ShaderImpl::_createRootSignature(const TShaderCompilerOutput* pCompilerData)
	{
		CFixedVector<CD3DX12_ROOT_PARAMETER, D3D12_MAX_ROOT_COST> rootParams{};
		CFixedVector<CD3DX12_DESCRIPTOR_RANGE, 256> descriptorRanges{};

		std::unordered_set<U32> existingBindings;

		for (const auto& currUniformBufferInfo : pCompilerData->mUniformBuffersInfo)
		{
			rootParams.emplace_back()->InitAsConstantBufferView(currUniformBufferInfo.second.mSlot);
			mLayoutInfo.mCBVActiveSlots.push_back(currUniformBufferInfo.second.mSlot);
		}

		for (const auto& currShaderResourceInfo : pCompilerData->mShaderResourcesInfo)
		{
			const U16 bindingOffset = GetBindingOffsetByResourceType(currShaderResourceInfo.second.mType); // \note This "binding offset" is used for code simplification to use same set for all types of resources instead of separate set per type

			if (existingBindings.find(bindingOffset + currShaderResourceInfo.second.mSlot) != existingBindings.cend())
			{
				continue;
			}

			existingBindings.emplace(bindingOffset + currShaderResourceInfo.second.mSlot);

			switch (currShaderResourceInfo.second.mType)
			{
				case E_SHADER_RESOURCE_TYPE::SRT_TEXTURE2D:
				case E_SHADER_RESOURCE_TYPE::SRT_TEXTURE3D:
				case E_SHADER_RESOURCE_TYPE::SRT_TEXTURE2D_ARRAY:
				case E_SHADER_RESOURCE_TYPE::SRT_TEXTURECUBE:
				case E_SHADER_RESOURCE_TYPE::SRT_STRUCTURED_BUFFER:
				case E_SHADER_RESOURCE_TYPE::SRT_RAW_BUFFER:
				{
					auto it = mLayoutInfo.mSRVActiveSlots.emplace(
						std::upper_bound(mLayoutInfo.mSRVActiveSlots.begin(), mLayoutInfo.mSRVActiveSlots.end(),
							TD3D12PipelineLayoutInfo::TBindingInfo{ static_cast<U32>(currShaderResourceInfo.second.mSlot) },
							[](const TD3D12PipelineLayoutInfo::TBindingInfo& left, const TD3D12PipelineLayoutInfo::TBindingInfo& right) { return left.mSlot < right.mSlot; }));

					it->mSlot = static_cast<U32>(currShaderResourceInfo.second.mSlot);
					it->mType = ConvertToBindingType(currShaderResourceInfo.second.mType);
				}
				break;

				case E_SHADER_RESOURCE_TYPE::SRT_SAMPLER_STATE:
					mLayoutInfo.mSamplersActiveSlots.emplace(std::upper_bound(mLayoutInfo.mSamplersActiveSlots.begin(), mLayoutInfo.mSamplersActiveSlots.end(), currShaderResourceInfo.second.mSlot), currShaderResourceInfo.second.mSlot);
					break;

				case E_SHADER_RESOURCE_TYPE::SRT_RW_IMAGE2D:
				case E_SHADER_RESOURCE_TYPE::SRT_RW_IMAGE3D:
				case E_SHADER_RESOURCE_TYPE::SRT_RW_STRUCTURED_BUFFER:
				case E_SHADER_RESOURCE_TYPE::SRT_RW_RAW_BUFFER:
				{
					auto it = mLayoutInfo.mUAVActiveSlots.emplace(
						std::upper_bound(mLayoutInfo.mUAVActiveSlots.begin(), mLayoutInfo.mUAVActiveSlots.end(),
							TD3D12PipelineLayoutInfo::TBindingInfo{ static_cast<U32>(currShaderResourceInfo.second.mSlot) },
							[](const TD3D12PipelineLayoutInfo::TBindingInfo& left, const TD3D12PipelineLayoutInfo::TBindingInfo& right) { return left.mSlot < right.mSlot; }));

					it->mSlot = static_cast<U32>(currShaderResourceInfo.second.mSlot);
					it->mType = ConvertToBindingType(currShaderResourceInfo.second.mType);
				}
				break;
			}
		}

		if (!mLayoutInfo.mSRVActiveSlots.empty())
		{
			for (const auto [firstIndex, length] : GetConsecutiveRanges(mLayoutInfo.mSRVActiveSlots.begin(), mLayoutInfo.mSRVActiveSlots.end(), [](const TD3D12PipelineLayoutInfo::TBindingInfo& info) { return static_cast<I32>(info.mSlot); }))
			{
				descriptorRanges.emplace_back(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, length, static_cast<U32>(mLayoutInfo.mSRVActiveSlots[firstIndex].mSlot));
			}
		}

		if (!mLayoutInfo.mUAVActiveSlots.empty())
		{
			for (const auto [firstIndex, length] : GetConsecutiveRanges(mLayoutInfo.mUAVActiveSlots.begin(), mLayoutInfo.mUAVActiveSlots.end(), [](const TD3D12PipelineLayoutInfo::TBindingInfo& info) { return static_cast<I32>(info.mSlot); }))
			{
				descriptorRanges.emplace_back(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, length, static_cast<U32>(mLayoutInfo.mUAVActiveSlots[firstIndex].mSlot));
			}
		}

		rootParams.emplace_back()->InitAsDescriptorTable(static_cast<U32>(descriptorRanges.size()), descriptorRanges.data());

		mLayoutInfo.mResourcesTableRootIndex = static_cast<U32>(rootParams.size() - 1);

		if (!mLayoutInfo.mSamplersActiveSlots.empty())
		{
			const USIZE samplersStartOffset = descriptorRanges.size();

			for (const auto [firstIndex, length] : GetConsecutiveRanges(mLayoutInfo.mSamplersActiveSlots.begin(), mLayoutInfo.mSamplersActiveSlots.end(), [](const U32& slot) { return static_cast<I32>(slot); }))
			{
				descriptorRanges.emplace_back(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, length, static_cast<U32>(mLayoutInfo.mSamplersActiveSlots[firstIndex]));
			}

			rootParams.emplace_back()->InitAsDescriptorTable(static_cast<U32>(descriptorRanges.size() - samplersStartOffset), &descriptorRanges[samplersStartOffset]);

			mLayoutInfo.mSamplersTableRootIndex = static_cast<U32>(rootParams.size() - 1);
		}

		CD3D12GraphicsContext* pD3D12ImplContext = dynamic_cast<CD3D12GraphicsContext*>(mpGraphicsContext);
		TDE2_ASSERT(pD3D12ImplContext);

		ComPtr<ID3DBlob> pSignatureBlob = nullptr;
		ComPtr<ID3DBlob> pErrorBlob = nullptr;

		D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
		rootSignatureDesc.Flags         = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		rootSignatureDesc.NumParameters = static_cast<U32>(rootParams.size());
		rootSignatureDesc.pParameters   = rootParams.data();

		if (FAILED(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &pSignatureBlob, &pErrorBlob)))
		{
			LOG_ERROR(Wrench::StringUtils::Format("[CD3D12Shader] Root signature creation failed, msg: {0}", reinterpret_cast<const C8*>(pErrorBlob->GetBufferPointer())));
			return RC_FAIL;
		}

		if (FAILED(pD3D12ImplContext->GetDeviceContext()->CreateRootSignature(0, pSignatureBlob->GetBufferPointer(), pSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&mpRootSignature))))
		{
			return RC_FAIL;
		}

		return RC_OK;
	}

	E_RESULT_CODE CD3D12ShaderImpl::_createUniformBuffers(const TShaderCompilerOutput* pCompilerData)
	{
		E_RESULT_CODE result = _createRootSignature(pCompilerData);

		auto uniformBuffersInfo = pCompilerData->mUniformBuffersInfo;

		TUniformBufferDesc currDesc;

		mUniformBuffers.resize(uniformBuffersInfo.size() - TotalNumberOfInternalConstantBuffers);

		auto pGraphicsObjectManager = mpGraphicsContext->GetGraphicsObjectManager();

		/// here only user uniforms buffers are created
		for (auto iter = uniformBuffersInfo.cbegin(); iter != uniformBuffersInfo.cend(); ++iter)
		{
			currDesc = (*iter).second;

			/// skip internal buffers, because they are created separately by IGlobalShaderProperties implementation
			if (HasEnumFlag(currDesc.mFlags, E_UNIFORM_BUFFER_DESC_FLAGS::UBDF_INTERNAL))
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


	IShaderImpl* CreateD3D12ShaderImpl(IGraphicsContext* pGraphicsContext, const std::string& shaderId, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IShaderImpl, CD3D12ShaderImpl, result, pGraphicsContext, shaderId);
	}


	TDE2_DEFINE_SCOPED_PTR(CD3D12ShaderImpl);


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
			case E_TEXTURE_IMPL_TYPE::CUBEMAP:
			case E_TEXTURE_IMPL_TYPE::TEXTURE_2D_ARRAY:
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
		textureDesc.DepthOrArraySize = E_TEXTURE_IMPL_TYPE::TEXTURE_3D == params.mType ? params.mDepth : (E_TEXTURE_IMPL_TYPE::CUBEMAP == params.mType ? 6 : params.mArraySize);
		textureDesc.MipLevels        = params.mNumOfMipLevels;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Format           = HasEnumFlag(params.mBindFlags, E_BIND_GRAPHICS_TYPE::BIND_DEPTH_BUFFER) ?
			CD3D12Mappings::GetTypelessVersionOfFormat(params.mFormat) : CD3D12Mappings::GetDXGIFormat(params.mFormat);

		if (HasEnumFlag(params.mBindFlags, E_BIND_GRAPHICS_TYPE::BIND_RENDER_TARGET))
		{
			textureDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		}
		else if (HasEnumFlag(params.mBindFlags, E_BIND_GRAPHICS_TYPE::BIND_DEPTH_BUFFER))
		{
			textureDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		}
		else if (HasEnumFlag(params.mBindFlags, E_BIND_GRAPHICS_TYPE::BIND_UNORDERED_ACCESS))
		{
			textureDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		}

		D3D12MA::ALLOCATION_DESC allocationDesc{};
		allocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

		TCreatedImageInfo output{};

		if (FAILED(pAllocator->CreateResource(&allocationDesc, &textureDesc, D3D12_RESOURCE_STATE_COMMON/*D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE*/, nullptr, &output.mpAllocation, IID_PPV_ARGS(&output.mpTexture))))
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		return Wrench::TOkValue<TCreatedImageInfo>(output);
	}

	static TD3D12ResourceDescriptor CreateShaderResourceViewInternal(CD3D12GraphicsContext* pGraphicsContext, ComPtr<ID3D12Resource> pTextureResource, const TInitTextureImplParams& params)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc{};

		const bool isCubemap = (params.mType == E_TEXTURE_IMPL_TYPE::CUBEMAP);
		const bool isDepthBufferResource = HasEnumFlag(params.mBindFlags, E_BIND_GRAPHICS_TYPE::BIND_DEPTH_BUFFER);

		viewDesc.Format                  = CD3D12Mappings::GetDXGIFormat(isDepthBufferResource ? CD3D12Mappings::GetBestFitStrongTypeFormat(params.mFormat) : params.mFormat);
		viewDesc.ViewDimension           = isCubemap ? D3D12_SRV_DIMENSION_TEXTURECUBE : (params.mArraySize > 1 ? D3D12_SRV_DIMENSION_TEXTURE2DARRAY : D3D12_SRV_DIMENSION_TEXTURE2D);
		viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		if (E_TEXTURE_IMPL_TYPE::TEXTURE_3D == params.mType)
		{
			viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
		}

		switch (params.mType)
		{
			case E_TEXTURE_IMPL_TYPE::CUBEMAP:
				viewDesc.TextureCube.MipLevels       = params.mNumOfMipLevels;
				viewDesc.TextureCube.MostDetailedMip = 0;
				break;
			case E_TEXTURE_IMPL_TYPE::TEXTURE_2D:
				viewDesc.Texture2D.MipLevels       = params.mNumOfMipLevels;
				viewDesc.Texture2D.MostDetailedMip = 0;
				break;
			case E_TEXTURE_IMPL_TYPE::TEXTURE_2D_ARRAY:
				viewDesc.Texture2DArray.MipLevels = params.mNumOfMipLevels;
				viewDesc.Texture2DArray.ArraySize = params.mArraySize;
				viewDesc.Texture2DArray.FirstArraySlice = 0;
				viewDesc.Texture2DArray.MostDetailedMip = 0;
				break;
			case E_TEXTURE_IMPL_TYPE::TEXTURE_3D:
				viewDesc.Texture3D.MipLevels       = params.mNumOfMipLevels;
				viewDesc.Texture3D.MostDetailedMip = 0;
				break;
		}

		TD3D12ResourceDescriptor newResourceDescriptor = pGraphicsContext->GetDescriptorsAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)->AllocDescriptor();
		pGraphicsContext->GetDeviceContext()->CreateShaderResourceView(pTextureResource.Get(), &viewDesc, newResourceDescriptor.mCPUHandle);

		return newResourceDescriptor;
	}


	static TD3D12ResourceDescriptor CreateUnorderedAccessViewInternal(CD3D12GraphicsContext* pGraphicsContext, ComPtr<ID3D12Resource> pTextureResource, const TInitTextureImplParams& params, U32 mipSliceIndex = 0)
	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC viewDesc{};

		const bool isCubemap = (params.mType == E_TEXTURE_IMPL_TYPE::CUBEMAP);

		viewDesc.Format        = CD3D12Mappings::GetDXGIFormat(params.mFormat);
		viewDesc.ViewDimension = (params.mArraySize > 1 || isCubemap) ? D3D12_UAV_DIMENSION_TEXTURE2DARRAY : D3D12_UAV_DIMENSION_TEXTURE2D;

		if (E_TEXTURE_IMPL_TYPE::TEXTURE_3D == params.mType)
		{
			viewDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
		}

		switch (params.mType)
		{
			case E_TEXTURE_IMPL_TYPE::TEXTURE_2D:
				viewDesc.Texture2D.MipSlice = mipSliceIndex;
				break;
			case E_TEXTURE_IMPL_TYPE::TEXTURE_2D_ARRAY:
			case E_TEXTURE_IMPL_TYPE::CUBEMAP:
				viewDesc.Texture2DArray.ArraySize       = isCubemap ? 6 : params.mArraySize;
				viewDesc.Texture2DArray.MipSlice        = mipSliceIndex;
				viewDesc.Texture2DArray.FirstArraySlice = 0;
				break;
			case E_TEXTURE_IMPL_TYPE::TEXTURE_3D:
				viewDesc.Texture3D.WSize       = -1;
				viewDesc.Texture3D.MipSlice    = mipSliceIndex;
				viewDesc.Texture3D.FirstWSlice = 0;
				break;
		}

		TD3D12ResourceDescriptor newResourceDescriptor = pGraphicsContext->GetDescriptorsAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)->AllocDescriptor();
		pGraphicsContext->GetDeviceContext()->CreateUnorderedAccessView(pTextureResource.Get(), nullptr, &viewDesc, newResourceDescriptor.mCPUHandle);

		return newResourceDescriptor;
	}


	static TD3D12ResourceDescriptor CreateRenderTargetViewInternal(CD3D12GraphicsContext* pGraphicsContext, ComPtr<ID3D12Resource> pTextureResource, const TInitTextureImplParams& params)
	{
		TD3D12ResourceDescriptor renderTargetDescriptor = pGraphicsContext->GetDescriptorsAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_RTV)->AllocDescriptor();

		D3D12_RENDER_TARGET_VIEW_DESC viewDesc{};

		const bool isCubemap = (params.mType == E_TEXTURE_IMPL_TYPE::CUBEMAP);

		viewDesc.Format        = CD3D12Mappings::GetDXGIFormat(params.mFormat);
		viewDesc.ViewDimension = (params.mArraySize > 1 || isCubemap) ? D3D12_RTV_DIMENSION_TEXTURE2DARRAY : D3D12_RTV_DIMENSION_TEXTURE2D;

		switch (params.mType)
		{
			case E_TEXTURE_IMPL_TYPE::TEXTURE_2D:
				viewDesc.Texture2D.MipSlice = 0;
				break;
			case E_TEXTURE_IMPL_TYPE::TEXTURE_2D_ARRAY:
			case E_TEXTURE_IMPL_TYPE::CUBEMAP:
				viewDesc.Texture2DArray.ArraySize = isCubemap ? 6 : params.mArraySize;
				viewDesc.Texture2DArray.MipSlice = 0;
				viewDesc.Texture2DArray.FirstArraySlice = 0;
				break;
			case E_TEXTURE_IMPL_TYPE::TEXTURE_3D:
				viewDesc.Texture3D.FirstWSlice = 0;
				viewDesc.Texture3D.MipSlice = 0;
				viewDesc.Texture3D.WSize = -1;
				break;
		}

		pGraphicsContext->GetDeviceContext()->CreateRenderTargetView(pTextureResource.Get(), &viewDesc, renderTargetDescriptor.mCPUHandle);

		return renderTargetDescriptor;
	}


	static TD3D12ResourceDescriptor CreateDepthStencilViewInternal(CD3D12GraphicsContext* pGraphicsContext, ComPtr<ID3D12Resource> pTextureResource, const TInitTextureImplParams& params)
	{
		TD3D12ResourceDescriptor renderTargetDescriptor = pGraphicsContext->GetDescriptorsAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_DSV)->AllocDescriptor();

		D3D12_DEPTH_STENCIL_VIEW_DESC viewDesc{};

		const bool isCubemap = (params.mType == E_TEXTURE_IMPL_TYPE::CUBEMAP);

		viewDesc.Format        = CD3D12Mappings::GetDXGIFormat(params.mFormat);
		viewDesc.ViewDimension = (params.mArraySize > 1 || isCubemap) ? D3D12_DSV_DIMENSION_TEXTURE2DARRAY : D3D12_DSV_DIMENSION_TEXTURE2D;

		switch (params.mType)
		{
			case E_TEXTURE_IMPL_TYPE::TEXTURE_2D:
				viewDesc.Texture2D.MipSlice = 0;
				break;
			case E_TEXTURE_IMPL_TYPE::TEXTURE_2D_ARRAY:
			case E_TEXTURE_IMPL_TYPE::CUBEMAP:
			case E_TEXTURE_IMPL_TYPE::TEXTURE_3D:
				viewDesc.Texture2DArray.ArraySize = isCubemap ? 6 : params.mArraySize;
				viewDesc.Texture2DArray.MipSlice = 0;
				viewDesc.Texture2DArray.FirstArraySlice = 0;
				break;
		}

		pGraphicsContext->GetDeviceContext()->CreateDepthStencilView(pTextureResource.Get(), &viewDesc, renderTargetDescriptor.mCPUHandle);

		return renderTargetDescriptor;
	}


	CD3D12TextureImpl::CD3D12TextureImpl() :
		CBaseObject(), mCurrLayout(E_RESOURCE_LAYOUT::SHADER_RESOURCE), mHandle(TTextureHandleId::Invalid)
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

	const TD3D12ResourceDescriptor& CD3D12TextureImpl::GetShaderResourceDescriptor() const
	{
		return mShaderResourceDescriptor;
	}

	const TD3D12ResourceDescriptor& CD3D12TextureImpl::GetUnorderedAccessViewHandle(U32 subresourceIndex) const
	{
		return subresourceIndex < (std::numeric_limits<U32>::max)() ? mUnorderedAccessViewDescriptors[subresourceIndex] : mUnorderedAccessViewDescriptors.front();
	}

	const TD3D12ResourceDescriptor& CD3D12TextureImpl::GetRenderTargetDescriptor() const
	{
		return mRenderTargetDescriptor;
	}

	const TD3D12ResourceDescriptor& CD3D12TextureImpl::GetDepthBufferDescriptor() const
	{
		return mDepthStencilDescriptor;
	}

	Vector<U8> CD3D12TextureImpl::ReadBytes(U32 index)
	{
		TDE2_ASSERT(E_TEXTURE_IMPL_USAGE_TYPE::DYNAMIC == mInitParams.mUsageType && mpReadbackBuffer);

		Vector<U8> outputBytes(static_cast<size_t>(mInitParams.mWidth * mInitParams.mHeight * mInitParams.mDepth * CFormatUtils::GetFormatSize(mInitParams.mFormat)));

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
		
		mShaderResourceDescriptor = CreateShaderResourceViewInternal(mpGraphicsContextImpl, mpResource, mInitParams);

		if (HasEnumFlag(mInitParams.mBindFlags, E_BIND_GRAPHICS_TYPE::BIND_UNORDERED_ACCESS))
		{
			mUnorderedAccessViewDescriptors.emplace_back(CreateUnorderedAccessViewInternal(mpGraphicsContextImpl, mpResource, mInitParams));

			for (U32 i = 1; i < mInitParams.mNumOfMipLevels; ++i)
			{
				mUnorderedAccessViewDescriptors.emplace_back(CreateUnorderedAccessViewInternal(mpGraphicsContextImpl, mpResource, mInitParams, i));
			}
		}

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

		if (HasEnumFlag(mInitParams.mBindFlags, E_BIND_GRAPHICS_TYPE::BIND_DEPTH_BUFFER))
		{
			mDepthStencilDescriptor = CreateDepthStencilViewInternal(mpGraphicsContextImpl, mpResource, mInitParams);
		}
		else if (HasEnumFlag(mInitParams.mBindFlags, E_BIND_GRAPHICS_TYPE::BIND_RENDER_TARGET))
		{
			mRenderTargetDescriptor = CreateRenderTargetViewInternal(mpGraphicsContextImpl, mpResource, mInitParams);
		}

#if TDE2_DEBUG_MODE
		if (mInitParams.mName)
		{
			mpResource->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<U32>(strlen(mInitParams.mName)), mInitParams.mName);
		}
#endif

		return result;
	}

	E_RESULT_CODE CD3D12TextureImpl::_onFreeInternal()
	{
		mIsInitialized = false;

		mpGraphicsContextImpl->DestroyObjectDeffered(mpResource, mpAllocation);

		mpResource   = nullptr;
		mpAllocation = nullptr;

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
			void Bind(IGraphicsContext* pGraphicsContext, const CStaticArray<TBufferHandleId>& pVertexBuffersArray, IShaderImpl* pShader) override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CD3D12VertexDeclaration)
	};


	CD3D12VertexDeclaration::CD3D12VertexDeclaration() :
		CVertexDeclaration()
	{
	}

	void CD3D12VertexDeclaration::Bind(IGraphicsContext* pGraphicsContext, const CStaticArray<TBufferHandleId>& pVertexBuffersArray, IShaderImpl* pShader)
	{
	}


	IVertexDeclaration* CreateD3D12VertexDeclaration(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IVertexDeclaration, CD3D12VertexDeclaration, result);
	}


	/*!
		\brief CD3D12BasePipeline's definition
	*/

	CD3D12BasePipeline::CD3D12BasePipeline()
	{
	}

	E_RESULT_CODE CD3D12BasePipeline::Init(IGraphicsContext* pGraphicsContext)
	{
		mpD3D12GraphicsContext = dynamic_cast<CD3D12GraphicsContext*>(pGraphicsContext);
		mpD3D12GraphicsObjectManagerImpl = dynamic_cast<CD3D12GraphicsObjectManager*>(mpD3D12GraphicsContext->GetGraphicsObjectManager());

		return RC_OK;
	}

	const TD3D12PipelineLayoutInfo& CD3D12BasePipeline::GetLayoutInfo() const
	{
		return mLayoutInfo;
	}
	
	ComPtr<ID3D12PipelineState> CD3D12BasePipeline::GetNativePSO() const
	{
		return mpPipelineStateObject;
	}

	ID3D12RootSignature* CD3D12BasePipeline::GetRootSignature() const
	{
		return mpCachedRootSignature.Get();
	}

	U32 CD3D12BasePipeline::GetHash() const
	{
		return mConfigHash;
	}


	CD3D12GraphicsPipeline::CD3D12GraphicsPipeline() :
		CBaseGraphicsPipeline()
	{
	}

	E_RESULT_CODE CD3D12GraphicsPipeline::Init(IGraphicsContext* pGraphicsContext, IResourceManager* pResourceManager, const TGraphicsPipelineConfigDesc& pipelineConfig)
	{
		E_RESULT_CODE result = CBaseGraphicsPipeline::Init(pGraphicsContext, pResourceManager, pipelineConfig);
		if (RC_OK != result)
		{
			return result;
		}

		result = CD3D12BasePipeline::Init(pGraphicsContext);
		if (RC_OK != result)
		{
			return result;
		}

		mpPipelineStateObject = nullptr;

		auto loadShaderResult = mpD3D12GraphicsObjectManagerImpl->LoadShader(pipelineConfig.mShaderIdStr);
		if (loadShaderResult.HasError())
		{
			return loadShaderResult.GetError();
		}

		mShaderResourceHandle = loadShaderResult.Get();

		if (TPtr<CD3D12ShaderImpl> pShaderImpl = mpD3D12GraphicsObjectManagerImpl->GetD3D12ShaderPtr(mShaderResourceHandle))
		{
			mpCachedRootSignature = pShaderImpl->GetRootSignature();
			mLayoutInfo           = pShaderImpl->GetLayoutInfo();
		}

		mConfigHash = ComputeStateDescHash(mConfig);

		return RC_OK;
	}

	E_RESULT_CODE CD3D12GraphicsPipeline::Bind()
	{
		if (!mpGraphicsObjectManager)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = CBaseGraphicsPipeline::Bind();
		result = result | mpD3D12GraphicsContext->BindPipelineState(this);

		return result;
	}


	static D3D12_PRIMITIVE_TOPOLOGY_TYPE GetPrimitiveTopologyType(E_PRIMITIVE_TOPOLOGY_TYPE topologyType)
	{
		switch (topologyType)
		{
			case E_PRIMITIVE_TOPOLOGY_TYPE::PTT_POINT_LIST:
				return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
			case E_PRIMITIVE_TOPOLOGY_TYPE::PTT_LINE_LIST:
				return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
			case E_PRIMITIVE_TOPOLOGY_TYPE::PTT_TRIANGLE_LIST:
			case E_PRIMITIVE_TOPOLOGY_TYPE::PTT_TRIANGLE_STRIP:
			case E_PRIMITIVE_TOPOLOGY_TYPE::PTT_TRIANGLE_FAN:
				return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		}

		TDE2_UNREACHABLE();
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
	}


	ComPtr<ID3D12PipelineState> CD3D12GraphicsPipeline::GetPipelineForRenderPass(const TRenderPassInfo& renderPassInfo)
	{
		if (TShaderHandleId::Invalid == mShaderResourceHandle)
		{
			return nullptr;
		}

		TPtr<CD3D12ShaderImpl> pShader = mpD3D12GraphicsObjectManagerImpl->GetD3D12ShaderPtr(mShaderResourceHandle);
		if (!pShader)
		{
			return nullptr;
		}

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
		psoDesc.NumRenderTargets = 0;
		
		for (USIZE i = 0; i < renderPassInfo.mRenderTargetFormats.size(); ++i)
		{
			if (E_FORMAT_TYPE::FT_UNKNOWN == renderPassInfo.mRenderTargetFormats[i])
			{
				break;
			}

			psoDesc.RTVFormats[i] = CD3D12Mappings::GetDXGIFormat(renderPassInfo.mRenderTargetFormats[i]);
			++psoDesc.NumRenderTargets;
		}

		psoDesc.DSVFormat             = CD3D12Mappings::GetDXGIFormat(renderPassInfo.mDepthStencilFormat);
		psoDesc.pRootSignature        = pShader->GetRootSignature().Get();
		psoDesc.VS                    = pShader->GetPipelineShaderStage(E_SHADER_STAGE_TYPE::SST_VERTEX);
		psoDesc.GS                    = pShader->GetPipelineShaderStage(E_SHADER_STAGE_TYPE::SST_GEOMETRY);
		psoDesc.PS                    = pShader->GetPipelineShaderStage(E_SHADER_STAGE_TYPE::SST_PIXEL);
		psoDesc.PrimitiveTopologyType = GetPrimitiveTopologyType(mConfig.mTopology);
		psoDesc.DepthStencilState     = CD3D12Mappings::GetDepthStencilState(mConfig.mDepthStencilStateParams);
		psoDesc.BlendState            = CD3D12Mappings::GetBlendState(mConfig.mBlendStateParams);
		psoDesc.RasterizerState       = CD3D12Mappings::GetRasterizerState(mConfig.mRasterizerStateParams);
		psoDesc.SampleDesc.Count      = 1;
		psoDesc.SampleMask            = 0xffffffff;

		ComPtr<ID3D12PipelineState> pPSO = nullptr;

		if (FAILED(mpD3D12GraphicsContext->GetDeviceContext()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pPSO))))
		{
			TDE2_ASSERT_MSG(false, "[CD3D12GraphicsPipeline] PSO creation has failed");
		}

		return pPSO;
	}

	TDE2_DEFINE_SCOPED_PTR(CD3D12GraphicsPipeline);


	IGraphicsPipeline* CreateD3D12GraphicsPipeline(IGraphicsContext* pGraphicsContext, IResourceManager* pResourceManager, const TGraphicsPipelineConfigDesc& config, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IGraphicsPipeline, CD3D12GraphicsPipeline, result, pGraphicsContext, pResourceManager, config);
	}


	/*!
		\brief CD3D12ComputePipeline's definition
	*/

	CD3D12ComputePipeline::CD3D12ComputePipeline() :
		CBaseComputePipeline()
	{
	}

	E_RESULT_CODE CD3D12ComputePipeline::Init(IGraphicsContext* pGraphicsContext, IResourceManager* pResourceManager, const std::string& shaderId)
	{
		E_RESULT_CODE result = CBaseComputePipeline::Init(pGraphicsContext, pResourceManager, shaderId);
		if (RC_OK != result)
		{
			return result;
		}

		result = CD3D12BasePipeline::Init(pGraphicsContext);
		if (RC_OK != result)
		{
			return result;
		}

		auto loadShaderResult = mpD3D12GraphicsObjectManagerImpl->LoadShader(shaderId);
		if (loadShaderResult.HasError())
		{
			return loadShaderResult.GetError();
		}

		mShaderResourceHandle = loadShaderResult.Get();

		TPtr<CD3D12ShaderImpl> pShader = mpD3D12GraphicsObjectManagerImpl->GetD3D12ShaderPtr(mShaderResourceHandle);
		if (!pShader)
		{
			return RC_FAIL;
		}

		D3D12_COMPUTE_PIPELINE_STATE_DESC computePsoDesc{};
		computePsoDesc.pRootSignature = pShader->GetRootSignature().Get();
		computePsoDesc.CS             = pShader->GetPipelineShaderStage(E_SHADER_STAGE_TYPE::SST_COMPUTE);

		if (FAILED(mpD3D12GraphicsContext->GetDeviceContext()->CreateComputePipelineState(&computePsoDesc, IID_PPV_ARGS(&mpPipelineStateObject))))
		{
			return RC_FAIL;
		}

		mConfigHash = TDE2_STRING_ID(mShaderIdStr.c_str());
		mpCachedRootSignature = pShader->GetRootSignature();
		mLayoutInfo           = pShader->GetLayoutInfo();

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CD3D12ComputePipeline::Bind()
	{
		if (!mpGraphicsObjectManager)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = CBaseComputePipeline::Bind();
		result = result | mpD3D12GraphicsContext->BindPipelineState(this);

		return result;
	}


	TDE2_DEFINE_SCOPED_PTR(CD3D12ComputePipeline);


	IComputePipeline* CreateD3D12ComputePipeline(IGraphicsContext* pGraphicsContext, IResourceManager* pResourceManager, const std::string& shaderId, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComputePipeline, CD3D12ComputePipeline, result, pGraphicsContext, pResourceManager, shaderId);
	}
}

#endif