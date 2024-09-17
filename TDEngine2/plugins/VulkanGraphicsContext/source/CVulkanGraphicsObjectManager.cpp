#include "../include/CVulkanGraphicsObjectManager.h"
#include "../include/CVulkanGraphicsContext.h"
#include "../include/CVulkanMappings.h"
#include "../include/CVulkanUtils.h"
#include "../include/CVulkanTexture.h"
#include "../include/CVulkanBuffer.h"
#include "../include/CVulkanVertexDeclaration.h"
#include <core/IFileSystem.h>
#include <core/IFile.h>
#include <core/CProjectSettings.h>


namespace TDEngine2
{
	TDE2_DEFINE_SCOPED_PTR(CVulkanTextureImpl)
	TDE2_DEFINE_SCOPED_PTR(CVulkanBuffer)


	CVulkanGraphicsObjectManager::CVulkanGraphicsObjectManager() :
		CBaseGraphicsObjectManager()
	{
	}

	TResult<TBufferHandleId> CVulkanGraphicsObjectManager::CreateBuffer(const TInitBufferParams& params)
	{
		E_RESULT_CODE result = RC_OK;

		TPtr<IBuffer> pBuffer = TPtr<IBuffer>(CreateVulkanBuffer(mpGraphicsContext, params, result));
		if (!pBuffer || RC_OK != result)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		auto it = std::find(mpBuffersArray.begin(), mpBuffersArray.end(), nullptr);
		const USIZE placementIndex = static_cast<USIZE>(std::distance(mpBuffersArray.begin(), it));

		if (placementIndex >= mpBuffersArray.size())
		{
			mpBuffersArray.emplace_back(DynamicPtrCast<CVulkanBuffer>(pBuffer));
		}
		else
		{
			mpBuffersArray[placementIndex] = DynamicPtrCast<CVulkanBuffer>(pBuffer);
		}

		return Wrench::TOkValue<TBufferHandleId>(static_cast<TBufferHandleId>(placementIndex));
	}

	TResult<TTextureHandleId> CVulkanGraphicsObjectManager::CreateTexture(const TInitTextureImplParams& params)
	{
		E_RESULT_CODE result = RC_OK;

		TPtr<ITextureImpl> pTexture = TPtr<ITextureImpl>(CreateVulkanTextureImpl(mpGraphicsContext, params, result));
		if (!pTexture || RC_OK != result)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		auto it = std::find(mpTexturesArray.begin(), mpTexturesArray.end(), nullptr);
		const USIZE placementIndex = static_cast<USIZE>(std::distance(mpTexturesArray.begin(), it));

		if (placementIndex >= mpTexturesArray.size())
		{
			mpTexturesArray.emplace_back(DynamicPtrCast<CVulkanTextureImpl>(pTexture));
		}
		else
		{
			mpTexturesArray[placementIndex] = DynamicPtrCast<CVulkanTextureImpl>(pTexture);
		}

		return Wrench::TOkValue<TTextureHandleId>(static_cast<TTextureHandleId>(placementIndex));
	}

	E_RESULT_CODE CVulkanGraphicsObjectManager::DestroyBuffer(TBufferHandleId bufferHandle)
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

		mpBuffersArray[bufferPlacementIndex] = nullptr;

		return RC_OK;
	}

	E_RESULT_CODE CVulkanGraphicsObjectManager::DestroyTexture(TTextureHandleId textureHandle)
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

		mpTexturesArray[texturePlacementIndex] = nullptr;

		return RC_OK;
	}

	TResult<IVertexDeclaration*> CVulkanGraphicsObjectManager::CreateVertexDeclaration()
	{
		E_RESULT_CODE result = RC_OK;

		IVertexDeclaration* pNewVertDecl = CreateVulkanVertexDeclaration(result);

		if (result != RC_OK)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		_insertVertexDeclaration(pNewVertDecl);

		return Wrench::TOkValue<IVertexDeclaration*>(pNewVertDecl);
	}

	TResult<TTextureSamplerId> CVulkanGraphicsObjectManager::CreateTextureSampler(const TTextureSamplerDesc& samplerDesc)
	{
		U32 hashValue = ComputeStateDescHash(samplerDesc);
		if (mTextureSamplesHashTable.find(hashValue) != mTextureSamplesHashTable.cend())
		{
			return Wrench::TOkValue<TTextureSamplerId>(TTextureSamplerId(mTextureSamplesHashTable[hashValue]));
		}

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = CVulkanMappings::GetFilterType(samplerDesc.mFilteringType);
		samplerInfo.minFilter = CVulkanMappings::GetFilterType(samplerDesc.mFilteringType);
		samplerInfo.addressModeU = CVulkanMappings::GetTextureAddressMode(samplerDesc.mUAddressMode);
		samplerInfo.addressModeV = CVulkanMappings::GetTextureAddressMode(samplerDesc.mVAddressMode);
		samplerInfo.addressModeW = CVulkanMappings::GetTextureAddressMode(samplerDesc.mWAddressMode);
		samplerInfo.anisotropyEnable = VK_FALSE;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		auto pVulkanGraphicsContext = dynamic_cast<CVulkanGraphicsContext*>(mpGraphicsContext);
		
		VkSampler samplerHandle = VK_NULL_HANDLE;
		VK_SAFE_TRESULT_CALL(vkCreateSampler(pVulkanGraphicsContext->GetDevice(), &samplerInfo, nullptr, &samplerHandle));

		U32 samplerId = static_cast<U32>(mTextureSamplersArray.size());

		mTextureSamplersArray.push_back(samplerHandle);
		mTextureSamplesHashTable.insert({ hashValue, samplerId });

		return Wrench::TOkValue<TTextureSamplerId>(TTextureSamplerId(samplerId));
	}

	TResult<TBlendStateId> CVulkanGraphicsObjectManager::CreateBlendState(const TBlendStateDesc& blendStateDesc)
	{
		TDE2_UNIMPLEMENTED();
		return Wrench::TErrValue<E_RESULT_CODE>(RC_NOT_IMPLEMENTED_YET);
		/*U32 hashValue = ComputeStateDescHash(blendStateDesc);
		if (mBlendStatesHashTable.find(hashValue) != mBlendStatesHashTable.cend())
		{
			return Wrench::TOkValue<TBlendStateId>(TBlendStateId(mBlendStatesHashTable[hashValue]));
		}

		const U32 stateId = static_cast<U32>(mBlendStates.Add(blendStateDesc));
		mBlendStatesHashTable.insert({ hashValue, stateId });

		return Wrench::TOkValue<TBlendStateId>(TBlendStateId(stateId));*/
	}

	TResult<TDepthStencilStateId> CVulkanGraphicsObjectManager::CreateDepthStencilState(const TDepthStencilStateDesc& depthStencilDesc)
	{
		TDE2_UNIMPLEMENTED();
		return Wrench::TErrValue<E_RESULT_CODE>(RC_NOT_IMPLEMENTED_YET);
		//return Wrench::TOkValue<TDepthStencilStateId>(TDepthStencilStateId(mDepthStencilStates.Add(depthStencilDesc)));
	}

	TResult<TRasterizerStateId> CVulkanGraphicsObjectManager::CreateRasterizerState(const TRasterizerStateDesc& rasterizerStateDesc)
	{
		TDE2_UNIMPLEMENTED();
		return Wrench::TErrValue<E_RESULT_CODE>(RC_NOT_IMPLEMENTED_YET);
		//return Wrench::TOkValue<TRasterizerStateId>(TRasterizerStateId(mRasterizerStates.Add(rasterizerStateDesc)));
	}

	TPtr<IBuffer> CVulkanGraphicsObjectManager::GetBufferPtr(TBufferHandleId handle)
	{
		return DynamicPtrCast<IBuffer>(GetVulkanBufferPtr(handle));
	}

	TPtr<CVulkanBuffer> CVulkanGraphicsObjectManager::GetVulkanBufferPtr(TBufferHandleId bufferHandle)
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

	TPtr<ITextureImpl> CVulkanGraphicsObjectManager::GetTexturePtr(TTextureHandleId handle)
	{
		return DynamicPtrCast<ITextureImpl>(GetVulkanTexturePtr(handle));
	}

	TPtr<CVulkanTextureImpl> CVulkanGraphicsObjectManager::GetVulkanTexturePtr(TTextureHandleId textureHandle)
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

	std::string CVulkanGraphicsObjectManager::GetDefaultShaderCode(const E_DEFAULT_SHADER_TYPE& type) const
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

	std::array<TVector4, 3> CVulkanGraphicsObjectManager::GetScreenTriangleVertices() const
	{
		// \note first two components are xy position, latter two are uv coordinates of the triangle
		return { TVector4(-1.0f, -1.0f, 0.0f, 0.0f), TVector4(-1.0f, 3.0f, 0.0f, 2.0f), TVector4(3.0f, -1.0f, 2.0f, 0.0f) };
	}

	E_RESULT_CODE CVulkanGraphicsObjectManager::_freeTextureSamplers()
	{
		return RC_OK;
	}

	E_RESULT_CODE CVulkanGraphicsObjectManager::_freeBlendStates()
	{
		return RC_OK;
	}

	E_RESULT_CODE CVulkanGraphicsObjectManager::_freeDepthStencilStates()
	{
		return RC_OK;
	}

	E_RESULT_CODE CVulkanGraphicsObjectManager::_freeRasterizerStates()
	{
		return RC_OK;
	}

	const std::string CVulkanGraphicsObjectManager::_getShaderCacheFilePath() const
	{
		return Wrench::StringUtils::Format(CProjectSettings::Get()->mGraphicsSettings.mShaderCachePathPattern, "Vulkan");
	}


	IGraphicsObjectManager* CreateVulkanGraphicsObjectManager(IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IGraphicsObjectManager, CVulkanGraphicsObjectManager, result, pGraphicsContext);
	}
}