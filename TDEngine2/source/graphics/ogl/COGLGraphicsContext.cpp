#include "./../../../include/graphics/ogl/COGLGraphicsContext.h"
#include "./../../../include/core/IWindowSystem.h"
#include "./../../../include/graphics/ogl/IOGLContextFactory.h"
#include <gl\glew.h>


#if defined (TDE2_USE_WIN32PLATFORM)
	#pragma comment(lib, "glew32.lib")
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

	E_RESULT_CODE COGLGraphicsContext::Init(const IWindowSystem* pWindowSystem)
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

		/// creating GL context
		mGLContextFactory = mGLContextFactoryCallback(pWindowSystem, result);

		if (result != RC_OK)
		{
			return result;
		}

		result = mGLContextFactory->SetContext();

		if (result != RC_OK)
		{
			return result;
		}

		/// initialization of GLEW
		glewExperimental = true;

		GLenum err = glewInit();
		
		if (err != GLEW_OK)
		{
			MessageBox(0, (const char*)glewGetErrorString(err), 0, 0);
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

		mIsInitialized = false;

		delete this;

		return RC_FAIL;
	}

	const TGraphicsCtxInternalData& COGLGraphicsContext::GetInternalData() const
	{
		return mInternalDataObject;
	}

	E_ENGINE_SUBSYSTEM_TYPE COGLGraphicsContext::GetType() const
	{
		return EST_GRAPHICS_CONTEXT;
	}


	TDE2_API IGraphicsContext* CreateOGLGraphicsContext(const IWindowSystem* pWindowSystem, TCreateGLContextFactoryCallback glContextFactoryCallback, E_RESULT_CODE& result)
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