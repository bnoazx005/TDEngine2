#include "./../../include/win32/CWin32GLContextFactory.h"
#include <core/IWindowSystem.h>
#include <GL/glew.h>


#if defined(TDE2_USE_WINPLATFORM)

#include <GL/wglew.h>

#pragma comment(lib, "opengl32.lib")


namespace TDEngine2
{
	/// \todo replace this array with more clear code
	static const I32 CONTEXT_ATTRIBUTES[] =
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
		WGL_CONTEXT_MINOR_VERSION_ARB, 3,
		WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		WGL_CONTEXT_FLAGS_ARB,         WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
		0
	};

	CWin32GLContextFactory::CWin32GLContextFactory():
		mIsInitialized(false)
	{
	}
	
	E_RESULT_CODE CWin32GLContextFactory::Init(IWindowSystem* pWindowSystem)
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

		mDeviceContextHandler = pWindowSystem->GetInternalData().mDeviceContextHandler;
		
		// create temporary context
		TOGLCtxHandler tempContext = _getTempContext(mDeviceContextHandler, result);
		
		if (result != RC_OK)
		{
			return result;
		}

		if (!wglMakeCurrent(mDeviceContextHandler, tempContext))
		{
			return RC_FAIL;
		}
		
		// create main context
		TWGLCreateContextWithAttribsCallback pCreateCtxWithAttribsCallback = (TWGLCreateContextWithAttribsCallback)wglGetProcAddress("wglCreateContextAttribsARB");

		if (!pCreateCtxWithAttribsCallback)
		{
			if (!wglDeleteContext(tempContext))
			{
				return RC_FAIL;
			}

			return RC_FAIL;
		}

		mCurrGLHandler = pCreateCtxWithAttribsCallback(mDeviceContextHandler, tempContext, CONTEXT_ATTRIBUTES);

		if (!mCurrGLHandler)
		{
			if (!wglDeleteContext(tempContext))
			{
				return RC_FAIL;
			}

			return RC_FAIL;
		}

		if (SetContext() != RC_OK)
		{
			if (!wglDeleteContext(tempContext))
			{
				return RC_FAIL;
			}

			if (!wglDeleteContext(mCurrGLHandler))
			{
				return RC_FAIL;
			}

			return RC_FAIL;
		}

		//delete temp context
		if (!wglDeleteContext(tempContext))
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CWin32GLContextFactory::Free()
	{
		if (!mIsInitialized)
		{
			return RC_OK;
		}

		if (!wglDeleteContext(mCurrGLHandler))
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	E_RESULT_CODE CWin32GLContextFactory::CreateContextsForWorkerThreads(U32 maxThreadsCount)
	{		
		std::lock_guard<std::mutex> lock(mMutex);

		TWGLCreateContextWithAttribsCallback pCreateCtxWithAttribsCallback = (TWGLCreateContextWithAttribsCallback)wglGetProcAddress("wglCreateContextAttribsARB");
		if (!pCreateCtxWithAttribsCallback)
		{
			return RC_FAIL;
		}

		for (U32 i = 0; i < maxThreadsCount; ++i)
		{
			mWorkerThreadsContexts.emplace_back(pCreateCtxWithAttribsCallback(mDeviceContextHandler, mCurrGLHandler, CONTEXT_ATTRIBUTES));
			TDE2_ASSERT(mWorkerThreadsContexts.back());
		}

		return RC_OK;
	}

	E_RESULT_CODE CWin32GLContextFactory::SetContextForWorkerThread()
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (mWorkerThreadsContexts[mLastUsedWorkerThreadContextCounter])
		{
			if (!wglMakeCurrent(mDeviceContextHandler, mWorkerThreadsContexts[mLastUsedWorkerThreadContextCounter++]))
			{
				return RC_FAIL;
			}

			return RC_OK;
		}

		return RC_OK;
	}

	E_RESULT_CODE CWin32GLContextFactory::SetContext()
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (!wglMakeCurrent(mDeviceContextHandler, mCurrGLHandler))
		{
			return RC_FAIL;
		}

		return RC_OK;
	}

	E_RESULT_CODE CWin32GLContextFactory::ResetContext()
	{
		if (!wglMakeCurrent(NULL, NULL))
		{
			return RC_FAIL;
		}

		return RC_OK;
	}
	
	TOGLCtxHandler CWin32GLContextFactory::GetContext() const
	{
		return mCurrGLHandler;
	}

	TOGLCtxHandler CWin32GLContextFactory::_getTempContext(const HDC& hdc, E_RESULT_CODE& result)
	{
		result = RC_OK;

		PIXELFORMATDESCRIPTOR pixelFormatDesc;
		memset(&pixelFormatDesc, 0, sizeof(pixelFormatDesc));

		pixelFormatDesc.nSize        = sizeof(pixelFormatDesc);
		pixelFormatDesc.nVersion     = 1;
		pixelFormatDesc.dwFlags      = PFD_DRAW_TO_WINDOW | LPD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		pixelFormatDesc.iPixelType   = PFD_TYPE_RGBA;
		pixelFormatDesc.cColorBits   = 32; // standard for RGBA
		pixelFormatDesc.cDepthBits   = 24;	/// \todo preliminary magic constant for depth buffer precision, should be replaced
		pixelFormatDesc.cStencilBits = 8;	/// \todo preliminary magic constant for stencil buffer precision, should be replaced

		U32  pixelFormat = 0x0;

		pixelFormat = ChoosePixelFormat(mDeviceContextHandler, &pixelFormatDesc);

		if (!pixelFormat)
		{
			result = RC_FAIL;

			return nullptr;
		}

		SetPixelFormat(mDeviceContextHandler, pixelFormat, &pixelFormatDesc);

		// create the context
		return wglCreateContext(mDeviceContextHandler);
	}


	TDE2_API IOGLContextFactory* CreateWin32GLContextFactory(IWindowSystem* pWindowSystem, E_RESULT_CODE& result)
	{
		CWin32GLContextFactory* pGLContextFactory = new (std::nothrow) CWin32GLContextFactory();

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