#include "./../include/COGLGraphicsContext.h"
#include "./../include/IOGLContextFactory.h"
#include "./../include/COGLGraphicsObjectManager.h"
#include "./../include/COGLMappings.h"
#include "./../include/COGLRenderTarget.h"
#include "./../include/COGLUtils.h"
#include <core/IEventManager.h>
#include <core/IWindowSystem.h>
#include <string>


#if defined (TDE2_USE_WIN32PLATFORM)
	#include <GL/glew.h>
	#pragma comment(lib, "glew32.lib")
#elif defined (TDE2_USE_UNIXPLATFORM)
	#include <GL/glxew.h>
#else
#endif


namespace TDEngine2
{
	COGLGraphicsContext::COGLGraphicsContext(TCreateGLContextFactoryCallback glContextFactoryCallback):
		mIsInitialized(false), mGLContextFactoryCallback(glContextFactoryCallback)
	{
	}

	COGLGraphicsContext::~COGLGraphicsContext()
	{
	}

	E_RESULT_CODE COGLGraphicsContext::Init(IWindowSystem* pWindowSystem)
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

		if (!mGLContextFactoryCallback)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		/// creating GL context
		mpGLContextFactory = mGLContextFactoryCallback(pWindowSystem, result);

		///< Should be called after initialization of GL context factory, because the latter
		/// can initialize a window object (i.e this feature is used on UNIX platform)
		mWindowInternalData = pWindowSystem->GetInternalData();

		if (result != RC_OK)
		{
			return result;
		}

		result = mpGLContextFactory->SetContext();

		if (result != RC_OK)
		{
			return result;
		}

		/// initialization of GLEW
		glewExperimental = true;

		GLenum err = glewInit();
		
		if (err != GLEW_OK)
		{
			return RC_FAIL;
		}

		mpGraphicsObjectManager = CreateOGLGraphicsObjectManager(this, result);

		if (result != RC_OK)
		{
			return result;
		}

		U32 creationFlags = pWindowSystem->GetFlags();

		/// Enable a depth buffer if it's needed
		if (creationFlags & P_ZBUFFER_ENABLED)
		{
			GL_SAFE_CALL(glEnable(GL_DEPTH_TEST));
			GL_SAFE_CALL(glDepthFunc(GL_LESS));
		}

		mpEventManager = pWindowSystem->GetEventManager();

		if (!mpEventManager)
		{
			return RC_FAIL;
		}

		mpEventManager->Subscribe(TOnWindowResized::GetTypeId(), this);

		if ((result = _initFBO(pWindowSystem)) != RC_OK)
		{
			return result;
		}

		SetViewport(0, 0, static_cast<F32>(pWindowSystem->GetWidth()), static_cast<F32>(pWindowSystem->GetHeight()), 0.0f, 1.0f);

		mIsInitialized = true;
		
		return RC_OK;
	}

	E_RESULT_CODE COGLGraphicsContext::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		GL_SAFE_CALL(glDeleteRenderbuffers(1, &mMainDepthStencilRenderbuffer));
		GL_SAFE_CALL(glDeleteFramebuffers(1, &mMainFBOHandler));

		E_RESULT_CODE result = mpGraphicsObjectManager->Free();
		result = result | mpGLContextFactory->Free();

		mIsInitialized = false;

		delete this;

		return result;
	}
	
	void COGLGraphicsContext::ClearBackBuffer(const TColor32F& color)
	{
		GL_SAFE_VOID_CALL(glClearColor(color.r, color.g, color.b, color.a));
		GL_SAFE_VOID_CALL(glClear(GL_COLOR_BUFFER_BIT));
	}

	void COGLGraphicsContext::ClearRenderTarget(IRenderTarget* pRenderTarget, const TColor32F& color)
	{
		if (!pRenderTarget)
		{
			LOG_WARNING("[COGLGraphicsContext] Try to clear the invalid render target");
			return;
		}

		U8 renderTargetBoundSlot = (std::numeric_limits<U8>::max)();

		for (U8 currSlotIndex = 0; currSlotIndex < mMaxNumOfRenderTargets; ++currSlotIndex)
		{
			if (mpRenderTargets[currSlotIndex] == pRenderTarget)
			{
				renderTargetBoundSlot = currSlotIndex;
				break;
			}
		}

		// \note The render target isn't bound to the main FBO
		if (renderTargetBoundSlot >= mMaxNumOfRenderTargets)
		{
			return;
		}

		ClearRenderTarget(renderTargetBoundSlot, color);
	}
	
	void COGLGraphicsContext::ClearRenderTarget(U8 slot, const TColor32F& color)
	{
		TDE2_ASSERT(slot < mMaxNumOfRenderTargets);
		if (slot >= mMaxNumOfRenderTargets)
		{
			return;
		}

		if (!mpRenderTargets[slot])
		{
			LOG_WARNING(Wrench::StringUtils::Format("[COGLGraphicsContext] Try to clear the render target in slot {0}, but it's empty", slot));
			return;
		}

		const F32 clearColorArray[4]{ color.r, color.g, color.b, color.a };
		//GL_SAFE_VOID_CALL(glClearBufferfv(GL_COLOR_ATTACHMENT0 + slot, 0, clearColorArray));
	}

	void COGLGraphicsContext::ClearDepthBufferTarget(IDepthBufferTarget* pDepthBufferTarget, F32 value, U8 stencilValue)
	{
		TDE2_UNIMPLEMENTED();
	}

	void COGLGraphicsContext::ClearDepthBuffer(F32 value)
	{
		GL_SAFE_VOID_CALL(glClearDepthf(value));
		GL_SAFE_VOID_CALL(glClear(GL_DEPTH_BUFFER_BIT));
	}
	
	void COGLGraphicsContext::ClearStencilBuffer(U8 value)
	{
		GL_SAFE_VOID_CALL(glClearStencil(value));
		GL_SAFE_VOID_CALL(glClear(GL_STENCIL_BUFFER_BIT));
	}

	void COGLGraphicsContext::Present()
	{
#if defined (TDE2_USE_WIN32PLATFORM)
		SwapBuffers(mWindowInternalData.mDeviceContextHandler);
#elif defined (TDE2_USE_UNIXPLATFORM)
		glXSwapBuffers(mWindowInternalData.mpDisplayHandler, mWindowInternalData.mWindowHandler);
#else
#endif
	}

	void COGLGraphicsContext::SetViewport(F32 x, F32 y, F32 width, F32 height, F32 minDepth, F32 maxDepth)
	{
		GL_SAFE_VOID_CALL(glViewport(static_cast<GLint>(x), static_cast<GLint>(y), static_cast<GLsizei>(width), static_cast<GLsizei>(height)));
	}
	
	void COGLGraphicsContext::SetScissorRect(const TRectF32& scissorRect)
	{
		GL_SAFE_VOID_CALL(glScissor(static_cast<GLint>(scissorRect.x), static_cast<GLint>(scissorRect.y), static_cast<GLsizei>(scissorRect.width), static_cast<GLsizei>(scissorRect.height)));
	}

	TMatrix4 COGLGraphicsContext::CalcPerspectiveMatrix(F32 fov, F32 aspect, F32 zn, F32 zf)
	{
		return PerspectiveProj(fov, aspect, zn, zf, -1.0f, 1.0f, 1.0f);
	}

	TMatrix4 COGLGraphicsContext::CalcOrthographicMatrix(F32 left, F32 top, F32 right, F32 bottom, F32 zn, F32 zf, bool isDepthless)
	{
		return OrthographicProj(left, top, right, bottom, zn, zf, -1.0f, 1.0f, 1.0f, isDepthless);
	}

	void COGLGraphicsContext::Draw(E_PRIMITIVE_TOPOLOGY_TYPE topology, U32 startVertex, U32 numOfVertices)
	{
		GL_SAFE_VOID_CALL(glDrawArrays(COGLMappings::GetPrimitiveTopology(topology), startVertex, numOfVertices));
	}

	void COGLGraphicsContext::DrawIndexed(E_PRIMITIVE_TOPOLOGY_TYPE topology, E_INDEX_FORMAT_TYPE indexFormatType, U32 baseVertex, U32 startIndex, U32 numOfIndices)
	{
		intptr_t indexOffset = startIndex * ((indexFormatType == E_INDEX_FORMAT_TYPE::IFT_INDEX16) ? 2 : 4);
		GL_SAFE_VOID_CALL(glDrawElementsBaseVertex(COGLMappings::GetPrimitiveTopology(topology), numOfIndices, COGLMappings::GetIndexFormat(indexFormatType), reinterpret_cast<void*>(indexOffset), baseVertex));
	}

	void COGLGraphicsContext::DrawInstanced(E_PRIMITIVE_TOPOLOGY_TYPE topology, U32 startVertex, U32 verticesPerInstance, U32 startInstance, U32 numOfInstances)
	{
		GL_SAFE_VOID_CALL(glDrawArraysInstanced(COGLMappings::GetPrimitiveTopology(topology), startVertex, verticesPerInstance, numOfInstances));
	}

	void COGLGraphicsContext::DrawIndexedInstanced(E_PRIMITIVE_TOPOLOGY_TYPE topology, E_INDEX_FORMAT_TYPE indexFormatType, U32 baseVertex, U32 startIndex,
												   U32 startInstance, U32 indicesPerInstance, U32 numOfInstances)
	{
		GL_SAFE_VOID_CALL(glDrawElementsInstanced(COGLMappings::GetPrimitiveTopology(topology), indicesPerInstance, COGLMappings::GetIndexFormat(indexFormatType), 0, numOfInstances));
	}

	void COGLGraphicsContext::BindTextureSampler(U32 slot, TTextureSamplerId samplerId)
	{
		if (samplerId == TTextureSamplerId::Invalid)
		{
			GL_SAFE_VOID_CALL(glBindSampler(slot, 0));
			return;
		}

		GLuint internalSamplerId = dynamic_cast<COGLGraphicsObjectManager*>(mpGraphicsObjectManager)->GetTextureSampler(samplerId).Get();
		GL_SAFE_VOID_CALL(glBindSampler(slot, internalSamplerId));
	}

	void COGLGraphicsContext::BindBlendState(TBlendStateId blendStateId)
	{
		if (blendStateId == TBlendStateId::Invalid)
		{
			GL_SAFE_VOID_CALL(glDisable(GL_BLEND));
			return;
		}

		const TBlendStateDesc& blendStateDesc = dynamic_cast<COGLGraphicsObjectManager*>(mpGraphicsObjectManager)->GetBlendState(blendStateId).Get();

		auto stateFunction = blendStateDesc.mIsEnabled ? glEnable : glDisable;
		GL_SAFE_VOID_CALL(stateFunction(GL_BLEND));

		GL_SAFE_VOID_CALL(glBlendFuncSeparate(COGLMappings::GetBlendFactorValue(blendStateDesc.mScrValue),
											  COGLMappings::GetBlendFactorValue(blendStateDesc.mDestValue),
											  COGLMappings::GetBlendFactorValue(blendStateDesc.mScrAlphaValue),
											  COGLMappings::GetBlendFactorValue(blendStateDesc.mDestAlphaValue)));

		GL_SAFE_VOID_CALL(glBlendEquationSeparate(COGLMappings::GetBlendOpType(blendStateDesc.mOpType),
								COGLMappings::GetBlendOpType(blendStateDesc.mAlphaOpType)));
	}

	void COGLGraphicsContext::BindDepthStencilState(TDepthStencilStateId depthStencilStateId)
	{
		if (depthStencilStateId == TDepthStencilStateId::Invalid)
		{
			// \note set up default values for depth-stencil state
			GL_SAFE_VOID_CALL(glEnable(GL_DEPTH_TEST));
			GL_SAFE_VOID_CALL(glDepthMask(GL_TRUE));
			GL_SAFE_VOID_CALL(glDepthFunc(GL_LESS));

			// \note default settings for stencil test
			GL_SAFE_VOID_CALL(glDisable(GL_STENCIL_TEST));
			GL_SAFE_VOID_CALL(glStencilMask(0x0));

			return;
		}

		const TDepthStencilStateDesc& depthStencilState = dynamic_cast<COGLGraphicsObjectManager*>(mpGraphicsObjectManager)->GetDepthStencilState(depthStencilStateId).Get();

		auto stateActivationFunction = depthStencilState.mIsDepthTestEnabled ? glEnable : glDisable;

		GL_SAFE_VOID_CALL(stateActivationFunction(GL_DEPTH_TEST));
		GL_SAFE_VOID_CALL(glDepthMask(depthStencilState.mIsDepthWritingEnabled));
		GL_SAFE_VOID_CALL(glDepthFunc(COGLMappings::GetComparisonFunc(depthStencilState.mDepthCmpFunc)));

		// \note stencil's parameters
		stateActivationFunction = depthStencilState.mIsStencilTestEnabled ? glEnable : glDisable;

		GL_SAFE_VOID_CALL(stateActivationFunction(GL_STENCIL_TEST));
		GL_SAFE_VOID_CALL(glStencilMask(depthStencilState.mStencilWriteMaskValue));
		GL_SAFE_VOID_CALL(glStencilOpSeparate(GL_FRONT, COGLMappings::GetStencilOpType(depthStencilState.mStencilFrontFaceOp.mFailOp),
							COGLMappings::GetStencilOpType(depthStencilState.mStencilFrontFaceOp.mDepthFailOp),
							COGLMappings::GetStencilOpType(depthStencilState.mStencilFrontFaceOp.mPassOp)));
		GL_SAFE_VOID_CALL(glStencilOpSeparate(GL_BACK, COGLMappings::GetStencilOpType(depthStencilState.mStencilBackFaceOp.mFailOp),
							COGLMappings::GetStencilOpType(depthStencilState.mStencilBackFaceOp.mDepthFailOp),
							COGLMappings::GetStencilOpType(depthStencilState.mStencilBackFaceOp.mPassOp)));
	}

	void COGLGraphicsContext::BindRasterizerState(TRasterizerStateId rasterizerStateId)
	{
		if (rasterizerStateId == TRasterizerStateId::Invalid)
		{
			GL_SAFE_VOID_CALL(glEnable(GL_CULL_FACE));

			return;
		}

		const TRasterizerStateDesc& stateDesc = dynamic_cast<COGLGraphicsObjectManager*>(mpGraphicsObjectManager)->GetRasterizerState(rasterizerStateId).Get();

		auto stateActivationFunction = stateDesc.mCullMode != E_CULL_MODE::NONE ? glEnable : glDisable;
		
		// \note Culling
		GL_SAFE_VOID_CALL(stateActivationFunction(GL_CULL_FACE));
		if (stateDesc.mCullMode != E_CULL_MODE::NONE)
		{
			GL_SAFE_VOID_CALL(glCullFace(COGLMappings::GetCullMode(stateDesc.mCullMode)));
		}
		GL_SAFE_VOID_CALL(glFrontFace(stateDesc.mIsFrontCCWEnabled ? GL_CCW : GL_CW));
		
		// \note Scissor testing
		stateActivationFunction = stateDesc.mIsScissorTestEnabled ? glEnable : glDisable;
		GL_SAFE_VOID_CALL(stateActivationFunction(GL_SCISSOR_TEST));

		GL_SAFE_VOID_CALL(glPolygonMode(GL_FRONT_AND_BACK, stateDesc.mIsWireframeModeEnabled ? GL_LINE : GL_FILL));

		// \todo Implement support of depth bias and clipping
	}

	void COGLGraphicsContext::BindRenderTarget(U8 slot, IRenderTarget* pRenderTarget)
	{
		TDE2_ASSERT(slot < mMaxNumOfRenderTargets);
		if (slot >= mMaxNumOfRenderTargets)
		{
			LOG_WARNING("[CD3D11GraphicsContext] Render target's slot goes out of limits");
			return;
		}

		if (!slot && !pRenderTarget)
		{
			GL_SAFE_VOID_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
			return;
		}

		mpRenderTargets[slot] = pRenderTarget;
		
		COGLRenderTarget* pOGLRenderTarget = dynamic_cast<COGLRenderTarget*>(pRenderTarget);

#if 1
		GL_SAFE_VOID_CALL(glBindFramebuffer(GL_FRAMEBUFFER, mMainFBOHandler));
		GL_SAFE_VOID_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + slot, GL_TEXTURE_2D, pOGLRenderTarget->GetInternalHandler(), 0));
#endif
	}

	void COGLGraphicsContext::BindDepthBufferTarget(IDepthBufferTarget* pDepthBufferTarget, bool disableRTWrite)
	{
		TDE2_UNIMPLEMENTED();
	}

	void COGLGraphicsContext::SetDepthBufferEnabled(bool value)
	{
		TDE2_UNIMPLEMENTED();
	}

	const TGraphicsCtxInternalData& COGLGraphicsContext::GetInternalData() const
	{
		return mInternalDataObject;
	}

	E_ENGINE_SUBSYSTEM_TYPE COGLGraphicsContext::GetType() const
	{
		return EST_GRAPHICS_CONTEXT;
	}
	
	IGraphicsObjectManager* COGLGraphicsContext::GetGraphicsObjectManager() const
	{
		return mpGraphicsObjectManager;
	}

	IWindowSystem* COGLGraphicsContext::GetWindowSystem() const
	{
		return mpWindowSystem;
	}

	F32 COGLGraphicsContext::GetPositiveZAxisDirection() const
	{
		return -1.0f;
	}

	E_RESULT_CODE COGLGraphicsContext::OnEvent(const TBaseEvent* pEvent)
	{
		if (pEvent->GetEventType() != TOnWindowResized::GetTypeId())
		{
			return RC_OK;
		}

		const TOnWindowResized* pOnWindowResizedEvent = dynamic_cast<const TOnWindowResized*>(pEvent);

		SetViewport(0.0f, 0.0f, static_cast<F32>(pOnWindowResizedEvent->mWidth), static_cast<F32>(pOnWindowResizedEvent->mHeight), 0.0f, 1.0f);

		return RC_OK;
	}

	TEventListenerId COGLGraphicsContext::GetListenerId() const
	{
		return TEventListenerId(GetTypeId());
	}

	TVideoAdapterInfo COGLGraphicsContext::GetInfo() const
	{
		std::string vendorName { reinterpret_cast<const C8*>(glGetString(GL_VENDOR)) };

		TVideoAdapterInfo::E_VENDOR_TYPE vendorType = TVideoAdapterInfo::E_VENDOR_TYPE::UNKNOWN;

		if (Wrench::StringUtils::StartsWith(vendorName, "NVIDIA"))
		{
			vendorType = TVideoAdapterInfo::E_VENDOR_TYPE::NVIDIA;
		}
		else if (Wrench::StringUtils::StartsWith(vendorName, "ATI"))
		{
			vendorType = TVideoAdapterInfo::E_VENDOR_TYPE::AMD;
		}
		else if (Wrench::StringUtils::StartsWith(vendorName, "INTEL"))
		{
			vendorType = TVideoAdapterInfo::E_VENDOR_TYPE::INTEL;
		}

		TVideoAdapterInfo outputInfo;
		//outputInfo.mAvailableVideoMemory = adapterInfo.DedicatedVideoMemory;
		outputInfo.mVendorType = vendorType;
		
		return outputInfo;
	}

	const TGraphicsContextInfo& COGLGraphicsContext::GetContextInfo() const
	{
		const static TGraphicsContextInfo infoData
		{
			{ { -1.0f, -1.0f, -1.0f }, { 1.0f, 1.0f, 1.0f } }
		};

		return infoData;
	}

	E_RESULT_CODE COGLGraphicsContext::_initFBO(const IWindowSystem* pWindowSystem)
	{
		// \todo move this code into FBO manager later
		U32 creationFlags = pWindowSystem->GetFlags();

		GL_SAFE_CALL(glGenFramebuffers(1, &mMainFBOHandler));

		if (!(creationFlags & P_ZBUFFER_ENABLED))
		{
			return RC_OK;
		}

		GL_SAFE_CALL(glGenRenderbuffers(1, &mMainDepthStencilRenderbuffer));

		GL_SAFE_VOID_CALL(glBindRenderbuffer(GL_RENDERBUFFER, mMainDepthStencilRenderbuffer));
		GL_SAFE_VOID_CALL(glBindFramebuffer(GL_FRAMEBUFFER, mMainFBOHandler));
		{
			CDeferOperation unbindBufferOperation([] { glBindRenderbuffer(GL_RENDERBUFFER, 0); glBindFramebuffer(GL_FRAMEBUFFER, 0); });

			GL_SAFE_CALL(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, pWindowSystem->GetWidth(), pWindowSystem->GetHeight()));
			GL_SAFE_CALL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mMainDepthStencilRenderbuffer));
		}

		return RC_OK;
	}


	TDE2_API IGraphicsContext* CreateOGLGraphicsContext(IWindowSystem* pWindowSystem, TCreateGLContextFactoryCallback glContextFactoryCallback, E_RESULT_CODE& result)
	{
		COGLGraphicsContext* pGraphicsContextInstance = new (std::nothrow) COGLGraphicsContext(glContextFactoryCallback);

		if (!pGraphicsContextInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pGraphicsContextInstance->Init(pWindowSystem);

		if (result != RC_OK)
		{
			delete pGraphicsContextInstance;

			pGraphicsContextInstance = nullptr;
		}

		return dynamic_cast<IGraphicsContext*>(pGraphicsContextInstance);
	}
}