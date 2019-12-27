#include "./../include/COGLGraphicsObjectManager.h"
#include "./../include/COGLVertexBuffer.h"
#include "./../include/COGLIndexBuffer.h"
#include "./../include/COGLConstantBuffer.h"
#include "./../include/COGLVertexDeclaration.h"
#include "./../include/COGLMappings.h"
#include "./../include/COGLUtils.h"


namespace TDEngine2
{
	COGLGraphicsObjectManager::COGLGraphicsObjectManager() :
		CBaseGraphicsObjectManager()
	{
	}

	TResult<IVertexBuffer*> COGLGraphicsObjectManager::CreateVertexBuffer(E_BUFFER_USAGE_TYPE usageType, U32 totalBufferSize, const void* pDataPtr)
	{
		E_RESULT_CODE result = RC_OK;

		IVertexBuffer* pNewVertexBuffer = CreateOGLVertexBuffer(mpGraphicsContext, usageType, totalBufferSize, pDataPtr, result);

		if (result != RC_OK)
		{
			return TErrorValue<E_RESULT_CODE>(result);
		}

		_insertBuffer(pNewVertexBuffer);

		return TOkValue<IVertexBuffer*>(pNewVertexBuffer);
	}

	TResult<IIndexBuffer*> COGLGraphicsObjectManager::CreateIndexBuffer(E_BUFFER_USAGE_TYPE usageType, E_INDEX_FORMAT_TYPE indexFormatType,
																		U32 totalBufferSize, const void* pDataPtr)
	{
		E_RESULT_CODE result = RC_OK;

		IIndexBuffer* pNewIndexBuffer = CreateOGLIndexBuffer(mpGraphicsContext, usageType, indexFormatType, totalBufferSize, pDataPtr, result);

		if (result != RC_OK)
		{
			return TErrorValue<E_RESULT_CODE>(result);
		}

		_insertBuffer(pNewIndexBuffer);

		return TOkValue<IIndexBuffer*>(pNewIndexBuffer);
	}

	TResult<IConstantBuffer*> COGLGraphicsObjectManager::CreateConstantBuffer(E_BUFFER_USAGE_TYPE usageType, U32 totalBufferSize, const void* pDataPtr)
	{
		E_RESULT_CODE result = RC_OK;

		IConstantBuffer* pNewConstantBuffer = CreateOGLConstantBuffer(mpGraphicsContext, usageType, totalBufferSize, pDataPtr, result);

		if (result != RC_OK)
		{
			return TErrorValue<E_RESULT_CODE>(result);
		}

		_insertBuffer(pNewConstantBuffer);

		return TOkValue<IConstantBuffer*>(pNewConstantBuffer);
	}

	TResult<IVertexDeclaration*> COGLGraphicsObjectManager::CreateVertexDeclaration()
	{
		E_RESULT_CODE result = RC_OK;

		IVertexDeclaration* pNewVertDecl = CreateOGLVertexDeclaration(result);

		if (result != RC_OK)
		{
			return TErrorValue<E_RESULT_CODE>(result);
		}

		_insertVertexDeclaration(pNewVertDecl);

		return TOkValue<IVertexDeclaration*>(pNewVertDecl);
	}

	TResult<TTextureSamplerId> COGLGraphicsObjectManager::CreateTextureSampler(const TTextureSamplerDesc& samplerDesc)
	{
		U32 hashValue = ComputeStateDescHash<const TTextureSamplerDesc&>(samplerDesc);
		if (mTextureSamplesHashTable.find(hashValue) != mTextureSamplesHashTable.cend())
		{
			return TOkValue<TTextureSamplerId>(mTextureSamplesHashTable[hashValue]);
		}

		GLuint samplerHandler = 0x0;

		glGenSamplers(1, &samplerHandler);

		if (glGetError() != GL_NO_ERROR)
		{
			return TErrorValue<E_RESULT_CODE>(RC_FAIL);
		}
		
		glSamplerParameteri(samplerHandler, GL_TEXTURE_WRAP_S, COGLMappings::GetTextureAddressMode(samplerDesc.mUAddressMode));
		glSamplerParameteri(samplerHandler, GL_TEXTURE_WRAP_T, COGLMappings::GetTextureAddressMode(samplerDesc.mVAddressMode));
		glSamplerParameteri(samplerHandler, GL_TEXTURE_WRAP_R, COGLMappings::GetTextureAddressMode(samplerDesc.mWAddressMode));
		glSamplerParameteri(samplerHandler, GL_TEXTURE_MAG_FILTER, COGLMappings::GetMagFilterType(samplerDesc.mFilterFlags));
		glSamplerParameteri(samplerHandler, GL_TEXTURE_MIN_FILTER, COGLMappings::GetMinFilterType(samplerDesc.mFilterFlags));
		glSamplerParameteri(samplerHandler, GL_TEXTURE_MIN_LOD, 0);

		U32 samplerId = mTextureSamplersArray.size();

		mTextureSamplersArray.push_back(samplerHandler);
		mTextureSamplesHashTable.insert({ hashValue, samplerId });

		return TOkValue<TTextureSamplerId>(samplerId);
	}

	TResult<TBlendStateId> COGLGraphicsObjectManager::CreateBlendState(const TBlendStateDesc& blendStateDesc)
	{
		U32 hashValue = ComputeStateDescHash<const TBlendStateDesc&>(blendStateDesc);
		if (mBlendStatesHashTable.find(hashValue) != mBlendStatesHashTable.cend())
		{
			return TOkValue<TBlendStateId>(mBlendStatesHashTable[hashValue]);
		}

		auto stateId = mBlendStates.Add(blendStateDesc);
		mBlendStatesHashTable.insert({ hashValue, stateId });

		return TOkValue<TBlendStateId>(stateId);
	}

	TResult<TDepthStencilStateId> COGLGraphicsObjectManager::CreateDepthStencilState(const TDepthStencilStateDesc& depthStencilDesc)
	{
		return TOkValue<TDepthStencilStateId>(mDepthStencilStates.Add(depthStencilDesc));
	}

	TResult<TRasterizerStateId> COGLGraphicsObjectManager::CreateRasterizerState(const TRasterizerStateDesc& rasterizerStateDesc)
	{
		return TOkValue<TRasterizerStateId>(mRasterizerStates.Add(rasterizerStateDesc));
	}

	TResult<GLuint> COGLGraphicsObjectManager::GetTextureSampler(TTextureSamplerId texSamplerId) const
	{
		if (texSamplerId >= mTextureSamplersArray.size())
		{
			return TErrorValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}

		return TOkValue<GLuint>(mTextureSamplersArray[texSamplerId]);
	}

	TResult<TBlendStateDesc> COGLGraphicsObjectManager::GetBlendState(TBlendStateId blendStateId) const
	{
		return mBlendStates[blendStateId];
	}

	TResult<TDepthStencilStateDesc> COGLGraphicsObjectManager::GetDepthStencilState(TDepthStencilStateId depthStencilStateId) const
	{
		return mDepthStencilStates[depthStencilStateId];
	}

	TResult<TRasterizerStateDesc> COGLGraphicsObjectManager::GetRasterizerState(TRasterizerStateId rasterizerStateId) const
	{
		return mRasterizerStates[rasterizerStateId];
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

					#if VERTEX

					layout (location = 0) in vec4 inlPos;

					void main(void)
					{
						gl_Position = ProjMat * ViewMat * ModelMat * inlPos;
					}

					#endif
					#if PIXEL

					out vec4 FragColor;

					void main(void)
					{
						FragColor = vec4(1.0, 0.0, 1.0, 1.0);
					}
					#endif
					)";
				
			case E_DEFAULT_SHADER_TYPE::DST_EDITOR_UI:
				return R"(
					#version 330 core

					#include <TDEngine2Globals.inc>

					#define VERTEX_ENTRY main
					#define PIXEL_ENTRY main

					#if VERTEX

					layout (location = 0) in vec4 inPosUV;
					layout (location = 1) in vec4 inColor;

					out vec2 VertOutUV;
					out vec4 VertOutColor;

					void main(void)
					{
						gl_Position  = ProjMat * vec4(inPosUV.xy, 0.0, 1.0);
						VertOutUV    = inPosUV.zw;
						VertOutColor = inColor;
					}

					#endif
					#if PIXEL

					in vec2 VertOutUV;
					in vec4 VertOutColor;

					out vec4 FragColor;

					DECLARE_TEX2D(Texture);

					void main(void)
					{
						FragColor = VertOutColor * TEX2D(Texture, VertOutUV);
					}
					#endif
					)";

			default:
				TDE2_UNIMPLEMENTED();
				break;
		}

		return "";
		
	}

	E_RESULT_CODE COGLGraphicsObjectManager::_freeTextureSamplers()
	{
		if (mTextureSamplersArray.empty())
		{
			return RC_OK;
		}

		GL_SAFE_CALL(glDeleteSamplers(mTextureSamplersArray.size(), &mTextureSamplersArray[0]));

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


	IGraphicsObjectManager* CreateOGLGraphicsObjectManager(IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		COGLGraphicsObjectManager* pManagerInstance = new (std::nothrow) COGLGraphicsObjectManager();

		if (!pManagerInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pManagerInstance->Init(pGraphicsContext);

		if (result != RC_OK)
		{
			delete pManagerInstance;

			pManagerInstance = nullptr;
		}

		return pManagerInstance;
	}
}