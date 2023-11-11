#include "../include/CVulkanGraphicsObjectManager.h"
#include "../include/CVulkanGraphicsContext.h"
#include "../include/CVulkanMappings.h"
#include "../include/CVulkanUtils.h"
#include "../include/CVulkanBuffer.h"
#include "../include/CVulkanVertexDeclaration.h"
#include <core/IFileSystem.h>
#include <core/IFile.h>
#include <core/CProjectSettings.h>


namespace TDEngine2
{
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

		return Wrench::TOkValue<TBufferHandleId>(_insertBuffer(pBuffer));
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
		TDE2_UNIMPLEMENTED();
		return Wrench::TErrValue<E_RESULT_CODE>(RC_NOT_IMPLEMENTED_YET);
		/*U32 hashValue = ComputeStateDescHash(samplerDesc);
		if (mTextureSamplesHashTable.find(hashValue) != mTextureSamplesHashTable.cend())
		{
			return Wrench::TOkValue<TTextureSamplerId>(TTextureSamplerId(mTextureSamplesHashTable[hashValue]));
		}

		GLuint samplerHandler = 0x0;

		glGenSamplers(1, &samplerHandler);

		if (glGetError() != GL_NO_ERROR)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}
		
		glSamplerParameteri(samplerHandler, GL_TEXTURE_WRAP_S, CVulkanMappings::GetTextureAddressMode(samplerDesc.mUAddressMode));
		glSamplerParameteri(samplerHandler, GL_TEXTURE_WRAP_T, CVulkanMappings::GetTextureAddressMode(samplerDesc.mVAddressMode));
		glSamplerParameteri(samplerHandler, GL_TEXTURE_WRAP_R, CVulkanMappings::GetTextureAddressMode(samplerDesc.mWAddressMode));
		glSamplerParameteri(samplerHandler, GL_TEXTURE_MAG_FILTER, CVulkanMappings::GetMagFilterType(samplerDesc.mFilteringType));
		glSamplerParameteri(samplerHandler, GL_TEXTURE_MIN_FILTER, CVulkanMappings::GetMinFilterType(samplerDesc.mFilteringType, samplerDesc.mUseMipMaps));
		glSamplerParameteri(samplerHandler, GL_TEXTURE_MIN_LOD, 0);

		U32 samplerId = static_cast<U32>(mTextureSamplersArray.size());

		mTextureSamplersArray.push_back(samplerHandler);
		mTextureSamplesHashTable.insert({ hashValue, samplerId });

		return Wrench::TOkValue<TTextureSamplerId>(TTextureSamplerId(samplerId));*/
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