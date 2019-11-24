#include "./../include/COGLGraphicsContext.h"
#include "./../include/IOGLContextFactory.h"
#include "./../include/COGLGraphicsObjectManager.h"
#include "./../include/COGLMappings.h"
#include "./../include/COGLRenderTarget.h"
#include <core/IEventManager.h>
#include <core/IWindowSystem.h>


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
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LESS);
		}

		mpEventManager = pWindowSystem->GetEventManager();

		if (!mpEventManager)
		{
			return RC_FAIL;
		}

		mpEventManager->Subscribe(TOnWindowResized::GetTypeId(), this);

		mIsInitialized = true;
		
		return RC_OK;
	}

	E_RESULT_CODE COGLGraphicsContext::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = mpGraphicsObjectManager->Free();
		result = result | mpGLContextFactory->Free();

		mIsInitialized = false;

		delete this;

		return result;
	}
	
	void COGLGraphicsContext::ClearBackBuffer(const TColor32F& color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	void COGLGraphicsContext::ClearDepthBuffer(F32 value)
	{
		glClearDepthf(value);
		glClear(GL_DEPTH_BUFFER_BIT);
	}
	
	void COGLGraphicsContext::ClearStencilBuffer(U8 value)
	{
		glClearStencil(value);
		glClear(GL_STENCIL_BUFFER_BIT);
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
		glViewport(static_cast<GLint>(x), static_cast<GLint>(y), static_cast<GLsizei>(width), static_cast<GLsizei>(height));
	}

	TMatrix4 COGLGraphicsContext::CalcPerspectiveMatrix(F32 fov, F32 aspect, F32 zn, F32 zf)
	{
		return PerspectiveProj(fov, aspect, zn, zf, -1.0f, 1.0f, 1.0f);
	}

	TMatrix4 COGLGraphicsContext::CalcOrthographicMatrix(F32 left, F32 top, F32 right, F32 bottom, F32 zn, F32 zf)
	{
		return OrthographicProj(left, top, right, bottom, zn, zf, -1.0f, 1.0f, 1.0f);
	}

	void COGLGraphicsContext::Draw(E_PRIMITIVE_TOPOLOGY_TYPE topology, U32 startVertex, U32 numOfVertices)
	{
		glDrawArrays(COGLMappings::GetPrimitiveTopology(topology), startVertex, numOfVertices);
	}

	void COGLGraphicsContext::DrawIndexed(E_PRIMITIVE_TOPOLOGY_TYPE topology, E_INDEX_FORMAT_TYPE indexFormatType, U32 baseVertex, U32 startIndex, U32 numOfIndices)
	{
		glDrawElements(COGLMappings::GetPrimitiveTopology(topology), numOfIndices, COGLMappings::GetIndexFormat(indexFormatType), 0);
	}

	void COGLGraphicsContext::DrawInstanced(E_PRIMITIVE_TOPOLOGY_TYPE topology, U32 startVertex, U32 verticesPerInstance, U32 startInstance, U32 numOfInstances)
	{
		glDrawArraysInstanced(COGLMappings::GetPrimitiveTopology(topology), startVertex, verticesPerInstance, numOfInstances);
	}

	void COGLGraphicsContext::DrawIndexedInstanced(E_PRIMITIVE_TOPOLOGY_TYPE topology, E_INDEX_FORMAT_TYPE indexFormatType, U32 baseVertex, U32 startIndex,
												   U32 startInstance, U32 indicesPerInstance, U32 numOfInstances)
	{
		glDrawElementsInstanced(COGLMappings::GetPrimitiveTopology(topology), indicesPerInstance, COGLMappings::GetIndexFormat(indexFormatType), 0, numOfInstances);
	}

	void COGLGraphicsContext::BindTextureSampler(U32 slot, TTextureSamplerId samplerId)
	{
		GLuint internalSamplerId = dynamic_cast<COGLGraphicsObjectManager*>(mpGraphicsObjectManager)->GetTextureSampler(samplerId).Get();

		glBindSampler(slot, internalSamplerId);
	}

	void COGLGraphicsContext::BindBlendState(TBlendStateId blendStateId)
	{
		const TBlendStateDesc& blendStateDesc = dynamic_cast<COGLGraphicsObjectManager*>(mpGraphicsObjectManager)->GetBlendState(blendStateId).Get();

		auto stateFunction = blendStateDesc.mIsEnabled ? glEnable : glDisable;
		stateFunction(GL_BLEND);

		glBlendFuncSeparate(COGLMappings::GetBlendFactorValue(blendStateDesc.mScrValue),
							COGLMappings::GetBlendFactorValue(blendStateDesc.mDestValue),
							COGLMappings::GetBlendFactorValue(blendStateDesc.mScrAlphaValue),
							COGLMappings::GetBlendFactorValue(blendStateDesc.mDestAlphaValue));

		glBlendEquationSeparate(COGLMappings::GetBlendOpType(blendStateDesc.mOpType), 
								COGLMappings::GetBlendOpType(blendStateDesc.mAlphaOpType));
	}

	void COGLGraphicsContext::BindRenderTarget(IRenderTarget* pRenderTarget)
	{
		if (!pRenderTarget)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			return;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, dynamic_cast<COGLRenderTarget*>(pRenderTarget)->GetInternalHandler());
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
		return GetTypeId();
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