#include "../include/COGLGraphicsObjectManager.h"
#include "../include/COGLBuffer.h"
#include "../include/COGLTexture.h"
#include "../include/COGLVertexDeclaration.h"
#include "../include/COGLMappings.h"
#include "../include/COGLUtils.h"
#include <core/IFileSystem.h>
#include <core/IFile.h>
#include <core/CProjectSettings.h>


namespace TDEngine2
{
	TDE2_DEFINE_SCOPED_PTR(COGLTextureImpl)
	TDE2_DEFINE_SCOPED_PTR(COGLBuffer)


	COGLGraphicsObjectManager::COGLGraphicsObjectManager() :
		CBaseGraphicsObjectManager()
	{
	}

	TResult<TBufferHandleId> COGLGraphicsObjectManager::CreateBuffer(const TInitBufferParams& params)
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

		TPtr<IBuffer> pBuffer = TPtr<IBuffer>(CreateOGLBuffer(mpGraphicsContext, params, result));
		if (!pBuffer || RC_OK != result)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		auto it = std::find(mpBuffersArray.begin(), mpBuffersArray.end(), nullptr);
		const USIZE placementIndex = static_cast<USIZE>(std::distance(mpBuffersArray.begin(), it));

		if (placementIndex >= mpBuffersArray.size())
		{
			mpBuffersArray.emplace_back(DynamicPtrCast<COGLBuffer>(pBuffer));
		}
		else
		{
			mpBuffersArray[placementIndex] = DynamicPtrCast<COGLBuffer>(pBuffer);
		}

		return Wrench::TOkValue<TBufferHandleId>(static_cast<TBufferHandleId>(placementIndex));
	}

	TResult<TTextureHandleId> COGLGraphicsObjectManager::CreateTexture(const TInitTextureImplParams& params)
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

		TPtr<ITextureImpl> pTexture = TPtr<ITextureImpl>(CreateOGLTextureImpl(mpGraphicsContext, params, result));
		if (!pTexture || RC_OK != result)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		auto it = std::find(mpTexturesArray.begin(), mpTexturesArray.end(), nullptr);
		const USIZE placementIndex = static_cast<USIZE>(std::distance(mpTexturesArray.begin(), it));

		if (placementIndex >= mpTexturesArray.size())
		{
			mpTexturesArray.emplace_back(DynamicPtrCast<COGLTextureImpl>(pTexture));
		}
		else
		{
			mpTexturesArray[placementIndex] = DynamicPtrCast<COGLTextureImpl>(pTexture);
		}

		return Wrench::TOkValue<TTextureHandleId>(static_cast<TTextureHandleId>(placementIndex));
	}

	E_RESULT_CODE COGLGraphicsObjectManager::DestroyBuffer(TBufferHandleId bufferHandle)
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

	E_RESULT_CODE COGLGraphicsObjectManager::DestroyTexture(TTextureHandleId textureHandle)
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

	TResult<IVertexDeclaration*> COGLGraphicsObjectManager::CreateVertexDeclaration()
	{
		E_RESULT_CODE result = RC_OK;

		IVertexDeclaration* pNewVertDecl = CreateOGLVertexDeclaration(result);

		if (result != RC_OK)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		_insertVertexDeclaration(pNewVertDecl);

		return Wrench::TOkValue<IVertexDeclaration*>(pNewVertDecl);
	}

	TResult<TTextureSamplerId> COGLGraphicsObjectManager::CreateTextureSampler(const TTextureSamplerDesc& samplerDesc)
	{
		U32 hashValue = ComputeStateDescHash(samplerDesc);
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
		
		glSamplerParameteri(samplerHandler, GL_TEXTURE_WRAP_S, COGLMappings::GetTextureAddressMode(samplerDesc.mUAddressMode));
		glSamplerParameteri(samplerHandler, GL_TEXTURE_WRAP_T, COGLMappings::GetTextureAddressMode(samplerDesc.mVAddressMode));
		glSamplerParameteri(samplerHandler, GL_TEXTURE_WRAP_R, COGLMappings::GetTextureAddressMode(samplerDesc.mWAddressMode));
		glSamplerParameteri(samplerHandler, GL_TEXTURE_MAG_FILTER, COGLMappings::GetMagFilterType(samplerDesc.mFilteringType));
		glSamplerParameteri(samplerHandler, GL_TEXTURE_MIN_FILTER, COGLMappings::GetMinFilterType(samplerDesc.mFilteringType, samplerDesc.mUseMipMaps));
		glSamplerParameteri(samplerHandler, GL_TEXTURE_MIN_LOD, 0);

		U32 samplerId = static_cast<U32>(mTextureSamplersArray.size());

		mTextureSamplersArray.push_back(samplerHandler);
		mTextureSamplesHashTable.insert({ hashValue, samplerId });

		return Wrench::TOkValue<TTextureSamplerId>(TTextureSamplerId(samplerId));
	}

	TResult<TBlendStateId> COGLGraphicsObjectManager::CreateBlendState(const TBlendStateDesc& blendStateDesc)
	{
		U32 hashValue = ComputeStateDescHash(blendStateDesc);
		if (mBlendStatesHashTable.find(hashValue) != mBlendStatesHashTable.cend())
		{
			return Wrench::TOkValue<TBlendStateId>(TBlendStateId(mBlendStatesHashTable[hashValue]));
		}

		const U32 stateId = static_cast<U32>(mBlendStates.Add(blendStateDesc));
		mBlendStatesHashTable.insert({ hashValue, stateId });

		return Wrench::TOkValue<TBlendStateId>(TBlendStateId(stateId));
	}

	TResult<TDepthStencilStateId> COGLGraphicsObjectManager::CreateDepthStencilState(const TDepthStencilStateDesc& depthStencilDesc)
	{
		return Wrench::TOkValue<TDepthStencilStateId>(TDepthStencilStateId(mDepthStencilStates.Add(depthStencilDesc)));
	}

	TResult<TRasterizerStateId> COGLGraphicsObjectManager::CreateRasterizerState(const TRasterizerStateDesc& rasterizerStateDesc)
	{
		return Wrench::TOkValue<TRasterizerStateId>(TRasterizerStateId(mRasterizerStates.Add(rasterizerStateDesc)));
	}

	TResult<GLuint> COGLGraphicsObjectManager::GetTextureSampler(TTextureSamplerId texSamplerId) const
	{
		U32 texSamplerIdValue = static_cast<U32>(texSamplerId);

		if (texSamplerIdValue >= mTextureSamplersArray.size())
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}

		return Wrench::TOkValue<GLuint>(mTextureSamplersArray[texSamplerIdValue]);
	}

	TResult<TBlendStateDesc> COGLGraphicsObjectManager::GetBlendState(TBlendStateId blendStateId) const
	{
		return mBlendStates[static_cast<U32>(blendStateId)];
	}

	TResult<TDepthStencilStateDesc> COGLGraphicsObjectManager::GetDepthStencilState(TDepthStencilStateId depthStencilStateId) const
	{
		return mDepthStencilStates[static_cast<U32>(depthStencilStateId)];
	}

	TResult<TRasterizerStateDesc> COGLGraphicsObjectManager::GetRasterizerState(TRasterizerStateId rasterizerStateId) const
	{
		return mRasterizerStates[static_cast<U32>(rasterizerStateId)];
	}

	TPtr<IBuffer> COGLGraphicsObjectManager::GetBufferPtr(TBufferHandleId handle)
	{
		return DynamicPtrCast<IBuffer>(GetOGLBufferPtr(handle));
	}

	TPtr<COGLBuffer> COGLGraphicsObjectManager::GetOGLBufferPtr(TBufferHandleId bufferHandle)
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

	TPtr<ITextureImpl> COGLGraphicsObjectManager::GetTexturePtr(TTextureHandleId handle)
	{
		return DynamicPtrCast<ITextureImpl>(GetOGLTexturePtr(handle));
	}

	TPtr<COGLTextureImpl> COGLGraphicsObjectManager::GetOGLTexturePtr(TTextureHandleId textureHandle)
	{
		if (TTextureHandleId::Invalid == textureHandle)
		{
			return nullptr;
		}

		const USIZE bufferPlacementIndex = static_cast<USIZE>(textureHandle);
		if (bufferPlacementIndex >= mpTexturesArray.size())
		{
			return nullptr;
		}

		return mpTexturesArray[bufferPlacementIndex];
	}

	std::string COGLGraphicsObjectManager::GetDefaultShaderCode(const E_DEFAULT_SHADER_TYPE& type) const
	{
		switch (type)
		{
			case E_DEFAULT_SHADER_TYPE::DST_BASIC:
				return R"(
					#version 330 core

					#include <TDEngine2Globals.inc>

					#define VERTEX_ENTRY main
					#define PIXEL_ENTRY main

					#program vertex

					layout (location = 0) in vec4 inlPos;

					void main(void)
					{
						gl_Position = ProjMat * ViewMat * ModelMat * inlPos;
					}

					#endprogram

					#program pixel

					out vec4 FragColor;

					void main(void)
					{
						FragColor = vec4(1.0, 0.0, 1.0, 1.0);
					}
					#endprogram
					)";
				
			default:
				TDE2_UNIMPLEMENTED();
				break;
		}

		return "";
		
	}

	std::array<TVector4, 3> COGLGraphicsObjectManager::GetScreenTriangleVertices() const
	{
		// \note first two components are xy position, latter two are uv coordinates of the triangle
		return { TVector4(-1.0f, -1.0f, 0.0f, 0.0f), TVector4(-1.0f, 3.0f, 0.0f, 2.0f), TVector4(3.0f, -1.0f, 2.0f, 0.0f) };
	}

	E_RESULT_CODE COGLGraphicsObjectManager::_freeTextureSamplers()
	{
		if (mTextureSamplersArray.empty())
		{
			return RC_OK;
		}

		GL_SAFE_CALL(glDeleteSamplers(static_cast<GLsizei>(mTextureSamplersArray.size()), &mTextureSamplersArray.front()));

		mTextureSamplersArray.clear();

		return RC_OK;
	}

	E_RESULT_CODE COGLGraphicsObjectManager::_freeBlendStates()
	{
		return RC_OK;
	}

	E_RESULT_CODE COGLGraphicsObjectManager::_freeDepthStencilStates()
	{
		return RC_OK;
	}

	E_RESULT_CODE COGLGraphicsObjectManager::_freeRasterizerStates()
	{
		return RC_OK;
	}

	const std::string COGLGraphicsObjectManager::_getShaderCacheFilePath() const
	{
		return Wrench::StringUtils::Format(CProjectSettings::Get()->mGraphicsSettings.mShaderCachePathPattern, "GL");
	}


	IGraphicsObjectManager* CreateOGLGraphicsObjectManager(IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IGraphicsObjectManager, COGLGraphicsObjectManager, result, pGraphicsContext);
	}
}