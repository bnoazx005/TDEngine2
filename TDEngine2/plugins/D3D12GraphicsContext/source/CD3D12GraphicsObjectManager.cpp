#include "../include/CD3D12GraphicsObjectManager.h"
#include "../include/CD3D12GraphicsContext.h"
#include "../include/CD3D12Mappings.h"
#include "../include/CD3D12Resources.h"
#include <core/CProjectSettings.h>


namespace TDEngine2
{
	CD3D12GraphicsObjectManager::CD3D12GraphicsObjectManager() :
		CBaseGraphicsObjectManager()
	{
	}

	E_RESULT_CODE CD3D12GraphicsObjectManager::DestroyBuffer(TBufferHandleId bufferHandle)
	{
		if (TBufferHandleId::Invalid == bufferHandle)
		{
			return RC_INVALID_ARGS;
		}

		const USIZE bufferPlacementIndex = static_cast<USIZE>(bufferHandle);
		if (bufferPlacementIndex >= mpBuffersArray.size())
		{
			return RC_FAIL;
		}

		const auto& bufferParams = mpBuffersArray[bufferPlacementIndex]->GetParams();

		if (E_GRAPHICS_RESOURCE_INIT_FLAGS::TRANSIENT == (bufferParams.mFlags & E_GRAPHICS_RESOURCE_INIT_FLAGS::TRANSIENT))
		{
			mTransientBuffersPool[ComputeStateDescHash(bufferParams)].push_back(bufferHandle);
			return RC_OK;
		}

		mpBuffersArray[bufferPlacementIndex] = nullptr;

		return RC_OK;
	}

	E_RESULT_CODE CD3D12GraphicsObjectManager::DestroyTexture(TTextureHandleId textureHandle)
	{
		if (TTextureHandleId::Invalid == textureHandle)
		{
			return RC_INVALID_ARGS;
		}

		const USIZE texturePlacementIndex = static_cast<USIZE>(textureHandle);
		if (texturePlacementIndex >= mpTexturesArray.size())
		{
			return RC_FAIL;
		}

		const auto& textureParams = mpTexturesArray[texturePlacementIndex]->GetParams();

		if (E_GRAPHICS_RESOURCE_INIT_FLAGS::TRANSIENT == (textureParams.mFlags & E_GRAPHICS_RESOURCE_INIT_FLAGS::TRANSIENT))
		{
			mTransientTexturesPool[ComputeStateDescHash(textureParams)].push_back(textureHandle);
			return RC_OK;
		}

		mpTexturesArray[texturePlacementIndex] = nullptr;

		return RC_OK;
	}

	E_RESULT_CODE CD3D12GraphicsObjectManager::DestroyShader(TShaderHandleId shaderHandle)
	{
		if (TShaderHandleId::Invalid == shaderHandle)
		{
			return RC_INVALID_ARGS;
		}

		const USIZE shaderPlacementIndex = static_cast<USIZE>(shaderHandle);
		if (shaderPlacementIndex >= mpTexturesArray.size())
		{
			return RC_FAIL;
		}

		mpShadersArray[shaderPlacementIndex] = nullptr;

		return RC_OK;
	}

	TResult<IVertexDeclaration*> CD3D12GraphicsObjectManager::CreateVertexDeclaration()
	{
		E_RESULT_CODE result = RC_OK;

		IVertexDeclaration* pNewVertDecl = CreateD3D12VertexDeclaration(result);

		if (result != RC_OK)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		_insertVertexDeclaration(pNewVertDecl);

		return Wrench::TOkValue<IVertexDeclaration*>(pNewVertDecl);
	}

	TResult<TTextureSamplerId> CD3D12GraphicsObjectManager::CreateTextureSampler(const TTextureSamplerDesc& samplerDesc)
	{
		U32 hashValue = ComputeStateDescHash(samplerDesc);
		if (mTextureSamplesHashTable.find(hashValue) != mTextureSamplesHashTable.cend())
		{
			return Wrench::TOkValue<TTextureSamplerId>(TTextureSamplerId(mTextureSamplesHashTable[hashValue]));
		}

		D3D12_SAMPLER_DESC samplerDescInfo{};

		samplerDescInfo.AddressU = CD3D12Mappings::GetTextureAddressMode(samplerDesc.mUAddressMode);
		samplerDescInfo.AddressV = CD3D12Mappings::GetTextureAddressMode(samplerDesc.mVAddressMode);
		samplerDescInfo.AddressW = CD3D12Mappings::GetTextureAddressMode(samplerDesc.mWAddressMode);
		samplerDescInfo.Filter   = CD3D12Mappings::GetFilterType(samplerDesc.mFilteringType);
		samplerDescInfo.MinLOD   = 0.0f;
		samplerDescInfo.MaxLOD   = D3D12_FLOAT32_MAX;

		CD3D12GraphicsContext* pD3D12GraphicsContext = dynamic_cast<CD3D12GraphicsContext*>(mpGraphicsContext);
		
		TPtr<ID3D12CPUDescriptorsAllocator> pDescriptorsAllocator = pD3D12GraphicsContext->GetDescriptorsAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
		if (!pDescriptorsAllocator)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		const TD3D12ResourceDescriptor& samplerDescriptor = pDescriptorsAllocator->AllocDescriptor();

		pD3D12GraphicsContext->GetDeviceContext()->CreateSampler(&samplerDescInfo, samplerDescriptor.mCPUHandle);
		
		const U32 samplerId = static_cast<U32>(mTextureSamplersArray.size());

		mTextureSamplersArray.emplace_back(samplerDescriptor);
		mTextureSamplesHashTable.insert({ hashValue, samplerId });

		return Wrench::TOkValue<TTextureSamplerId>(TTextureSamplerId(samplerId));
	}

	TPtr<IBuffer> CD3D12GraphicsObjectManager::GetBufferPtr(TBufferHandleId handle)
	{
		return DynamicPtrCast<IBuffer>(GetD3D12BufferPtr(handle));
	}

	TPtr<CD3D12Buffer> CD3D12GraphicsObjectManager::GetD3D12BufferPtr(TBufferHandleId bufferHandle)
	{
		if (TBufferHandleId::Invalid == bufferHandle)
		{
			return nullptr;
		}

		const USIZE bufferPlacementIndex = static_cast<USIZE>(bufferHandle);
		if (bufferPlacementIndex >= mpBuffersArray.size())
		{
			return nullptr;
		}

		return mpBuffersArray[bufferPlacementIndex];
	}

	TPtr<ITextureImpl> CD3D12GraphicsObjectManager::GetTexturePtr(TTextureHandleId handle)
	{
		return DynamicPtrCast<ITextureImpl>(GetD3D12TexturePtr(handle));
	}

	TPtr<CD3D12TextureImpl> CD3D12GraphicsObjectManager::GetD3D12TexturePtr(TTextureHandleId textureHandle)
	{
		if (TTextureHandleId::Invalid == textureHandle)
		{
			return nullptr;
		}

		const USIZE texturePlacementIndex = static_cast<USIZE>(textureHandle);
		if (texturePlacementIndex >= mpTexturesArray.size())
		{
			return nullptr;
		}

		return mpTexturesArray[texturePlacementIndex];
	}

	TPtr<IShaderImpl> CD3D12GraphicsObjectManager::GetShaderPtr(TShaderHandleId handle)
	{
		return DynamicPtrCast<IShaderImpl>(GetD3D12ShaderPtr(handle));
	}

	TPtr<CD3D12ShaderImpl> CD3D12GraphicsObjectManager::GetD3D12ShaderPtr(TShaderHandleId handle)
	{
		if (TShaderHandleId::Invalid == handle)
		{
			return nullptr;
		}

		const USIZE placementIndex = static_cast<USIZE>(handle);
		if (placementIndex >= mpShadersArray.size())
		{
			return nullptr;
		}

		return mpShadersArray[placementIndex];
	}

	TResult<TD3D12ResourceDescriptor> CD3D12GraphicsObjectManager::GetTextureSampler(TTextureSamplerId texSamplerId) const
	{
		const USIZE textureSamplerIndex = static_cast<USIZE>(texSamplerId);
		if (textureSamplerIndex >= mTextureSamplersArray.size())
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		return Wrench::TOkValue<TD3D12ResourceDescriptor>(mTextureSamplersArray[textureSamplerIndex]);
	}

	std::string CD3D12GraphicsObjectManager::GetDefaultShaderCode(const E_DEFAULT_SHADER_TYPE& type) const
	{
		switch (type)
		{
			case E_DEFAULT_SHADER_TYPE::DST_BASIC:
				return R"(
					#define VERTEX_ENTRY mainVS
					#define PIXEL_ENTRY mainPS

					#include <TDEngine2Globals.inc>

					#program vertex

					float4 mainVS(float4 lPos : POSITION0): SV_POSITION
					{
						return mul(ProjMat, mul(ViewMat, mul(ModelMat, lPos)));
					}

					#endprogram

					#program pixel

					float4 mainPS(float4 wPos : SV_POSITION): SV_TARGET0
					{
						return float4(1.0, 0.0, 1.0, 1.0);
					}
					#endprogram
					)";
				
			default:
				TDE2_UNIMPLEMENTED();
				break;
		}

		return "";
		
	}

	std::array<TVector4, 3> CD3D12GraphicsObjectManager::GetScreenTriangleVertices() const
	{
		// \note first two components are xy position, latter two are uv coordinates of the triangle
		return { TVector4(-1.0f, -1.0f, 0.0f, 0.0f), TVector4(-1.0f, 3.0f, 0.0f, 2.0f), TVector4(3.0f, -1.0f, 2.0f, 0.0f) };
	}

	TPtr<IGraphicsPipeline> CD3D12GraphicsObjectManager::_createGraphicsPipelineInternal(IResourceManager* pResourceManager, const TGraphicsPipelineConfigDesc& pipelineConfigDesc)
	{
		E_RESULT_CODE result = RC_OK;
		return TPtr<IGraphicsPipeline>(CreateD3D12GraphicsPipeline(mpGraphicsContext, pResourceManager, pipelineConfigDesc, result));
	}

	TPtr<IComputePipeline> CD3D12GraphicsObjectManager::_createComputePipelineInternal(IResourceManager* pResourceManager, const std::string& shaderId)
	{
		E_RESULT_CODE result = RC_OK;
		return TPtr<IComputePipeline>(CreateD3D12ComputePipeline(mpGraphicsContext, pResourceManager, shaderId, result));
	}

	TPtr<IBuffer> CD3D12GraphicsObjectManager::_createBufferInternal(const TInitBufferParams& params)
	{
		E_RESULT_CODE result = RC_OK;
		return TPtr<IBuffer>(CreateD3D12Buffer(mpGraphicsContext, params, result));
	}

	TPtr<ITextureImpl> CD3D12GraphicsObjectManager::_createTextureInternal(const TInitTextureImplParams& params)
	{
		E_RESULT_CODE result = RC_OK;
		return TPtr<ITextureImpl>(CreateD3D12TextureImpl(mpGraphicsContext, params, result));
	}

	TPtr<IShaderImpl> CD3D12GraphicsObjectManager::_createShaderImplInternal(const std::string& shaderId)
	{
		E_RESULT_CODE result = RC_OK;
		return TPtr<IShaderImpl>(CreateD3D12ShaderImpl(mpGraphicsContext, shaderId, result));
	}

	USIZE CD3D12GraphicsObjectManager::_insertBuffer(TPtr<IBuffer> pObject)
	{
		return PlaceObjectAtFirstNullPosition(mpBuffersArray, DynamicPtrCast<CD3D12Buffer>(pObject));
	}

	USIZE CD3D12GraphicsObjectManager::_insertTexture(TPtr<ITextureImpl> pObject)
	{
		return PlaceObjectAtFirstNullPosition(mpTexturesArray, DynamicPtrCast<CD3D12TextureImpl>(pObject));
	}

	USIZE CD3D12GraphicsObjectManager::_insertShaderImpl(TPtr<IShaderImpl> pObject)
	{
		return PlaceObjectAtFirstNullPosition(mpShadersArray, DynamicPtrCast<CD3D12ShaderImpl>(pObject));
	}

	E_RESULT_CODE CD3D12GraphicsObjectManager::_freeTextureSamplers()
	{
		return RC_OK;
	}

	E_RESULT_CODE CD3D12GraphicsObjectManager::_freeBlendStates()
	{
		return RC_OK;
	}

	E_RESULT_CODE CD3D12GraphicsObjectManager::_freeDepthStencilStates()
	{
		return RC_OK;
	}

	E_RESULT_CODE CD3D12GraphicsObjectManager::_freeRasterizerStates()
	{
		return RC_OK;
	}

	const std::string CD3D12GraphicsObjectManager::_getShaderCacheFilePath() const
	{
		return Wrench::StringUtils::Format(CProjectSettings::Get()->mGraphicsSettings.mShaderCachePathPattern, "D3D12");
	}


	IGraphicsObjectManager* CreateD3D12GraphicsObjectManager(IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IGraphicsObjectManager, CD3D12GraphicsObjectManager, result, pGraphicsContext, pFileSystem);
	}
}