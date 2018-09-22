#include "./../../../include/graphics/ogl/COGLGraphicsContext.h"
#include "./../../../include/core/IWindowSystem.h"


namespace TDEngine2
{
	COGLGraphicsContext::COGLGraphicsContext():
		mIsInitialized(false)
	{
	}

	COGLGraphicsContext::~COGLGraphicsContext()
	{
	}

	E_RESULT_CODE COGLGraphicsContext::Init(const IWindowSystem* pWindowSystem)
	{
		return RC_FAIL;
	}

	E_RESULT_CODE COGLGraphicsContext::Free()
	{
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


	TDE2_API IGraphicsContext* CreateOGLGraphicsContext(const IWindowSystem* pWindowSystem, E_RESULT_CODE& result)
	{
		IGraphicsContext* pGraphicsContextInstance = new (std::nothrow) COGLGraphicsContext();

		if (!pGraphicsContextInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pGraphicsContextInstance->Init(pWindowSystem);

		return pGraphicsContextInstance;
	}
}