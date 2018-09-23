#include "./../../../include/graphics/d3d11/CD3D11GraphicsContext.h"
#include "./../../../include/core/IWindowSystem.h"


namespace TDEngine2
{
	CD3D11GraphicsContext::CD3D11GraphicsContext() :
		mIsInitialized(false)
	{
	}

	CD3D11GraphicsContext::~CD3D11GraphicsContext()
	{
	}

	E_RESULT_CODE CD3D11GraphicsContext::Init(const IWindowSystem* pWindowSystem)
	{
		return RC_FAIL;
	}
	
	E_RESULT_CODE CD3D11GraphicsContext::Free()
	{
		return RC_FAIL;
	}
	
	void CD3D11GraphicsContext::ClearBackBuffer(const TColor32F& color)
	{
		/// \todo not implemented yet
	}

	void CD3D11GraphicsContext::Present()
	{
		/// \todo not implemented yet
	}

	const TGraphicsCtxInternalData& CD3D11GraphicsContext::GetInternalData() const
	{
		return mInternalDataObject;
	}

	E_ENGINE_SUBSYSTEM_TYPE CD3D11GraphicsContext::GetType() const
	{
		return EST_GRAPHICS_CONTEXT;
	}


	TDE2_API IGraphicsContext* CreateD3D11GraphicsContext(const IWindowSystem* pWindowSystem, E_RESULT_CODE& result)
	{
		IGraphicsContext* pGraphicsContextInstance = new (std::nothrow) CD3D11GraphicsContext();

		if (!pGraphicsContextInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pGraphicsContextInstance->Init(pWindowSystem);

		return pGraphicsContextInstance;
	}
}