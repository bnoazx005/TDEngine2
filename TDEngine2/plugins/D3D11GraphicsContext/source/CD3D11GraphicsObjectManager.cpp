#include "./../include/CD3D11GraphicsObjectManager.h"
#include "./../include/CD3D11VertexBuffer.h"
#include "./../include/CD3D11IndexBuffer.h"
#include "./../include/CD3D11ConstantBuffer.h"
#include "./../include/CD3D11VertexDeclaration.h"
#include "./../include/CD3D11Mappings.h"
#include <core/IGraphicsContext.h>


#if defined (TDE2_USE_WIN32PLATFORM)

namespace TDEngine2
{
	CD3D11GraphicsObjectManager::CD3D11GraphicsObjectManager():
		CBaseGraphicsObjectManager()
	{
	}

	TResult<IVertexBuffer*> CD3D11GraphicsObjectManager::CreateVertexBuffer(E_BUFFER_USAGE_TYPE usageType, U32 totalBufferSize, const void* pDataPtr)
	{
		E_RESULT_CODE result = RC_OK;

		IVertexBuffer* pNewVertexBuffer = CreateD3D11VertexBuffer(mpGraphicsContext, usageType, totalBufferSize, pDataPtr, result);

		if (result != RC_OK)
		{
			return TErrorValue<E_RESULT_CODE>(result);
		}

		_insertBuffer(pNewVertexBuffer);

		return TOkValue<IVertexBuffer*>(pNewVertexBuffer);
	}

	TResult<IIndexBuffer*> CD3D11GraphicsObjectManager::CreateIndexBuffer(E_BUFFER_USAGE_TYPE usageType, E_INDEX_FORMAT_TYPE indexFormatType,
																		  U32 totalBufferSize, const void* pDataPtr)
	{
		E_RESULT_CODE result = RC_OK;

		IIndexBuffer* pNewIndexBuffer = CreateD3D11IndexBuffer(mpGraphicsContext, usageType, indexFormatType, totalBufferSize, pDataPtr, result);

		if (result != RC_OK)
		{
			return TErrorValue<E_RESULT_CODE>(result);
		}

		_insertBuffer(pNewIndexBuffer);

		return TOkValue<IIndexBuffer*>(pNewIndexBuffer);
	}

	TResult<IConstantBuffer*> CD3D11GraphicsObjectManager::CreateConstantBuffer(E_BUFFER_USAGE_TYPE usageType, U32 totalBufferSize, const void* pDataPtr)
	{
		E_RESULT_CODE result = RC_OK;

		IConstantBuffer* pNewConstantBuffer = CreateD3D11ConstantBuffer(mpGraphicsContext, usageType, totalBufferSize, pDataPtr, result);

		if (result != RC_OK)
		{
			return TErrorValue<E_RESULT_CODE>(result);
		}

		_insertBuffer(pNewConstantBuffer);

		return TOkValue<IConstantBuffer*>(pNewConstantBuffer);
	}
	
	TResult<IVertexDeclaration*> CD3D11GraphicsObjectManager::CreateVertexDeclaration()
	{
		E_RESULT_CODE result = RC_OK;

		IVertexDeclaration* pNewVertDecl = CreateD3D11VertexDeclaration(result);

		if (result != RC_OK)
		{
			return TErrorValue<E_RESULT_CODE>(result);
		}

		_insertVertexDeclaration(pNewVertDecl);

		return TOkValue<IVertexDeclaration*>(pNewVertDecl);
	}

	TResult<TTextureSamplerId> CD3D11GraphicsObjectManager::CreateTextureSampler(const TTextureSamplerDesc& samplerDesc)
	{
		U32 hashValue = ComputeStateDescHash(samplerDesc);
		if (mTextureSamplesHashTable.find(hashValue) != mTextureSamplesHashTable.cend())
		{
			return TOkValue<TTextureSamplerId>(mTextureSamplesHashTable[hashValue]);
		}

		ID3D11SamplerState* pNewTextureSampler = nullptr;

		D3D11_SAMPLER_DESC samplerDescInfo;

		memset(&samplerDescInfo, 0, sizeof(samplerDescInfo));

		samplerDescInfo.AddressU = CD3D11Mappings::GetTextureAddressMode(samplerDesc.mUAddressMode);
		samplerDescInfo.AddressV = CD3D11Mappings::GetTextureAddressMode(samplerDesc.mVAddressMode);
		samplerDescInfo.AddressW = CD3D11Mappings::GetTextureAddressMode(samplerDesc.mWAddressMode);
		samplerDescInfo.Filter   = CD3D11Mappings::GetFilterType(samplerDesc.mFilterFlags);
		samplerDescInfo.MaxLOD   = D3D11_FLOAT32_MAX;

		ID3D11Device* p3dDevice = nullptr;

#if _HAS_CXX17
		p3dDevice = std::get<TD3D11CtxInternalData>(mpGraphicsContext->GetInternalData()).mp3dDevice;
#else
		p3dDevice = mpGraphicsContext->GetInternalData().mD3D11.mp3dDevice;
#endif
		
		if (!p3dDevice)
		{
			return TErrorValue<E_RESULT_CODE>(RC_FAIL);
		}
		
		if (FAILED(p3dDevice->CreateSamplerState(&samplerDescInfo, &pNewTextureSampler)))
		{
			return TErrorValue<E_RESULT_CODE>(RC_FAIL);
		}

		U32 samplerId = mpTextureSamplersArray.size();

		mpTextureSamplersArray.push_back(pNewTextureSampler);
		mTextureSamplesHashTable.insert({ hashValue, samplerId });

		return TOkValue<TTextureSamplerId>(samplerId);
	}

	TResult<TBlendStateId> CD3D11GraphicsObjectManager::CreateBlendState(const TBlendStateDesc& blendStateDesc)
	{
		U32 hashValue = ComputeStateDescHash(blendStateDesc);
		if (mBlendStatesHashTable.find(hashValue) != mBlendStatesHashTable.cend())
		{
			return TOkValue<TBlendStateId>(mBlendStatesHashTable[hashValue]);
		}

		ID3D11Device* p3dDevice = nullptr;

#if _HAS_CXX17
		p3dDevice = std::get<TD3D11CtxInternalData>(mpGraphicsContext->GetInternalData()).mp3dDevice;
#else
		p3dDevice = mpGraphicsContext->GetInternalData().mD3D11.mp3dDevice;
#endif

		ID3D11BlendState* pBlendState = nullptr;

		D3D11_BLEND_DESC desc;
		desc.AlphaToCoverageEnable  = false;
		desc.IndependentBlendEnable = false;
		
		D3D11_RENDER_TARGET_BLEND_DESC& rtBlendStateDesc = desc.RenderTarget[0];
		rtBlendStateDesc.BlendEnable           = blendStateDesc.mIsEnabled;
		rtBlendStateDesc.SrcBlend              = CD3D11Mappings::GetBlendFactorValue(blendStateDesc.mScrValue);
		rtBlendStateDesc.DestBlend             = CD3D11Mappings::GetBlendFactorValue(blendStateDesc.mDestValue);
		rtBlendStateDesc.BlendOp               = CD3D11Mappings::GetBlendOpType(blendStateDesc.mOpType);
		rtBlendStateDesc.SrcBlendAlpha         = CD3D11Mappings::GetBlendFactorValue(blendStateDesc.mScrAlphaValue);
		rtBlendStateDesc.DestBlendAlpha        = CD3D11Mappings::GetBlendFactorValue(blendStateDesc.mDestAlphaValue);
		rtBlendStateDesc.BlendOpAlpha          = CD3D11Mappings::GetBlendOpType(blendStateDesc.mAlphaOpType);
		rtBlendStateDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		if (FAILED(p3dDevice->CreateBlendState(&desc, &pBlendState)))
		{
			return TErrorValue<E_RESULT_CODE>(RC_FAIL);
		}

		auto stateId = mpBlendStates.Add(pBlendState);
		mBlendStatesHashTable.insert({ hashValue, stateId });

		return TOkValue<TBlendStateId>(stateId);
	}

	TResult<TDepthStencilStateId> CD3D11GraphicsObjectManager::CreateDepthStencilState(const TDepthStencilStateDesc& depthStencilDesc)
	{
		ID3D11Device* p3dDevice = nullptr;

#if _HAS_CXX17
		p3dDevice = std::get<TD3D11CtxInternalData>(mpGraphicsContext->GetInternalData()).mp3dDevice;
#else
		p3dDevice = mpGraphicsContext->GetInternalData().mD3D11.mp3dDevice;
#endif

		ID3D11DepthStencilState* pDepthStencilState = nullptr;

		D3D11_DEPTH_STENCIL_DESC internalStateDesc;
		internalStateDesc.DepthEnable                  = depthStencilDesc.mIsDepthTestEnabled;
		internalStateDesc.DepthWriteMask               = depthStencilDesc.mIsDepthWritingEnabled ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
		internalStateDesc.DepthFunc                    = CD3D11Mappings::GetComparisonFunc(depthStencilDesc.mDepthCmpFunc);
		internalStateDesc.StencilEnable                = depthStencilDesc.mIsStencilTestEnabled;
		internalStateDesc.StencilReadMask              = depthStencilDesc.mStencilReadMaskValue;
		internalStateDesc.StencilWriteMask             = depthStencilDesc.mStencilWriteMaskValue;
		internalStateDesc.FrontFace.StencilFunc        = CD3D11Mappings::GetComparisonFunc(depthStencilDesc.mStencilFrontFaceOp.mFunc);
		internalStateDesc.FrontFace.StencilPassOp      = CD3D11Mappings::GetStencilOpType(depthStencilDesc.mStencilFrontFaceOp.mPassOp);
		internalStateDesc.FrontFace.StencilFailOp      = CD3D11Mappings::GetStencilOpType(depthStencilDesc.mStencilFrontFaceOp.mFailOp);
		internalStateDesc.FrontFace.StencilDepthFailOp = CD3D11Mappings::GetStencilOpType(depthStencilDesc.mStencilFrontFaceOp.mDepthFailOp);
		internalStateDesc.BackFace.StencilFunc         = CD3D11Mappings::GetComparisonFunc(depthStencilDesc.mStencilBackFaceOp.mFunc);
		internalStateDesc.BackFace.StencilPassOp       = CD3D11Mappings::GetStencilOpType(depthStencilDesc.mStencilBackFaceOp.mPassOp);
		internalStateDesc.BackFace.StencilFailOp       = CD3D11Mappings::GetStencilOpType(depthStencilDesc.mStencilBackFaceOp.mFailOp);
		internalStateDesc.BackFace.StencilDepthFailOp  = CD3D11Mappings::GetStencilOpType(depthStencilDesc.mStencilBackFaceOp.mDepthFailOp);

		if (FAILED(p3dDevice->CreateDepthStencilState(&internalStateDesc, &pDepthStencilState)))
		{
			return TErrorValue<E_RESULT_CODE>(RC_FAIL);
		}

		return TOkValue<TDepthStencilStateId>(mpDepthStencilStatesArray.Add(pDepthStencilState));
	}

	TResult<TRasterizerStateId> CD3D11GraphicsObjectManager::CreateRasterizerState(const TRasterizerStateDesc& rasterizerStateDesc)
	{
		ID3D11Device* p3dDevice = nullptr;

#if _HAS_CXX17
		p3dDevice = std::get<TD3D11CtxInternalData>(mpGraphicsContext->GetInternalData()).mp3dDevice;
#else
		p3dDevice = mpGraphicsContext->GetInternalData().mD3D11.mp3dDevice;
#endif

		ID3D11RasterizerState* pRasterizerState = nullptr;

		D3D11_RASTERIZER_DESC internalStateDesc;
		internalStateDesc.FillMode              = rasterizerStateDesc.mIsWireframeModeEnabled ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;
		internalStateDesc.CullMode              = CD3D11Mappings::GetCullMode(rasterizerStateDesc.mCullMode);
		internalStateDesc.DepthBias             = rasterizerStateDesc.mDepthBias;
		internalStateDesc.DepthBiasClamp        = rasterizerStateDesc.mMaxDepthBias;
		internalStateDesc.DepthClipEnable       = rasterizerStateDesc.mIsDepthClippingEnabled;
		internalStateDesc.FrontCounterClockwise = rasterizerStateDesc.mIsFrontCCWEnabled;
		internalStateDesc.ScissorEnable         = rasterizerStateDesc.mIsScissorTestEnabled;

		if (FAILED(p3dDevice->CreateRasterizerState(&internalStateDesc, &pRasterizerState)))
		{
			return TErrorValue<E_RESULT_CODE>(RC_FAIL);
		}

		return TOkValue<TRasterizerStateId>(mpRasterizerStatesArray.Add(pRasterizerState));
	}

	TResult<ID3D11SamplerState*> CD3D11GraphicsObjectManager::GetTextureSampler(TTextureSamplerId texSamplerId) const
	{
		if (texSamplerId >= mpTextureSamplersArray.size())
		{
			return TErrorValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}

		return TOkValue<ID3D11SamplerState*>(mpTextureSamplersArray[texSamplerId]);
	}

	TResult<ID3D11BlendState*> CD3D11GraphicsObjectManager::GetBlendState(TBlendStateId blendStateId) const
	{
		return mpBlendStates[blendStateId];
	}

	TResult<ID3D11DepthStencilState*> CD3D11GraphicsObjectManager::GetDepthStencilState(TDepthStencilStateId stateId) const
	{
		return mpDepthStencilStatesArray[stateId];
	}

	TResult<ID3D11RasterizerState*> CD3D11GraphicsObjectManager::GetRasterizerState(TRasterizerStateId rasterizerStateId) const
	{
		return mpRasterizerStatesArray[rasterizerStateId];
	}

	std::string CD3D11GraphicsObjectManager::GetDefaultShaderCode(const E_DEFAULT_SHADER_TYPE& type) const
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

			case E_DEFAULT_SHADER_TYPE::DST_EDITOR_UI:
				return R"(
					#define VERTEX_ENTRY mainVS
					#define PIXEL_ENTRY mainPS

					#include <TDEngine2Globals.inc>

					struct VertexOut
					{
						float4 mPos   : SV_POSITION;
						float2 mUV    : TEXCOORD0;
						float4 mColor : COLOR;
					};

					#program vertex

					struct VertexIn
					{
						float4 mPosUV : POSITION0;
						float4 mColor : COLOR;
					};

					VertexOut mainVS(VertexIn input)
					{
						VertexOut output;
						
						output.mPos = mul(ModelMat, float4(input.mPosUV.xy, 0.0, 1.0));
						output.mUV = input.mPosUV.zw;
						output.mColor = input.mColor;

						return output;
					}

					#endprogram

					#program pixel

					DECLARE_TEX2D(Texture);

					float4 mainPS(VertexOut input): SV_TARGET0
					{
						return GammaToLinear(input.mColor * TEX2D(Texture, input.mUV));
					}
					#endprogram
				)";
			case E_DEFAULT_SHADER_TYPE::DST_SCREEN_SPACE:
				return R"(
					#define VERTEX_ENTRY mainVS
					#define PIXEL_ENTRY mainPS

					#include <TDEngine2Globals.inc>

					struct VertexOut
					{
						float4 mPos   : SV_POSITION;
						float2 mUV    : TEXCOORD0;
					};

					#program vertex

					VertexOut mainVS(uint id : SV_VertexID)
					{
						VertexOut output;					
						
						float x = float((id & 1) << 2);
						float y = -float((id & 2) << 1);
						output.mPos = float4(x - 1, y + 1, 0, 1);
						output.mUV = float2(x, -y) * 0.5;

						return output;
					}

					#endprogram

					#program pixel

					DECLARE_TEX2D(FrameTexture);

					float4 mainPS(VertexOut input): SV_TARGET0
					{
						float4 color = TEX2D(FrameTexture, input.mUV);
						float3 mappedColor = 1 - exp(-color.rgb * 2.5f);
						return float4(LinearToGamma(mappedColor), color.a);
						//return float4(input.mUV.xy, 1, 1);
					}
					#endprogram
				)";
			case E_DEFAULT_SHADER_TYPE::DST_SELECTION:
				return R"(
					#define VERTEX_ENTRY mainVS
					#define PIXEL_ENTRY mainPS

					#include <TDEngine2Globals.inc>

					struct VertexOut
					{
						float4 mPos : SV_POSITION;
						uint mID  : COLOR0;
					};

					#program vertex

					VertexOut mainVS(float4 lPos : POSITION0)
					{
						VertexOut output;			

						output.mPos = mul(ProjMat, mul(ViewMat, mul(ModelMat, lPos)));
						output.mID  = ObjectID + 1;										

						return output;
					}

					#endprogram

					#program pixel

					uint mainPS(VertexOut input): SV_TARGET0
					{
						return input.mID;
					}
					#endprogram
				)";
			default:
				TDE2_UNIMPLEMENTED();
				break;
		}
		
		return "";
	}

	std::array<TVector4, 3> CD3D11GraphicsObjectManager::GetScreenTriangleVertices() const
	{
		// \todo D3D11 supports rendering without VB, so we don't use this vertices to draw triangle
		return { TVector4(0.0f), TVector4(0.0f), TVector4(0.0f) };
	}

	E_RESULT_CODE CD3D11GraphicsObjectManager::_freeTextureSamplers()
	{
		ID3D11SamplerState* pCurrSamplerState = nullptr;
		
		for (TTextureSamplersArray::iterator iter = mpTextureSamplersArray.begin(); iter != mpTextureSamplersArray.end(); ++iter)
		{
			pCurrSamplerState = (*iter);

			if (!pCurrSamplerState)
			{
				continue;
			}

			pCurrSamplerState->Release();
		}

		mpTextureSamplersArray.clear();

		return RC_OK;
	}

	E_RESULT_CODE CD3D11GraphicsObjectManager::_freeBlendStates()
	{
		ID3D11BlendState* pCurrBlendState = nullptr;

		for (U32 i = 0; i < mpBlendStates.GetSize(); ++i)
		{
			pCurrBlendState = mpBlendStates[i].GetOrDefault(nullptr);

			if (!pCurrBlendState)
			{
				continue;
			}

			pCurrBlendState->Release();
		}

		mpBlendStates.RemoveAll();

		return RC_OK;
	}

	E_RESULT_CODE CD3D11GraphicsObjectManager::_freeDepthStencilStates()
	{
		ID3D11DepthStencilState* pCurrState = nullptr;

		for (U32 i = 0; i < mpDepthStencilStatesArray.GetSize(); ++i)
		{
			pCurrState = mpDepthStencilStatesArray[i].GetOrDefault(nullptr);

			if (!pCurrState)
			{
				continue;
			}

			pCurrState->Release();
		}

		mpDepthStencilStatesArray.RemoveAll();

		return RC_OK;
	}

	E_RESULT_CODE CD3D11GraphicsObjectManager::_freeRasterizerStates()
	{
		ID3D11RasterizerState* pCurrState = nullptr;

		for (U32 i = 0; i < mpRasterizerStatesArray.GetSize(); ++i)
		{
			pCurrState = mpRasterizerStatesArray[i].GetOrDefault(nullptr);

			if (!pCurrState)
			{
				continue;
			}

			pCurrState->Release();
		}

		mpRasterizerStatesArray.RemoveAll();

		return RC_OK;
	}

	IGraphicsObjectManager* CreateD3D11GraphicsObjectManager(IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		CD3D11GraphicsObjectManager* pManagerInstance = new (std::nothrow) CD3D11GraphicsObjectManager();

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

#endif