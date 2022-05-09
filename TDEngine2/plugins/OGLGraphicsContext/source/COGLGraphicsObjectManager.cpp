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

	TResult<IVertexBuffer*> COGLGraphicsObjectManager::CreateVertexBuffer(E_BUFFER_USAGE_TYPE usageType, USIZE totalBufferSize, const void* pDataPtr)
	{
		E_RESULT_CODE result = RC_OK;

		IVertexBuffer* pNewVertexBuffer = CreateOGLVertexBuffer(mpGraphicsContext, usageType, totalBufferSize, pDataPtr, result);

		if (result != RC_OK)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		_insertBuffer(pNewVertexBuffer);

		return Wrench::TOkValue<IVertexBuffer*>(pNewVertexBuffer);
	}

	TResult<IIndexBuffer*> COGLGraphicsObjectManager::CreateIndexBuffer(E_BUFFER_USAGE_TYPE usageType, E_INDEX_FORMAT_TYPE indexFormatType,
																		USIZE totalBufferSize, const void* pDataPtr)
	{
		E_RESULT_CODE result = RC_OK;

		IIndexBuffer* pNewIndexBuffer = CreateOGLIndexBuffer(mpGraphicsContext, usageType, indexFormatType, totalBufferSize, pDataPtr, result);

		if (result != RC_OK)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		_insertBuffer(pNewIndexBuffer);

		return Wrench::TOkValue<IIndexBuffer*>(pNewIndexBuffer);
	}

	TResult<IConstantBuffer*> COGLGraphicsObjectManager::CreateConstantBuffer(E_BUFFER_USAGE_TYPE usageType, USIZE totalBufferSize, const void* pDataPtr)
	{
		E_RESULT_CODE result = RC_OK;

		IConstantBuffer* pNewConstantBuffer = CreateOGLConstantBuffer(mpGraphicsContext, usageType, totalBufferSize, pDataPtr, result);

		if (result != RC_OK)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		_insertBuffer(pNewConstantBuffer);

		return Wrench::TOkValue<IConstantBuffer*>(pNewConstantBuffer);
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
				
			case E_DEFAULT_SHADER_TYPE::DST_EDITOR_UI:
				return R"(
					#version 330 core

					#include <TDEngine2Globals.inc>

					#define VERTEX_ENTRY main
					#define PIXEL_ENTRY main

					#program vertex

					layout (location = 0) in vec4 inPosUV;
					layout (location = 1) in vec4 inColor;

					out vec2 VertOutUV;
					out vec4 VertOutColor;

					void main(void)
					{
						gl_Position  = ModelMat * vec4(inPosUV.xy, 0.0, 1.0);
						VertOutUV    = inPosUV.zw;
						VertOutColor = inColor;
					}

					#endprogram

					#program pixel

					in vec2 VertOutUV;
					in vec4 VertOutColor;

					out vec4 FragColor;

					DECLARE_TEX2D(Texture);

					void main(void)
					{
						FragColor = VertOutColor * TEX2D(Texture, VertOutUV);
					}
					#endprogram
					)";

			case E_DEFAULT_SHADER_TYPE::DST_SCREEN_SPACE:
				return R"(					
					#version 330 core

					#include <TDEngine2Globals.inc>

					#define VERTEX_ENTRY main
					#define PIXEL_ENTRY main

					#program vertex

					layout (location = 0) in vec4 inPosUV;

					out vec2 VertOutUV;

					void main(void)
					{
						gl_Position = vec4(inPosUV.xy, 0.0f, 1.0);
						VertOutUV   = inPosUV.zw;
					}

					#endprogram

					#program pixel

					in vec2 VertOutUV;

					out vec4 FragColor;

					DECLARE_TEX2D(FrameTexture);

					void main(void)
					{
						FragColor = TEX2D(FrameTexture, VertOutUV);
					}
					#endprogram
				)";

			case E_DEFAULT_SHADER_TYPE::DST_SELECTION:
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

					out uint FragColor;

					void main(void)
					{
						FragColor = uint(ObjectID + uint(1));
					}
					#endprogram
					)";

			case E_DEFAULT_SHADER_TYPE::DST_SELECTION_SKINNED:
				return R"(
					#version 330 core

					#include <TDEngine2Globals.inc>

					#define VERTEX_ENTRY main
					#define PIXEL_ENTRY main

					#program vertex

					#include <TDEngine2SkinningUtils.inc>

					layout (location = 0) in vec4 inlPos;
					layout (location = 1) in vec4 inJointWeights;
					layout (location = 2) in vec4 inJointIndices;

					void main(void)
					{
						gl_Position = ProjMat * ViewMat * ModelMat * ComputeSkinnedVertexPos(inlPos, inJointWeights, inJointIndices);
					}

					#endprogram

					#program pixel

					out uint FragColor;

					void main(void)
					{
						FragColor = uint(ObjectID + uint(1));
					}
					#endprogram
					)";

			case E_DEFAULT_SHADER_TYPE::DST_SELECTION_UI:
				return R"(
					#version 330 core

					#include <TDEngine2Globals.inc>

					#define VERTEX_ENTRY main
					#define PIXEL_ENTRY main

					#program vertex

					layout (location = 0) in vec4 inPosUV;
					layout (location = 1) in vec4 inColor;

					out vec2 VertOutUV;
					out vec4 VertOutColor;

					void main(void)
					{
						gl_Position  = ModelMat * vec4(inPosUV.xy, 0.0, 1.0);
						VertOutUV    = inPosUV.zw;
						VertOutColor = inColor;
					}

					#endprogram

					#program pixel

					out uint FragColor;

					void main(void)
					{
						FragColor = uint(ObjectID + uint(1));
					}
					#endprogram
					)";

			case E_DEFAULT_SHADER_TYPE::DST_SELECTION_OUTLINE:
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
						FragColor = vec4(1.0, 0.0, 1.0, abs(sin(4.0 * Time.x)));
					}
					#endprogram
					)";

			case E_DEFAULT_SHADER_TYPE::DST_SELECTION_SKINNED_OUTLINE:
				return R"(
					#version 330 core

					#include <TDEngine2Globals.inc>

					#define VERTEX_ENTRY main
					#define PIXEL_ENTRY main

					#program vertex

					#include <TDEngine2SkinningUtils.inc>

					layout (location = 0) in vec4 inlPos;
					layout (location = 1) in vec4 inJointWeights;
					layout (location = 2) in vec4 inJointIndices;

					void main(void)
					{
						gl_Position = ProjMat * ViewMat * ModelMat * ComputeSkinnedVertexPos(inlPos, inJointWeights, inJointIndices);
					}

					#endprogram

					#program pixel

					out vec4 FragColor;

					void main(void)
					{
						FragColor = vec4(1.0, 0.0, 1.0, abs(sin(4.0 * Time.x)));
					}
					#endprogram
					)";

			case E_DEFAULT_SHADER_TYPE::DST_SHADOW_PASS:
				return R"(
					#version 330 core

					#include <TDEngine2Globals.inc>

					#define VERTEX_ENTRY main
					#define PIXEL_ENTRY main

					#program vertex

					layout (location = 0) in vec4 inlPos;

					void main(void)
					{
						gl_Position = SunLightMat * ModelMat * inlPos;
					}

					#endprogram

					#program pixel
					void main(void) {}
					#endprogram
					)";

			case E_DEFAULT_SHADER_TYPE::DST_SHADOW_SKINNED_PASS:
				return R"(
					#version 330 core

					#include <TDEngine2Globals.inc>

					#define VERTEX_ENTRY main
					#define PIXEL_ENTRY main

					#program vertex

					#include <TDEngine2SkinningUtils.inc>

					layout (location = 0) in vec4 inlPos;
					layout (location = 1) in vec4 inJointWeights;
					layout (location = 2) in vec4 inJointIndices;

					void main(void)
					{
						gl_Position = SunLightMat * ModelMat * ComputeSkinnedVertexPos(inlPos, inJointWeights, inJointIndices);
					}

					#endprogram

					#program pixel
					void main(void) {}
					#endprogram
					)";

			case E_DEFAULT_SHADER_TYPE::DST_TONE_MAPPING_PASS:
				return R"(
					#version 330 core

					#include <TDEngine2Globals.inc>

					#define VERTEX_ENTRY main
					#define PIXEL_ENTRY main

					#program vertex

					layout (location = 0) in vec4 inPosUV;

					out vec2 VertOutUV;

					void main(void)
					{
						gl_Position = vec4(inPosUV.xy, 0.0f, 1.0);
						VertOutUV   = inPosUV.zw;
					}

					#endprogram

					#program pixel

					in vec2 VertOutUV;

					out vec4 FragColor;

					DECLARE_TEX2D(FrameTexture);
					DECLARE_TEX2D(ColorGradingLUT);

					CBUFFER_SECTION_EX(ToneMappingParameters, 4)
						vec4 toneMappingParams; // x  weight (0 is disabled, 1 is enabled), y - exposure
						vec4 colorGradingParams; // x - weight (enabled or not)
					CBUFFER_ENDSECTION

					// \todo Move this into TDEngine2EffectsUtils.inc later
					vec3 ApplyGrading(in vec3 color)
					{
						const float ColorsNum = 32.0;
						const float MaxColor = ColorsNum - 1.0;
					
						float cell = floor(color.b * MaxColor);

						float halfLUTxPixel = 0.5 / 1024.0; 
						float halfLUTyPixel = 0.5 / 32.0;

						const float threshold = MaxColor / ColorsNum;

						float xOffset = halfLUTxPixel + color.r / ColorsNum * threshold;
						float yOffset = halfLUTyPixel + color.g * threshold; 

						vec2 lutPos = vec2(cell / ColorsNum + xOffset, yOffset); 
						return TEX2D(ColorGradingLUT, lutPos).rgb;
					}

					void main(void)
					{
						vec4 color = TEX2D(FrameTexture, VertOutUV);
						vec3 mappedColor = mix(color.rgb, 1 - exp(-color.rgb * toneMappingParams.y), toneMappingParams.x);
						
						FragColor = vec4(mix(mappedColor, ApplyGrading(mappedColor), colorGradingParams.x)/*LinearToGamma(mappedColor)*/, color.a); // disabled because of SRGB back buffer
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


	IGraphicsObjectManager* CreateOGLGraphicsObjectManager(IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IGraphicsObjectManager, COGLGraphicsObjectManager, result, pGraphicsContext);
	}
}