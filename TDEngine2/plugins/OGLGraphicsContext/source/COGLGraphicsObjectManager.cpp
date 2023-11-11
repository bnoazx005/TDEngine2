#include "./../include/COGLGraphicsObjectManager.h"
#include "./../include/COGLBuffer.h"
#include "./../include/COGLVertexDeclaration.h"
#include "./../include/COGLMappings.h"
#include "./../include/COGLUtils.h"
#include <core/IFileSystem.h>
#include <core/IFile.h>
#include <core/CProjectSettings.h>


namespace TDEngine2
{
	COGLGraphicsObjectManager::COGLGraphicsObjectManager() :
		CBaseGraphicsObjectManager()
	{
	}

	TResult<TBufferHandleId> COGLGraphicsObjectManager::CreateBuffer(const TInitBufferParams& params)
	{
		E_RESULT_CODE result = RC_OK;

		TPtr<IBuffer> pBuffer = TPtr<IBuffer>(CreateOGLBuffer(mpGraphicsContext, params, result));
		if (!pBuffer || RC_OK != result)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		return Wrench::TOkValue<TBufferHandleId>(_insertBuffer(pBuffer));
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