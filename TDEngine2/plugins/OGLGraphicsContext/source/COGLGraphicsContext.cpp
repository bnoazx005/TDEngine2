#include "./../include/COGLGraphicsContext.h"
#include "./../include/IOGLContextFactory.h"
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
			return RC_OK;
		}
		
		if (!mGLContextFactoryCallback)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		mWindowInternalData = pWindowSystem->GetInternalData();

		/// creating GL context
		mpGLContextFactory = mGLContextFactoryCallback(pWindowSystem, result);

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

		mIsInitialized = true;
		
		return RC_OK;
	}

	E_RESULT_CODE COGLGraphicsContext::Free()
	{
		if (!mIsInitialized)
		{
			return RC_OK;
		}

		E_RESULT_CODE result = RC_OK;

		if ((result = mpGLContextFactory->Free()) != RC_OK)
		{
			return result;
		}

		mIsInitialized = false;

		delete this;

		return RC_FAIL;
	}
	
	void COGLGraphicsContext::ClearBackBuffer(const TColor32F& color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
		glClear(GL_COLOR_BUFFER_BIT);
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

	const TGraphicsCtxInternalData& COGLGraphicsContext::GetInternalData() const
	{
		return mInternalDataObject;
	}

	E_ENGINE_SUBSYSTEM_TYPE COGLGraphicsContext::GetType() const
	{
		return EST_GRAPHICS_CONTEXT;
	}


	TDE2_API IGraphicsContext* CreateOGLGraphicsContext(IWindowSystem* pWindowSystem, TCreateGLContextFactoryCallback glContextFactoryCallback, E_RESULT_CODE& result)
	{
		IGraphicsContext* pGraphicsContextInstance = new (std::nothrow) COGLGraphicsContext(glContextFactoryCallback);

		if (!pGraphicsContextInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pGraphicsContextInstance->Init(pWindowSystem);

		return pGraphicsContextInstance;
	}
}