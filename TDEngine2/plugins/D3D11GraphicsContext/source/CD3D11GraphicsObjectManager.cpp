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

	TResult<IVertexBuffer*> CD3D11GraphicsObjectManager::CreateVertexBuffer(E_BUFFER_USAGE_TYPE usageType, U32 totalBufferSize, void* pDataPtr)
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
																		  U32 totalBufferSize, void* pDataPtr) 
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

	TResult<IConstantBuffer*> CD3D11GraphicsObjectManager::CreateConstantBuffer(E_BUFFER_USAGE_TYPE usageType, U32 totalBufferSize, void* pDataPtr)
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

		return TOkValue<TTextureSamplerId>(samplerId);
	}

	TResult<TBlendStateId> CD3D11GraphicsObjectManager::CreateBlendState(const TBlendStateDesc& blendStateDesc)
	{
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

		return TOkValue<TBlendStateId>(mpBlendStates.Add(pBlendState));
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

	std::string CD3D11GraphicsObjectManager::GetDefaultShaderCode() const
	{
		return R"(
			#define VERTEX_ENTRY mainVS
			#define PIXEL_ENTRY mainPS

			#include "TDEngine2Globals.inc"

			#if VERTEX

			float4 mainVS(float4 lPos : POSITION0): SV_POSITION
			{
				return mul(ProjMat, mul(ViewMat, mul(ModelMat, lPos)));
			}

			#endif

			#if PIXEL

			float4 mainPS(float4 wPos : SV_POSITION): SV_TARGET0
			{
				return float4(1.0, 0.0, 1.0, 1.0);
			}
			#endif)";
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