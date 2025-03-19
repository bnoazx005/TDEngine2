#include "../include/CD3D12GraphicsObjectManager.h"
#include "../include/CD3D12GraphicsContext.h"
#include "../include/CD3D12Mappings.h"
#include "../include/CD3D12Resources.h"
#include <core/IFileSystem.h>
#include <core/IFile.h>
#include <core/CProjectSettings.h>


namespace TDEngine2
{
	CD3D12GraphicsObjectManager::CD3D12GraphicsObjectManager() :
		CBaseGraphicsObjectManager()
	{
	}

	TResult<TBufferHandleId> CD3D12GraphicsObjectManager::CreateBuffer(const TInitBufferParams& params)
	{
		E_RESULT_CODE result = RC_OK;

		if (E_GRAPHICS_RESOURCE_INIT_FLAGS::TRANSIENT == (params.mFlags & E_GRAPHICS_RESOURCE_INIT_FLAGS::TRANSIENT))
		{
			const U32 hash = ComputeStateDescHash(params);

			auto&& it = mTransientBuffersPool.find(hash);
			if (it != mTransientBuffersPool.cend())
			{
				auto& availableTransientBuffers = it->second;
				if (!availableTransientBuffers.empty())
				{
					const TBufferHandleId resourceId = availableTransientBuffers.back();
					availableTransientBuffers.pop_back();

					return TResult<TBufferHandleId>(resourceId);
				}
			}
		}

		TPtr<IBuffer> pBuffer = TPtr<IBuffer>(CreateD3D12Buffer(mpGraphicsContext, params, result));
		if (!pBuffer || RC_OK != result)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		auto it = std::find(mpBuffersArray.begin(), mpBuffersArray.end(), nullptr);
		const USIZE placementIndex = static_cast<USIZE>(std::distance(mpBuffersArray.begin(), it));

		if (placementIndex >= mpBuffersArray.size())
		{
			mpBuffersArray.emplace_back(DynamicPtrCast<CD3D12Buffer>(pBuffer));
		}
		else
		{
			mpBuffersArray[placementIndex] = DynamicPtrCast<CD3D12Buffer>(pBuffer);
		}

		return Wrench::TOkValue<TBufferHandleId>(static_cast<TBufferHandleId>(placementIndex));
	}

	TResult<TTextureHandleId> CD3D12GraphicsObjectManager::CreateTexture(const TInitTextureImplParams& params)
	{
		E_RESULT_CODE result = RC_OK;

		if (E_GRAPHICS_RESOURCE_INIT_FLAGS::TRANSIENT == (params.mFlags & E_GRAPHICS_RESOURCE_INIT_FLAGS::TRANSIENT))
		{
			const U32 hash = ComputeStateDescHash(params);

			auto&& it = mTransientTexturesPool.find(hash);
			if (it != mTransientTexturesPool.cend())
			{
				auto& availableTransientTextures = it->second;
				if (!availableTransientTextures.empty())
				{
					const TTextureHandleId resourceId = availableTransientTextures.back();
					availableTransientTextures.pop_back();

					return TResult<TTextureHandleId>(resourceId);
				}
			}
		}

		TPtr<ITextureImpl> pTexture = TPtr<ITextureImpl>(CreateD3D12TextureImpl(mpGraphicsContext, params, result));
		if (!pTexture || RC_OK != result)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		auto it = std::find(mpTexturesArray.begin(), mpTexturesArray.end(), nullptr);
		const USIZE placementIndex = static_cast<USIZE>(std::distance(mpTexturesArray.begin(), it));

		if (placementIndex >= mpTexturesArray.size())
		{
			mpTexturesArray.emplace_back(DynamicPtrCast<CD3D12TextureImpl>(pTexture));
		}
		else
		{
			mpTexturesArray[placementIndex] = DynamicPtrCast<CD3D12TextureImpl>(pTexture);
		}

		pTexture->SetHandle(static_cast<TTextureHandleId>(placementIndex), _getPassKey());

		return Wrench::TOkValue<TTextureHandleId>(static_cast<TTextureHandleId>(placementIndex));
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

		//VkSamplerCreateInfo samplerInfo{};
		//samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		//samplerInfo.magFilter = CD3D12Mappings::GetFilterType(samplerDesc.mFilteringType);
		//samplerInfo.minFilter = CD3D12Mappings::GetFilterType(samplerDesc.mFilteringType);
		//samplerInfo.addressModeU = CD3D12Mappings::GetTextureAddressMode(samplerDesc.mUAddressMode);
		//samplerInfo.addressModeV = CD3D12Mappings::GetTextureAddressMode(samplerDesc.mVAddressMode);
		//samplerInfo.addressModeW = CD3D12Mappings::GetTextureAddressMode(samplerDesc.mWAddressMode);
		//samplerInfo.anisotropyEnable = VK_FALSE;
		//samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		//samplerInfo.unnormalizedCoordinates = VK_FALSE;
		//samplerInfo.compareEnable = VK_FALSE;
		//samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		//samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		//samplerInfo.mipLodBias = 0.0f;
		//samplerInfo.minLod = 0.0f;
		//samplerInfo.maxLod = 0.0f;

		//auto pD3D12GraphicsContext = dynamic_cast<CD3D12GraphicsContext*>(mpGraphicsContext);

		//VkSampler samplerHandle = VK_NULL_HANDLE;
		//VK_SAFE_TRESULT_CALL(vkCreateSampler(pD3D12GraphicsContext->GetDevice(), &samplerInfo, nullptr, &samplerHandle));

		//U32 samplerId = static_cast<U32>(mTextureSamplersArray.size());

		/*mTextureSamplersArray.push_back(samplerHandle);
		mTextureSamplesHashTable.insert({ hashValue, samplerId });

		return Wrench::TOkValue<TTextureSamplerId>(TTextureSamplerId(samplerId));*/
		return Wrench::TOkValue<TTextureSamplerId>(TTextureSamplerId(0));
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

	/*TResult<VkSampler> CD3D12GraphicsObjectManager::GetTextureSampler(TTextureSamplerId texSamplerId) const
	{
		const USIZE textureSamplerIndex = static_cast<USIZE>(texSamplerId);
		if (textureSamplerIndex >= mTextureSamplersArray.size())
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		return Wrench::TOkValue<VkSampler>(mTextureSamplersArray[textureSamplerIndex]);
	}*/

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
		TDE2_UNIMPLEMENTED();
		return nullptr;
		//return TPtr<IGraphicsPipeline>(CreateD3D12GraphicsPipeline(mpGraphicsContext, pResourceManager, pipelineConfigDesc, result));
	}

	TPtr<IComputePipeline> CD3D12GraphicsObjectManager::_createComputePipelineInternal(IResourceManager* pResourceManager, const std::string& shaderId)
	{
		E_RESULT_CODE result = RC_OK;
		TDE2_UNIMPLEMENTED();
		return nullptr;
		//return TPtr<IComputePipeline>(CreateD3D12ComputePipeline(mpGraphicsContext, pResourceManager, shaderId, result));
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


	IGraphicsObjectManager* CreateD3D12GraphicsObjectManager(IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IGraphicsObjectManager, CD3D12GraphicsObjectManager, result, pGraphicsContext);
	}
}