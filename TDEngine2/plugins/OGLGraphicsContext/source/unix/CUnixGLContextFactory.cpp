#include "./../../include/unix/CUnixGLContextFactory.h"
#include <core/IWindowSystem.h>
#include <platform/unix/CUnixWindowSystem.h>
#include <GL/glew.h>


#if defined(TDE2_USE_UNIXPLATFORM)

#include <GL/glxew.h>
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

		I32 screenId = winData.mScreenId;

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

		PFNGLXCHOOSEFBCONFIGPROC pGLXChooseFBConfig = (PFNGLXCHOOSEFBCONFIGPROC)glXGetProcAddressARB((const GLubyte*)"glXChooseFBConfig");
		
		if (!pGLXChooseFBConfig)
		{
			return RC_FAIL;
		}

		GLXFBConfig* frameBufferConfig = pGLXChooseFBConfig(mpDisplayHandler, screenId, glxAttribs, &frameBuffersCount);

		if (!frameBufferConfig)
		{
			return RC_FAIL;
		}

		GLXFBConfig currentFrameBuffer = frameBufferConfig[0];

		XFree(frameBufferConfig);

		PFNGLXGETVISUALFROMFBCONFIGPROC pGLXGetVisualFromFBConfig = 
										(PFNGLXGETVISUALFROMFBCONFIGPROC)glXGetProcAddressARB((const GLubyte*)"glXGetVisualFromFBConfig");

		if (!pGLXGetVisualFromFBConfig)
		{
			return RC_FAIL;
		}

		XVisualInfo* pVisualInfo = pGLXGetVisualFromFBConfig(mpDisplayHandler, currentFrameBuffer);

		if (!pVisualInfo || (screenId != pVisualInfo->screen))
		{
			return RC_FAIL;
		}

		if ((result = (dynamic_cast<CUnixWindowSystem*>(pWindowSystem))->EnableOpenGL(pVisualInfo)) != RC_OK)
		{
			return result;
		}
		
		winData = pWindowSystem->GetInternalData();

		mWindowHandler = winData.mWindowHandler;

		/// \todo add creation of legacy context if the new one is not supported by a hardware

		// create and set context
		TGLXCreateContextAttribsARBCallback pGLXCreateContextAttribsARB = 
							(TGLXCreateContextAttribsARBCallback)glXGetProcAddressARB((const GLubyte*)"glXCreateContextAttribsARB");

		if (!pGLXCreateContextAttribsARB)
		{
			return RC_FAIL;
		}

		I32 attributes[] = 
		{
			GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
			GLX_CONTEXT_MINOR_VERSION_ARB, 0,
			GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
			None
		};
		
		mCurrGLHandler = pGLXCreateContextAttribsARB(mpDisplayHandler, currentFrameBuffer, 0, true, attributes);

		if ((result = SetContext()) != RC_OK)
		{
			return result;
		}
		
		if (glxewInit() != GLEW_NO_ERROR)
		{
			return RC_FAIL;
		}

		XFree(pVisualInfo);

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CUnixGLContextFactory::Free()
	{
		if (!mIsInitialized)
		{
			return RC_OK;
		}

		glXDestroyContext(mpDisplayHandler, mCurrGLHandler);

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	E_RESULT_CODE CUnixGLContextFactory::SetContext()
	{
		if (!glXMakeCurrent(mpDisplayHandler, mWindowHandler, mCurrGLHandler))
		{
			return RC_FAIL;
		}

		return RC_OK;
	}

	E_RESULT_CODE CUnixGLContextFactory::ResetContext()
	{
		if (!glXMakeCurrent(mpDisplayHandler, NULL, NULL))
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