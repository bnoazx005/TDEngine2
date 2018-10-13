#include "./../../include/unix/CUnixGLContextFactory.h"
#include <core/IWindowSystem.h>
#include <platform/unix/CUnixWindowSystem.h>
#include <GL/glew.h>


#if defined(TDE2_USE_UNIXPLATFORM)

#include <gl/glxew.h>
#include <X11/Xlib.h>


namespace TDEngine2
{
	CUnixGLContextFactory::CUnixGLContextFactory() :
		mIsInitialized(false)
	{
	}

	E_RESULT_CODE CUnixGLContextFactory::Init(IWindowSystem* pWindowSystem)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		if (!pWindowSystem)
		{
			return RC_INVALID_ARGS;
		}

		E_RESULT_CODE result = RC_OK;

		TWindowSystemInternalData winData = pWindowSystem->GetInternalData();

		mpDisplayHandler = winData.mpDisplayHandler;

		mWindowHandler = winData.mWindowHandler;

		I32 screenId = windData.mScreenId;

		if (!mpDisplayHandler)
		{
			return RC_FAIL;
		}

		GLint glxAttribs[] = 
		{
			GLX_X_RENDERABLE    , True,
			GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
			GLX_RENDER_TYPE     , GLX_RGBA_BIT,
			GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
			GLX_RED_SIZE        , 8,
			GLX_GREEN_SIZE      , 8,
			GLX_BLUE_SIZE       , 8,
			GLX_ALPHA_SIZE      , 8,
			GLX_DEPTH_SIZE      , 24,
			GLX_STENCIL_SIZE    , 8,
			GLX_DOUBLEBUFFER    , true,
			None
		};

		I32 frameBuffersCount = 0;

		GLXFBConfig* frameBufferConfig = glXChooseFBConfig(mpDisplayHandler, screenId, glxAttribs, &frameBuffersCount);

		if (!frameBufferConfig)
		{
			return RC_FAIL;
		}

		GLXFBConfig currentFrameBuffer = frameBufferConfig[0];

		XFree(frameBufferConfig);

		XVisualInfo* pVisualInfo = glXGetVisualFromFBConfig(mpDisplayHandler, currentFrameBuffer);

		if (!pVisualInfo || (screenId != visual->screen))
		{
			return RC_FAIL;
		}

		if ((result = (dynamic_cast<CUnixWindowSystem*>(pWindowSystem))->EnableOpenGL(pVisualInfo)) != RC_OK)
		{
			return result;
		}
		
		winData = pWindowSystem->GetInternalData();

		mWindowHandler = winData.mWindowHandler;

		// create and set context
		TGLXCreateContextAttribsARBCallback glXCreateContextAttribsARB = 
							(TGLXCreateContextAttribsARBCallback)glXGetProcAddressARB((const GLubyte*)"glXCreateContextAttribsARB");

		I32 attributes[] = 
		{
			GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
			GLX_CONTEXT_MINOR_VERSION_ARB, 3,
			GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
			None
		};
		
		mCurrGLHandler = glXCreateContextAttribsARB(mpDisplayHandler, currentFrameBuffer, 0, true, attributes);

		if ((result = SetContext()) != RC_OK)
		{
			return result;
		}
		
		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CUnixGLContextFactory::Free()
	{
		if (!mIsInitialized)
		{
			return RC_OK;
		}

		if (!glXDestroyContext(mpDisplayHandler, mCurrGLHandler))
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	E_RESULT_CODE CUnixGLContextFactory::SetContext()
	{
		if (!glXMakeCurrent(mDeviceContextHandler, mWindowHandler, mCurrGLHandler))
		{
			return RC_FAIL;
		}

		return RC_OK;
	}

	E_RESULT_CODE CUnixGLContextFactory::ResetContext()
	{
		if (!glXMakeCurrent(mDeviceContextHandler, NULL, NULL))
		{
			return RC_FAIL;
		}

		return RC_OK;
	}

	TOGLCtxHandler CUnixGLContextFactory::GetContext() const
	{
		return mCurrGLHandler;
	}


	TDE2_API IOGLContextFactory* CreateUnixGLContextFactory(IWindowSystem* pWindowSystem, E_RESULT_CODE& result)
	{
		CUnixGLContextFactory* pGLContextFactory = new (std::nothrow) CUnixGLContextFactory();

		if (!pGLContextFactory)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pGLContextFactory->Init(pWindowSystem);

		if (result != RC_OK)
		{
			delete pGLContextFactory;

			pGLContextFactory = nullptr;
		}

		return dynamic_cast<IOGLContextFactory*>(pGLContextFactory);
	}
}

#endif