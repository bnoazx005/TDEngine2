#include "./../include/CImGUIContext.h"
#include <core/IWindowSystem.h>
#include <core/IGraphicsContext.h>
#include <core/IInputContext.h>
#include <core/IResourceManager.h>
#include <core/IResourceHandler.h>
#include <graphics/CBaseTexture2D.h>
#include <graphics/IGraphicsObjectManager.h>
#include <platform/win32/CWin32WindowSystem.h>
#include <platform/unix/CUnixWindowSystem.h>
#include "./../deps/imgui-1.72/imgui.h"
#include <cassert>


namespace TDEngine2
{
	CImGUIContext::CImGUIContext():
		mIsInitialized(false)
	{
	}

	E_RESULT_CODE CImGUIContext::Init(IWindowSystem* pWindowSystem, IGraphicsObjectManager* pGraphicsObjectManager,
									  IResourceManager* pResourceManager, IInputContext* pInputContext)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pWindowSystem ||
			!pGraphicsObjectManager ||
			!pResourceManager ||
			!pInputContext)
		{
			return RC_INVALID_ARGS;	
		}
		
		mpWindowSystem          = pWindowSystem;
		mpGraphicsContext       = pGraphicsObjectManager->GetGraphicsContext();
		mpGraphicsObjectManager = pGraphicsObjectManager;
		mpResourceManager       = pResourceManager;
		mpInputContext          = pInputContext;

		if (!mpGraphicsContext) // \note the really strange case, but if it's happened we should check for it
		{
			return RC_FAIL;
		}

		// \note Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		mpIOContext = &ImGui::GetIO();
		// \todo fill in information about a platform and a renderer

		E_RESULT_CODE result = _initInternalImGUIContext(*mpIOContext);
		if (result != RC_OK)
		{
			return result;
		}

		// \note bind ImGUI to the graphics context
		

		// \note Setup Dear ImGui style
		ImGui::StyleColorsDark();

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CImGUIContext::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	E_RESULT_CODE CImGUIContext::ConfigureForWin32Platform(const CWin32WindowSystem* pWindowSystem)
	{
		if (!mpIOContext)
		{
			return RC_FAIL;
		}

		mpIOContext->BackendPlatformName = "Win32Platform";
		mpIOContext->ImeWindowHandle = pWindowSystem->GetInternalData().mWindowHandler;

		return RC_OK;
	}

	E_RESULT_CODE CImGUIContext::ConfigureForUnixPlatform(const CUnixWindowSystem* pWindowSystem)
	{
		if (!mpIOContext)
		{
			return RC_FAIL;
		}

		mpIOContext->BackendPlatformName = "UnixPlatform";
		TDE2_UNIMPLEMENTED();

		return RC_OK;
	}

	void CImGUIContext::BeginFrame(float dt)
	{
		if (!mpIOContext)
		{
			return;
		}

		mpIOContext->DeltaTime = dt;

		// \note update current display size
		TRectU32&& windowRect = mpWindowSystem->GetWindowRect();
		mpIOContext->DisplaySize = ImVec2(static_cast<F32>(windowRect.width), static_cast<F32>(windowRect.height));

		_updateInputState(*mpIOContext, mpInputContext);
		// \todo 

		ImGui::NewFrame();
	}

	void CImGUIContext::EndFrame()
	{
		if (!mpIOContext)
		{
			return;
		}

		ImGui::EndFrame();
	}

	E_ENGINE_SUBSYSTEM_TYPE CImGUIContext::GetType() const
	{
		return EST_IMGUI_CONTEXT;
	}

	E_RESULT_CODE CImGUIContext::_initInternalImGUIContext(ImGuiIO& io)
	{
		E_RESULT_CODE result = RC_OK;

		// \note bind ImGUI to the window system
		if ((result = mpWindowSystem->ConfigureImGUIContext(this)) != RC_OK)
		{
			return result;
		}

		io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
		io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

		io.Fonts->AddFontDefault();

		// \note Initialize graphical binding
		if ((result = _initGraphicsResources(io, mpGraphicsContext, mpGraphicsObjectManager, mpResourceManager)) != RC_OK)
		{
			return result;
		}

		// \todo Implement input system binding
		auto ic = dynamic_cast<IDesktopInputContext*>(mpInputContext);

		return RC_OK;
	}

	void CImGUIContext::_updateInputState(ImGuiIO& io, IInputContext* pInputContext)
	{
		IDesktopInputContext* pDesktopInputCtx = dynamic_cast<IDesktopInputContext*>(mpInputContext);

		assert(pDesktopInputCtx);

		io.KeyAlt   = pDesktopInputCtx->IsKey(E_KEYCODES::KC_LALT) || pDesktopInputCtx->IsKey(E_KEYCODES::KC_RALT);
		io.KeyCtrl  = pDesktopInputCtx->IsKey(E_KEYCODES::KC_LCONTROL) || pDesktopInputCtx->IsKey(E_KEYCODES::KC_RCONTROL);
		io.KeyShift = pDesktopInputCtx->IsKey(E_KEYCODES::KC_LSHIFT) || pDesktopInputCtx->IsKey(E_KEYCODES::KC_RSHIFT);
		io.KeySuper = pDesktopInputCtx->IsKey(E_KEYCODES::KC_LSYS) || pDesktopInputCtx->IsKey(E_KEYCODES::KC_RSYS);

		// \note Update mouse position
		TVector3&& mousePosition = pDesktopInputCtx->GetMousePosition();
		io.MousePos = ImVec2(mousePosition.x, mousePosition.y);

		// \todo Add update of information about pressed keys and buttons

		// \todo Implement support of gamepads
	}

	E_RESULT_CODE CImGUIContext::_initGraphicsResources(ImGuiIO& io, IGraphicsContext* pGraphicsContext, IGraphicsObjectManager* pGraphicsManager,
														IResourceManager* pResourceManager)
	{
		io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

		E_RESULT_CODE result = RC_OK;

		// \todo Create a vertex shader
		// \todo Create a pixel shader
		// \todo Create a render state

		// \todo Create a blending state
		auto blendStateResult = pGraphicsManager->CreateBlendState({});
		if (blendStateResult.HasError())
		{
			return blendStateResult.GetError();
		}

		mBlendStateHandle = blendStateResult.Get();

		// \todo Create a depth-stencil state
		auto depthHandleResult = pGraphicsManager->CreateDepthStencilState({});
		if (depthHandleResult.HasError())
		{
			return depthHandleResult.GetError();
		}

		mDepthStencilStateHandle = depthHandleResult.Get();

		// \note Create a font texture
		if ((result = _initSystemFonts(io, pResourceManager, pGraphicsManager)) != RC_OK)
		{
			return result;
		}

		return RC_OK;
	}

	E_RESULT_CODE CImGUIContext::_initSystemFonts(ImGuiIO& io, IResourceManager* pResourceManager, IGraphicsObjectManager* pGraphicsManager)
	{
		U8* pPixelsData = nullptr;
		I32 width = -1;
		I32 height = -1;
		io.Fonts->GetTexDataAsRGBA32(&pPixelsData, &width, &height);

		mpFontTextureHandler = mpResourceManager->Create<CBaseTexture2D>("imgui_defaultfont_texatlas",
																		 TTexture2DParameters
																		 {
																			 static_cast<U32>(width),
																			 static_cast<U32>(height),
																			 FT_NORM_UBYTE4, 1, 1, 0
																		 });

		if (!mpFontTextureHandler->IsValid())
		{
			return RC_FAIL;
		}

		auto pRawTextureResource = mpFontTextureHandler->Get<CBaseTexture2D>(RAT_BLOCKING);
		if (!pRawTextureResource)
		{
			return RC_FAIL;
		}

		io.Fonts->TexID = static_cast<ImTextureID>(mpFontTextureHandler);
		
		E_RESULT_CODE result = pRawTextureResource->WriteData({ 0, 0, width, height }, pPixelsData);
		if (result != RC_OK)
		{
			return result;
		}

		// \note Create a texture sampler
		TTextureSamplerDesc textureSamplerDesc;
		textureSamplerDesc.mUAddressMode = E_ADDRESS_MODE_TYPE::AMT_WRAP;
		textureSamplerDesc.mVAddressMode = E_ADDRESS_MODE_TYPE::AMT_WRAP;
		textureSamplerDesc.mWAddressMode = E_ADDRESS_MODE_TYPE::AMT_WRAP;

		textureSamplerDesc.mFilterFlags = (U32)E_FILTER_TYPE::FT_BILINEAR << 16 |
										  (U32)E_FILTER_TYPE::FT_BILINEAR << 8 |
										  (U32)E_FILTER_TYPE::FT_BILINEAR;

		auto samplerResult = pGraphicsManager->CreateTextureSampler(textureSamplerDesc);
		if (samplerResult.HasError())
		{
			return samplerResult.GetError();
		}

		mFontTextureSamplerHandle = samplerResult.Get();

		return RC_OK;
	}


	TDE2_API IImGUIContext* CreateImGUIContext(IWindowSystem* pWindowSystem, IGraphicsObjectManager* pGraphicsObjectManager,
											   IResourceManager* pResourceManager, IInputContext* pInputContext, E_RESULT_CODE& result)
	{
		CImGUIContext* pImGUIContextInstance = new (std::nothrow) CImGUIContext();

		if (!pImGUIContextInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return pImGUIContextInstance;
		}

		result = pImGUIContextInstance->Init(pWindowSystem, pGraphicsObjectManager, pResourceManager, pInputContext);

		if (result != RC_OK)
		{
			delete pImGUIContextInstance;

			pImGUIContextInstance = nullptr;
		}

		return dynamic_cast<IImGUIContext*>(pImGUIContextInstance);
	}
}