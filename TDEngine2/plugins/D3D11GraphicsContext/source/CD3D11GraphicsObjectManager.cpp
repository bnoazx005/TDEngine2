#include "../include/CD3D11GraphicsObjectManager.h"
#include "../include/CD3D11Buffer.h"
#include "../include/CD3D11Texture.h"
#include "../include/CD3D11VertexDeclaration.h"
#include "../include/CD3D11Mappings.h"
#include "../include/CD3D11Utils.h"
#include <core/IGraphicsContext.h>
#include <core/IFileSystem.h>
#include <core/IFile.h>
#include <core/CProjectSettings.h>


#if defined (TDE2_USE_WINPLATFORM)

namespace TDEngine2
{
	TDE2_DEFINE_SCOPED_PTR(CD3D11Buffer)


	CD3D11GraphicsObjectManager::CD3D11GraphicsObjectManager():
		CBaseGraphicsObjectManager()
	{
	}

	TResult<TBufferHandleId> CD3D11GraphicsObjectManager::CreateBuffer(const TInitBufferParams& params)
	{
		E_RESULT_CODE result = RC_OK;

		TPtr<IBuffer> pBuffer = TPtr<IBuffer>(CreateD3D11Buffer(mpGraphicsContext, params, result));
		if (!pBuffer || RC_OK != result)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		auto it = std::find(mpBuffersArray.begin(), mpBuffersArray.end(), nullptr);
		const USIZE placementIndex = static_cast<USIZE>(std::distance(mpBuffersArray.begin(), it));

		if (placementIndex >= mpBuffersArray.size())
		{
			mpBuffersArray.emplace_back(DynamicPtrCast<CD3D11Buffer>(pBuffer));
		}
		else
		{
			mpBuffersArray[placementIndex] = DynamicPtrCast<CD3D11Buffer>(pBuffer);
		}

		return Wrench::TOkValue<TBufferHandleId>(static_cast<TBufferHandleId>(placementIndex));
	}

	TResult<TTextureHandleId> CD3D11GraphicsObjectManager::CreateTexture(const TInitTextureImplParams& params)
	{
		E_RESULT_CODE result = RC_OK;

		TPtr<ITextureImpl> pTexture = TPtr<ITextureImpl>(CreateD3D11TextureImpl(mpGraphicsContext, params, result));
		if (!pTexture || RC_OK != result)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		auto it = std::find(mpTexturesArray.begin(), mpTexturesArray.end(), nullptr);
		const USIZE placementIndex = static_cast<USIZE>(std::distance(mpTexturesArray.begin(), it));

		if (placementIndex >= mpTexturesArray.size())
		{
			mpTexturesArray.emplace_back(DynamicPtrCast<CD3D11TextureImpl>(pTexture));
		}
		else
		{
			mpTexturesArray[placementIndex] = DynamicPtrCast<CD3D11TextureImpl>(pTexture);
		}

		return Wrench::TOkValue<TTextureHandleId>(static_cast<TTextureHandleId>(placementIndex));
	}

	E_RESULT_CODE CD3D11GraphicsObjectManager::DestroyBuffer(TBufferHandleId bufferHandle)
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

	E_RESULT_CODE CD3D11GraphicsObjectManager::DestroyTexture(TTextureHandleId textureHandle)
	{
		if (TTextureHandleId::Invalid == textureHandle)
		{
			return RC_INVALID_ARGS;
		}

		const USIZE bufferPlacementIndex = static_cast<USIZE>(textureHandle);
		if (bufferPlacementIndex >= mpTexturesArray.size())
		{
			return RC_FAIL;
		}

		mpTexturesArray[bufferPlacementIndex] = nullptr;

		return RC_OK;
	}

	TResult<IVertexDeclaration*> CD3D11GraphicsObjectManager::CreateVertexDeclaration()
	{
		E_RESULT_CODE result = RC_OK;

		IVertexDeclaration* pNewVertDecl = CreateD3D11VertexDeclaration(result);

		if (result != RC_OK)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		_insertVertexDeclaration(pNewVertDecl);

		return Wrench::TOkValue<IVertexDeclaration*>(pNewVertDecl);
	}

	TResult<TTextureSamplerId> CD3D11GraphicsObjectManager::CreateTextureSampler(const TTextureSamplerDesc& samplerDesc)
	{
		U32 hashValue = ComputeStateDescHash(samplerDesc);
		if (mTextureSamplesHashTable.find(hashValue) != mTextureSamplesHashTable.cend())
		{
			return Wrench::TOkValue<TTextureSamplerId>(TTextureSamplerId(mTextureSamplesHashTable[hashValue]));
		}

		ID3D11SamplerState* pNewTextureSampler = nullptr;

		D3D11_SAMPLER_DESC samplerDescInfo;

		memset(&samplerDescInfo, 0, sizeof(samplerDescInfo));

		samplerDescInfo.AddressU = CD3D11Mappings::GetTextureAddressMode(samplerDesc.mUAddressMode);
		samplerDescInfo.AddressV = CD3D11Mappings::GetTextureAddressMode(samplerDesc.mVAddressMode);
		samplerDescInfo.AddressW = CD3D11Mappings::GetTextureAddressMode(samplerDesc.mWAddressMode);
		samplerDescInfo.Filter   = CD3D11Mappings::GetFilterType(samplerDesc.mFilteringType);
		samplerDescInfo.MaxLOD   = D3D11_FLOAT32_MAX;

		ID3D11Device* p3dDevice = nullptr;

#if _HAS_CXX17
		p3dDevice = std::get<TD3D11CtxInternalData>(mpGraphicsContext->GetInternalData()).mp3dDevice;
#else
		p3dDevice = mpGraphicsContext->GetInternalData().mD3D11.mp3dDevice;
#endif
		
		if (!p3dDevice)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}
		
		if (FAILED(p3dDevice->CreateSamplerState(&samplerDescInfo, &pNewTextureSampler)))
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		U32 samplerId = static_cast<U32>(mpTextureSamplersArray.size());

		mpTextureSamplersArray.push_back(pNewTextureSampler);
		mTextureSamplesHashTable.insert({ hashValue, samplerId });

		return Wrench::TOkValue<TTextureSamplerId>(TTextureSamplerId(samplerId));
	}

	TResult<TBlendStateId> CD3D11GraphicsObjectManager::CreateBlendState(const TBlendStateDesc& blendStateDesc)
	{
		U32 hashValue = ComputeStateDescHash(blendStateDesc);
		if (mBlendStatesHashTable.find(hashValue) != mBlendStatesHashTable.cend())
		{
			return Wrench::TOkValue<TBlendStateId>(TBlendStateId(mBlendStatesHashTable[hashValue]));
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
		rtBlendStateDesc.RenderTargetWriteMask = blendStateDesc.mWriteMask;

		if (FAILED(p3dDevice->CreateBlendState(&desc, &pBlendState)))
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		U32 stateId = static_cast<U32>(mpBlendStates.Add(pBlendState));
		mBlendStatesHashTable.insert({ hashValue, stateId });

		return Wrench::TOkValue<TBlendStateId>(TBlendStateId(stateId));
	}

	TResult<TDepthStencilStateId> CD3D11GraphicsObjectManager::CreateDepthStencilState(const TDepthStencilStateDesc& depthStencilDesc)
	{
		ID3D11Device* p3dDevice = nullptr;

#if _HAS_CXX17
		p3dDevice = std::get<TD3D11CtxInternalData>(mpGraphicsContext->GetInternalData()).mp3dDevice;
#else
		p3dDevice = mpGraphicsContext->GetInternalData().mD3D11.mp3dDevice;
#endif

		const U32 hash = ComputeStateDescHash(depthStencilDesc);
		
		auto it = mDepthStencilStatesTable.find(hash);
		if (it != mDepthStencilStatesTable.cend())
		{
			return Wrench::TOkValue<TDepthStencilStateId>(it->second);
		}

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
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		const TDepthStencilStateId handle = TDepthStencilStateId(mpDepthStencilStatesArray.Add(pDepthStencilState));

		mDepthStencilStatesTable.emplace(ComputeStateDescHash(depthStencilDesc), handle);

		return Wrench::TOkValue<TDepthStencilStateId>(handle);
	}

	TResult<TRasterizerStateId> CD3D11GraphicsObjectManager::CreateRasterizerState(const TRasterizerStateDesc& rasterizerStateDesc)
	{
		ID3D11Device* p3dDevice = nullptr;

#if _HAS_CXX17
		p3dDevice = std::get<TD3D11CtxInternalData>(mpGraphicsContext->GetInternalData()).mp3dDevice;
#else
		p3dDevice = mpGraphicsContext->GetInternalData().mD3D11.mp3dDevice;
#endif

		const U32 hashValue = ComputeStateDescHash(rasterizerStateDesc);

		auto it = mRasterizerStatesTable.find(hashValue);
		if (it != mRasterizerStatesTable.cend())
		{
			return Wrench::TOkValue<TRasterizerStateId>(it->second);
		}

		ID3D11RasterizerState* pRasterizerState = nullptr;

		D3D11_RASTERIZER_DESC internalStateDesc;
		internalStateDesc.FillMode              = rasterizerStateDesc.mIsWireframeModeEnabled ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;
		internalStateDesc.CullMode              = CD3D11Mappings::GetCullMode(rasterizerStateDesc.mCullMode);
		internalStateDesc.DepthBias             = static_cast<I32>(rasterizerStateDesc.mDepthBias);
		internalStateDesc.DepthBiasClamp        = rasterizerStateDesc.mMaxDepthBias;
		internalStateDesc.DepthClipEnable       = rasterizerStateDesc.mIsDepthClippingEnabled;
		internalStateDesc.FrontCounterClockwise = rasterizerStateDesc.mIsFrontCCWEnabled;
		internalStateDesc.ScissorEnable         = rasterizerStateDesc.mIsScissorTestEnabled;
		internalStateDesc.SlopeScaledDepthBias  = 0.0f;

		if (FAILED(p3dDevice->CreateRasterizerState(&internalStateDesc, &pRasterizerState)))
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		const TRasterizerStateId handle = TRasterizerStateId(mpRasterizerStatesArray.Add(pRasterizerState));
		mRasterizerStatesTable.emplace(ComputeStateDescHash(rasterizerStateDesc), handle);

		return Wrench::TOkValue<TRasterizerStateId>(handle);
	}

	TResult<ID3D11SamplerState*> CD3D11GraphicsObjectManager::GetTextureSampler(TTextureSamplerId texSamplerId) const
	{
		U32 texSamplerIdValue = static_cast<U32>(texSamplerId);

		if (texSamplerIdValue >= mpTextureSamplersArray.size())
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}

		return Wrench::TOkValue<ID3D11SamplerState*>(mpTextureSamplersArray[texSamplerIdValue]);
	}

	TResult<ID3D11BlendState*> CD3D11GraphicsObjectManager::GetBlendState(TBlendStateId blendStateId) const
	{
		return mpBlendStates[static_cast<U32>(blendStateId)];
	}

	TResult<ID3D11DepthStencilState*> CD3D11GraphicsObjectManager::GetDepthStencilState(TDepthStencilStateId stateId) const
	{
		return mpDepthStencilStatesArray[static_cast<U32>(stateId)];
	}

	TResult<ID3D11RasterizerState*> CD3D11GraphicsObjectManager::GetRasterizerState(TRasterizerStateId rasterizerStateId) const
	{
		return mpRasterizerStatesArray[static_cast<U32>(rasterizerStateId)];
	}

	TPtr<IBuffer> CD3D11GraphicsObjectManager::GetBufferPtr(TBufferHandleId handle)
	{
		return DynamicPtrCast<IBuffer>(GetD3D11BufferPtr(handle));
	}

	TPtr<CD3D11Buffer> CD3D11GraphicsObjectManager::GetD3D11BufferPtr(TBufferHandleId bufferHandle)
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

	TPtr<ITextureImpl> CD3D11GraphicsObjectManager::GetTexturePtr(TTextureHandleId handle)
	{
		return DynamicPtrCast<ITextureImpl>(GetD3D11TexturePtr(handle));
	}

	TPtr<CD3D11TextureImpl> CD3D11GraphicsObjectManager::GetD3D11TexturePtr(TTextureHandleId textureHandle)
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
		
		for (ID3D11SamplerState* pCurrSamplerState : mpTextureSamplersArray)
		{
			SafeReleaseCOMPtr<ID3D11SamplerState>(&pCurrSamplerState);
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
			SafeReleaseCOMPtr<ID3D11BlendState>(&pCurrBlendState);
		}

		mpBlendStates.RemoveAll();

		return RC_OK;
	}

	E_RESULT_CODE CD3D11GraphicsObjectManager::_freeDepthStencilStates()
	{
		ID3D11DepthStencilState* pCurrState = nullptr;

		for (USIZE i = 0; i < mpDepthStencilStatesArray.GetSize(); ++i)
		{
			pCurrState = mpDepthStencilStatesArray[i].GetOrDefault(nullptr);
			mpDepthStencilStatesArray.RemoveAt(i);

			SafeReleaseCOMPtr<ID3D11DepthStencilState>(&pCurrState, mpDepthStencilStatesArray.FindIndex(pCurrState, i + 1) == decltype(mpDepthStencilStatesArray)::mInvalidIndex);
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
			SafeReleaseCOMPtr<ID3D11RasterizerState>(&pCurrState);
		}

		mpRasterizerStatesArray.RemoveAll();

		return RC_OK;
	}

	const std::string CD3D11GraphicsObjectManager::_getShaderCacheFilePath() const
	{
		return Wrench::StringUtils::Format(CProjectSettings::Get()->mGraphicsSettings.mShaderCachePathPattern, "D3D11");
	}

	IGraphicsObjectManager* CreateD3D11GraphicsObjectManager(IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IGraphicsObjectManager, CD3D11GraphicsObjectManager, result, pGraphicsContext);
	}
}

#endif