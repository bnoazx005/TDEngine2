#include "../../include/core/CProxyGraphicsContext.h"
#include "../../include/utils/CFileLogger.h"
#include "stringUtils.hpp"


namespace TDEngine2
{
	CProxyGraphicsContext::CProxyGraphicsContext() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CProxyGraphicsContext::Init(TPtr<IWindowSystem> pWindowSystem)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pWindowSystem)
		{
			return RC_INVALID_ARGS;
		}

		mpWindowSystem = pWindowSystem;

		mIsInitialized = true;

		return RC_OK;
	}

	void CProxyGraphicsContext::ClearBackBuffer(const TColor32F& color)
	{
		LOG_MESSAGE("[ProxyGraphicsContext] ClearBackBuffer");
	}

	void CProxyGraphicsContext::ClearRenderTarget(IRenderTarget* pRenderTarget, const TColor32F& color)
	{
		if (!pRenderTarget)
		{
			return;
		}

		LOG_MESSAGE("[ProxyGraphicsContext] ClearRenderTarget(IRenderTarget*, const TColor32F&)");
	}

	void CProxyGraphicsContext::ClearRenderTarget(U8 slot, const TColor32F& color)
	{
		LOG_MESSAGE("[ProxyGraphicsContext] ClearRenderTarget(U8, const TColor32F&)");
	}

	void CProxyGraphicsContext::ClearDepthBufferTarget(IDepthBufferTarget* pDepthBufferTarget, F32 value, U8 stencilValue)
	{
		LOG_MESSAGE("[ProxyGraphicsContext] ClearDepthBufferTarget(IDepthBufferTarget, F32, U8)");
	}

	void CProxyGraphicsContext::ClearDepthBuffer(F32 value)
	{
		LOG_MESSAGE("[ProxyGraphicsContext] ClearDepthBuffer(F32)");
	}

	void CProxyGraphicsContext::ClearStencilBuffer(U8 value)
	{
		LOG_MESSAGE("[ProxyGraphicsContext] ClearStencilBuffer(U8)");
	}

	void CProxyGraphicsContext::BeginFrame()
	{
		LOG_MESSAGE("[ProxyGraphicsContext] BeginFrame");
	}

	void CProxyGraphicsContext::Present()
	{
		LOG_MESSAGE("[ProxyGraphicsContext] Present()");
	}

	void CProxyGraphicsContext::SetViewport(F32 x, F32 y, F32 width, F32 height, F32 minDepth, F32 maxDepth)
	{
		LOG_MESSAGE("[ProxyGraphicsContext] SetViewport(F32, F32, F32, F32, F32, F32)");
	}

	void CProxyGraphicsContext::SetScissorRect(const TRectU32& scissorRect)
	{
		LOG_MESSAGE("[ProxyGraphicsContext] SetScissorTest(const TRectU32)");
	}

	TMatrix4 CProxyGraphicsContext::CalcPerspectiveMatrix(F32 fov, F32 aspect, F32 zn, F32 zf)
	{
		return PerspectiveProj(fov, aspect, zn, zf, 0.0f, 1.0f, -1.0f);
	}

	TMatrix4 CProxyGraphicsContext::CalcOrthographicMatrix(F32 left, F32 top, F32 right, F32 bottom, F32 zn, F32 zf, bool isDepthless)
	{
		return OrthographicProj(left, top, right, bottom, zn, zf, 0.0f, 1.0f, -1.0f, isDepthless);
	}

	E_RESULT_CODE CProxyGraphicsContext::SetVertexBuffer(U32 slot, TBufferHandleId vertexBufferHandle, U32 offset, U32 strideSize)
	{
		LOG_MESSAGE("[ProxyGraphicsContext] SetVertexBuffer");
		return RC_OK;
	}

	E_RESULT_CODE CProxyGraphicsContext::SetIndexBuffer(TBufferHandleId indexBufferHandle, U32 offset)
	{
		LOG_MESSAGE("[ProxyGraphicsContext] SetIndexBuffer");
		return RC_OK;
	}

	E_RESULT_CODE CProxyGraphicsContext::SetConstantBuffer(U32 slot, TBufferHandleId constantsBufferHandle)
	{
		LOG_MESSAGE("[ProxyGraphicsContext] SetConstantBuffer");
		return RC_OK;
	}

	E_RESULT_CODE CProxyGraphicsContext::SetStructuredBuffer(U32 slot, TBufferHandleId bufferHandle, bool isWriteEnabled)
	{
		LOG_MESSAGE("[ProxyGraphicsContext] SetStructuredBuffer");
		return RC_OK;
	}

	E_RESULT_CODE CProxyGraphicsContext::SetTexture(U32 slot, TTextureHandleId textureHandle, bool isWriteEnabled)
	{
		LOG_MESSAGE("[ProxyGraphicsContext] SetTexture");
		return RC_OK;
	}

	E_RESULT_CODE CProxyGraphicsContext::SetSampler(U32 slot, TTextureSamplerId samplerHandle)
	{
		LOG_MESSAGE("[ProxyGraphicsContext] SetSampler");
		return RC_OK;
	}

	E_RESULT_CODE CProxyGraphicsContext::UpdateTexture2D(TTextureHandleId textureHandle, U32 mipLevel, const TRectI32& regionRect, const void* pData, USIZE dataSize)
	{
		LOG_MESSAGE("[ProxyGraphicsContext] UpdateTexture2D");
		return RC_OK;
	}

	E_RESULT_CODE CProxyGraphicsContext::UpdateTexture2DArray(TTextureHandleId textureHandle, U32 index, const TRectI32& regionRect, const void* pData, USIZE dataSize)
	{
		LOG_MESSAGE("[ProxyGraphicsContext] UpdateTexture2DArray");
		return RC_OK;
	}

	E_RESULT_CODE CProxyGraphicsContext::UpdateCubemapTexture(TTextureHandleId textureHandle, E_CUBEMAP_FACE face, const TRectI32& regionRect, const void* pData, USIZE dataSize)
	{
		LOG_MESSAGE("[ProxyGraphicsContext] UpdateCubemapTexture");
		return RC_OK;
	}

	E_RESULT_CODE CProxyGraphicsContext::UpdateTexture3D(TTextureHandleId textureHandle, U32 depthFrom, U32 depthTo, const TRectI32& regionRect, const void* pData, USIZE dataSize)
	{
		LOG_MESSAGE("[ProxyGraphicsContext] UpdateTexture3D");
		return RC_OK;
	}

	E_RESULT_CODE CProxyGraphicsContext::CopyResource(TTextureHandleId sourceHandle, TTextureHandleId destHandle)
	{
		LOG_MESSAGE("[ProxyGraphicsContext] CopyResource");
		return RC_OK;
	}

	E_RESULT_CODE CProxyGraphicsContext::CopyResource(TBufferHandleId sourceHandle, TTextureHandleId destHandle)
	{
		LOG_MESSAGE("[ProxyGraphicsContext] CopyResource");
		return RC_OK;
	}

	E_RESULT_CODE CProxyGraphicsContext::CopyResource(TTextureHandleId sourceHandle, TBufferHandleId destHandle)
	{
		LOG_MESSAGE("[ProxyGraphicsContext] CopyResource");
		return RC_OK;
	}

	E_RESULT_CODE CProxyGraphicsContext::CopyResource(TBufferHandleId sourceHandle, TBufferHandleId destHandle)
	{
		LOG_MESSAGE("[ProxyGraphicsContext] CopyResource");
		return RC_OK;
	}

	void CProxyGraphicsContext::Draw(E_PRIMITIVE_TOPOLOGY_TYPE topology, U32 startVertex, U32 numOfVertices)
	{
		LOG_MESSAGE("[ProxyGraphicsContext] Draw(E_PRIMITIVE_TOPOLOGY_TYPE, U32, U32)");
	}

	void CProxyGraphicsContext::DrawIndexed(E_PRIMITIVE_TOPOLOGY_TYPE topology, E_INDEX_FORMAT_TYPE indexFormatType, U32 baseVertex, U32 startIndex, U32 numOfIndices)
	{
		LOG_MESSAGE("[ProxyGraphicsContext] DrawIndexed(E_PRIMITIVE_TOPOLOGY_TYPE, E_INDEX_FORMAT_TYPE, U32, U32, U32)");
	}

	void CProxyGraphicsContext::DrawInstanced(E_PRIMITIVE_TOPOLOGY_TYPE topology, U32 startVertex, U32 verticesPerInstance, U32 startInstance, U32 numOfInstances)
	{
		LOG_MESSAGE("[ProxyGraphicsContext] DrawIndexed(E_PRIMITIVE_TOPOLOGY_TYPE, U32, U32, U32, U32)");
	}

	void CProxyGraphicsContext::DrawIndexedInstanced(E_PRIMITIVE_TOPOLOGY_TYPE topology, E_INDEX_FORMAT_TYPE indexFormatType, U32 baseVertex, U32 startIndex,
		U32 startInstance, U32 indicesPerInstance, U32 numOfInstances)
	{
		LOG_MESSAGE("[ProxyGraphicsContext] DrawIndexedInstanced(E_PRIMITIVE_TOPOLOGY_TYPE, E_INDEX_FORMAT_TYPE, U32, U32, U32, U32, U32)");
	}

	void CProxyGraphicsContext::DispatchCompute(U32 groupsCountX, U32 groupsCountY, U32 groupsCountZ)
	{
		LOG_MESSAGE("[ProxyGraphicsContext] DispatchCompute(U32, U32, U32)");
	}

	void CProxyGraphicsContext::BindBlendState(TBlendStateId blendStateId)
	{
		LOG_MESSAGE("[ProxyGraphicsContext] BindBlendState(TBlendStateId)");
	}

	void CProxyGraphicsContext::BindDepthStencilState(TDepthStencilStateId depthStencilStateId, U8 stencilRef)
	{
		LOG_MESSAGE("[ProxyGraphicsContext] BindDepthStencilState(TDepthStencilStateId)");
	}

	void CProxyGraphicsContext::BindRasterizerState(TRasterizerStateId rasterizerStateId)
	{
		LOG_MESSAGE("[ProxyGraphicsContext] BindRasterizerState(TRasterizerStateId)");
	}

	void CProxyGraphicsContext::BindRenderTarget(U8 slot, IRenderTarget* pRenderTarget)
	{
		LOG_MESSAGE("[ProxyGraphicsContext] BindRenderTarget(U8, IRenderTarget*)");
	}

	void CProxyGraphicsContext::BindDepthBufferTarget(IDepthBufferTarget* pDepthBufferTarget, bool disableRTWrite)
	{
		LOG_MESSAGE("[ProxyGraphicsContext] BindDepthBufferTarget(IDepthBufferTarget*, bool)");
	}

	void CProxyGraphicsContext::SetDepthBufferEnabled(bool value)
	{
		LOG_MESSAGE("[ProxyGraphicsContext] SetDepthBufferEnabled(bool)");
	}

#if TDE2_DEBUG_MODE

	void CProxyGraphicsContext::BeginSectionMarker(const std::string& id)
	{
		LOG_MESSAGE("[ProxyGraphicsContext] BeginSectionMarker(std::string)");
	}
	
	void CProxyGraphicsContext::EndSectionMarker()
	{
		LOG_MESSAGE("[ProxyGraphicsContext] EndSectionMarker()");
	}

#endif

	const TGraphicsCtxInternalData& CProxyGraphicsContext::GetInternalData() const
	{
		static TGraphicsCtxInternalData data;
		return data;
	}

	E_ENGINE_SUBSYSTEM_TYPE CProxyGraphicsContext::GetType() const
	{
		return EST_GRAPHICS_CONTEXT;
	}

	IGraphicsObjectManager* CProxyGraphicsContext::GetGraphicsObjectManager() const
	{
		return nullptr;
	}

	F32 CProxyGraphicsContext::GetPositiveZAxisDirection() const
	{
		return 1.0f;
	}

	TVideoAdapterInfo CProxyGraphicsContext::GetInfo() const
	{
		static TVideoAdapterInfo outputInfo;
		outputInfo.mAvailableVideoMemory = 0;

		return outputInfo;
	}

	const TGraphicsContextInfo& CProxyGraphicsContext::GetContextInfo() const
	{
		const static TGraphicsContextInfo infoData
		{
			{ ZeroVector3, ZeroVector3 },
			false,
			E_GRAPHICS_CONTEXT_GAPI_TYPE::GCGT_UNKNOWN
		};

		return infoData;
	}

	TPtr<IWindowSystem> CProxyGraphicsContext::GetWindowSystem() const
	{
		return mpWindowSystem;
	}

	std::vector<U8> CProxyGraphicsContext::GetBackBufferData() const
	{
		std::vector<U8> backBufferData;
		return std::move(backBufferData);
	}


	TDE2_API IGraphicsContext* CreateProxyGraphicsContext(TPtr<IWindowSystem> pWindowSystem, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IGraphicsContext, CProxyGraphicsContext, result, pWindowSystem);
	}
}