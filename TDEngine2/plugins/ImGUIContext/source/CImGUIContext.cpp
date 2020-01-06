#include "./../include/CImGUIContext.h"
#include <utils/Types.h>
#include <core/IWindowSystem.h>
#include <core/IGraphicsContext.h>
#include <core/IInputContext.h>
#include <core/IResourceManager.h>
#include <core/IResourceHandler.h>
#include <graphics/CBaseTexture2D.h>
#include <graphics/IGraphicsObjectManager.h>
#include <graphics/IIndexBuffer.h>
#include <graphics/IVertexBuffer.h>
#include <graphics/CBaseMaterial.h>
#include <graphics/IRenderer.h>
#include <graphics/CRenderQueue.h>
#include <graphics/IVertexDeclaration.h>
#include <platform/win32/CWin32WindowSystem.h>
#include <platform/unix/CUnixWindowSystem.h>
#include <utils/CFileLogger.h>
#include "./../deps/imgui-1.72/imgui.h"
#include <cassert>
#include <vector>
#include <cstring>


namespace TDEngine2
{
	CImGUIContext::CImGUIContext():
		mIsInitialized(false)
	{
	}

	E_RESULT_CODE CImGUIContext::Init(IWindowSystem* pWindowSystem, IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager,
									  IResourceManager* pResourceManager, IInputContext* pInputContext)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pWindowSystem ||
			!pGraphicsObjectManager ||
			!pResourceManager ||
			!pRenderer ||
			!pInputContext)
		{
			return RC_INVALID_ARGS;	
		}
		
		mpWindowSystem          = pWindowSystem;
		mpGraphicsContext       = pGraphicsObjectManager->GetGraphicsContext();
		mpGraphicsObjectManager = pGraphicsObjectManager;
		mpResourceManager       = pResourceManager;
		mpInputContext          = pInputContext;
		mpEditorUIRenderQueue	= pRenderer->GetRenderQueue(E_RENDER_QUEUE_GROUP::RQG_DEBUG);

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

#if defined(TDE2_USE_WIN32PLATFORM)
		mpIOContext->BackendPlatformName = "Win32Platform";
		mpIOContext->ImeWindowHandle = pWindowSystem->GetInternalData().mWindowHandler;
#endif

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
		TRectU32&& windowRect = mpWindowSystem->GetClientRect();
		mpIOContext->DisplaySize = ImVec2(static_cast<F32>(windowRect.width), static_cast<F32>(windowRect.height));

		_updateInputState(*mpIOContext, mpInputContext);

		ImGui::NewFrame();
	}

	void CImGUIContext::EndFrame()
	{
		if (!mpIOContext)
		{
			return;
		}
		static bool f = true;
		ImGui::ShowDemoWindow(&f);

		ImGui::Render();
		_engineInternalRender(ImGui::GetDrawData(), mpEditorUIRenderQueue);
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

		_initInputMappings(io);

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
		io.MousePos = ImVec2(mousePosition.x, mpIOContext->DisplaySize.y - mousePosition.y);

		for (U8 buttonId = 0; buttonId < sizeof(io.MouseDown) / sizeof(bool); ++buttonId)
		{
			io.MouseDown[buttonId] = pDesktopInputCtx->IsMouseButton(buttonId);
		}

		// \todo Add update of information about pressed keys and buttons

		TVector3 mouseShiftVector = pDesktopInputCtx->GetMouseShiftVec();
		io.MouseWheel += mouseShiftVector.z;
		// \todo Implement support of gamepads
	}

	E_RESULT_CODE CImGUIContext::_initGraphicsResources(ImGuiIO& io, IGraphicsContext* pGraphicsContext, IGraphicsObjectManager* pGraphicsManager,
														IResourceManager* pResourceManager)
	{
		io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

		E_RESULT_CODE result = RC_OK;

		auto vertexBufferResult = pGraphicsManager->CreateVertexBuffer(BUT_DYNAMIC, 8192 * sizeof(ImDrawVert), nullptr);
		if (vertexBufferResult.HasError())
		{
			return vertexBufferResult.GetError();
		}

		mpVertexBuffer = vertexBufferResult.Get();

		auto indexBufferResult = pGraphicsManager->CreateIndexBuffer(BUT_DYNAMIC, IFT_INDEX16, 65536 * sizeof(U16), nullptr);
		if (indexBufferResult.HasError())
		{
			return indexBufferResult.GetError();
		}

		mpIndexBuffer = indexBufferResult.Get();

		mpEditorUIVertexDeclaration = pGraphicsManager->CreateVertexDeclaration().Get();
		mpEditorUIVertexDeclaration->AddElement({ TDEngine2::FT_FLOAT4, 0, TDEngine2::VEST_POSITION });
		mpEditorUIVertexDeclaration->AddElement({ TDEngine2::FT_NORM_UBYTE4, 0, TDEngine2::VEST_COLOR });

		// \note load default editor's material (depth test and writing to the depth buffer are disabled)
		TMaterialParameters editorUIMaterialParams { "DefaultEditorUI", true, { false, false } };

		auto& blendingParams = editorUIMaterialParams.mBlendingParams;
		blendingParams.mScrValue       = E_BLEND_FACTOR_VALUE::SOURCE_ALPHA;
		blendingParams.mDestValue      = E_BLEND_FACTOR_VALUE::ONE_MINUS_SOURCE_ALPHA;
		blendingParams.mScrAlphaValue  = E_BLEND_FACTOR_VALUE::ONE_MINUS_SOURCE_ALPHA;
		blendingParams.mDestAlphaValue = E_BLEND_FACTOR_VALUE::ZERO;

		mpDefaultEditorMaterial = pResourceManager->Create<CBaseMaterial>("DefaultEditorUIMaterial.material", editorUIMaterialParams);

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

		return RC_OK;
	}

	void CImGUIContext::_engineInternalRender(ImDrawData* pImGUIData, CRenderQueue* pRenderQueue)
	{
		mpVertexBuffer->Map(BMT_WRITE_DISCARD);
		mpIndexBuffer->Map(BMT_WRITE_DISCARD);
		{
			std::vector<ImDrawVert> vertices(pImGUIData->TotalVtxCount);
			std::vector<ImDrawIdx> indices(pImGUIData->TotalIdxCount);

			ImDrawVert* pCurrVertexPtr = !vertices.empty() ? &vertices[0] : nullptr;
			ImDrawIdx* pCurrIndexPtr = !indices.empty() ? &indices[0] : nullptr;

			for (I32 n = 0; n < pImGUIData->CmdListsCount; ++n)
			{
				const ImDrawList* pCommandList = pImGUIData->CmdLists[n];

				memcpy(pCurrVertexPtr, pCommandList->VtxBuffer.Data, pCommandList->VtxBuffer.Size * sizeof(ImDrawVert));
				memcpy(pCurrIndexPtr, pCommandList->IdxBuffer.Data, pCommandList->IdxBuffer.Size * sizeof(ImDrawIdx));
				
				pCurrVertexPtr += pCommandList->VtxBuffer.Size;
				pCurrIndexPtr += pCommandList->IdxBuffer.Size;
			}

			if (!vertices.empty())
			{
				mpVertexBuffer->Write(&vertices[0], sizeof(ImDrawVert) * vertices.size());
			}

			if (!indices.empty())
			{
				mpIndexBuffer->Write(&indices[0], sizeof(ImDrawIdx) * indices.size());
			}
		}
		mpIndexBuffer->Unmap();
		mpVertexBuffer->Unmap();

		// \note Render command lists
		I32 currIndexOffset  = 0;
		I32 currVertexOffset = 0;

		TMatrix4 projectionMatrix = mpGraphicsContext->CalcOrthographicMatrix(pImGUIData->DisplayPos.x, pImGUIData->DisplayPos.y,
																			  pImGUIData->DisplayPos.x + pImGUIData->DisplaySize.x,
																			  pImGUIData->DisplayPos.y + pImGUIData->DisplaySize.y,
																			  0.0f, 1.0f, true);

		ImVec2 clipRect = pImGUIData->DisplayPos;

		U32 batchId = 0x0;

		for (I32 n = 0; n < pImGUIData->CmdListsCount; ++n)
		{
			const ImDrawList* pCommandList = pImGUIData->CmdLists[n];

			for (I32 currCommandIndex = 0; currCommandIndex < pCommandList->CmdBuffer.Size; ++currCommandIndex)
			{
				const ImDrawCmd* pCurrCommand = &pCommandList->CmdBuffer[currCommandIndex];
				
				auto pTexture = static_cast<IResourceHandler*>(pCurrCommand->TextureId);
				mpDefaultEditorMaterial->Get<IMaterial>(RAT_BLOCKING)->SetTextureResource("Texture", pTexture->Get<ITexture>(RAT_BLOCKING));

				TDrawIndexedCommand* pCurrDrawCommand = pRenderQueue->SubmitDrawCommand<TDrawIndexedCommand>(0xFFFFFFF0 - ++batchId);

				pCurrDrawCommand->mObjectData.mUnused = Transpose(projectionMatrix); // \note assign it as ModelMat and don't use global ProjMat
				pCurrDrawCommand->mpVertexDeclaration = mpEditorUIVertexDeclaration;
				pCurrDrawCommand->mpVertexBuffer      = mpVertexBuffer;
				pCurrDrawCommand->mpIndexBuffer       = mpIndexBuffer;
				pCurrDrawCommand->mpMaterialHandler   = mpDefaultEditorMaterial;
				pCurrDrawCommand->mPrimitiveType      = E_PRIMITIVE_TOPOLOGY_TYPE::PTT_TRIANGLE_LIST;
				pCurrDrawCommand->mNumOfIndices       = pCurrCommand->ElemCount;
				pCurrDrawCommand->mStartIndex         = pCurrCommand->IdxOffset + currIndexOffset;
				pCurrDrawCommand->mStartVertex        = pCurrCommand->VtxOffset + currVertexOffset;

				//if (pCurrCommand->UserCallback != NULL)
				//{
				//	// User callback, registered via ImDrawList::AddCallback()
				//	// (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
				//	if (pCurrCommand->UserCallback == ImDrawCallback_ResetRenderState)
				//		ImGui_ImplDX11_SetupRenderState(draw_data, ctx);
				//	else
				//		pCurrCommand->UserCallback(cmd_list, pCurrCommand);
				//}
				//else
				//{
				//	// Apply scissor/clipping rectangle
				//	const D3D11_RECT r = { (LONG)(pCurrCommand->ClipRect.x - clip_off.x), (LONG)(pCurrCommand->ClipRect.y - clip_off.y), (LONG)(pCurrCommand->ClipRect.z - clip_off.x), (LONG)(pCurrCommand->ClipRect.w - clip_off.y) };
				//	ctx->RSSetScissorRects(1, &r);
				//}
			}

			currIndexOffset  += pCommandList->IdxBuffer.Size;
			currVertexOffset += pCommandList->VtxBuffer.Size;
		}
	}

	void CImGUIContext::_initInputMappings(ImGuiIO& io)
	{
		io.KeyMap[ImGuiKey_Tab]        = static_cast<I32>(E_KEYCODES::KC_TAB);
		io.KeyMap[ImGuiKey_LeftArrow]  = static_cast<I32>(E_KEYCODES::KC_LEFT);
		io.KeyMap[ImGuiKey_RightArrow] = static_cast<I32>(E_KEYCODES::KC_RIGHT);
		io.KeyMap[ImGuiKey_UpArrow]    = static_cast<I32>(E_KEYCODES::KC_UP);
		io.KeyMap[ImGuiKey_DownArrow]  = static_cast<I32>(E_KEYCODES::KC_DOWN);
		io.KeyMap[ImGuiKey_PageUp]     = static_cast<I32>(E_KEYCODES::KC_PAGEUP);
		io.KeyMap[ImGuiKey_PageDown]   = static_cast<I32>(E_KEYCODES::KC_PAGEDOWN);
		io.KeyMap[ImGuiKey_Home]       = static_cast<I32>(E_KEYCODES::KC_HOME);
		io.KeyMap[ImGuiKey_End]        = static_cast<I32>(E_KEYCODES::KC_END);
		io.KeyMap[ImGuiKey_Insert]     = static_cast<I32>(E_KEYCODES::KC_INSERT);
		io.KeyMap[ImGuiKey_Delete]     = static_cast<I32>(E_KEYCODES::KC_DELETE);
		io.KeyMap[ImGuiKey_Backspace]  = static_cast<I32>(E_KEYCODES::KC_BACKSPACE);
		io.KeyMap[ImGuiKey_Space]      = static_cast<I32>(E_KEYCODES::KC_SPACE);
		io.KeyMap[ImGuiKey_Enter]      = static_cast<I32>(E_KEYCODES::KC_RETURN);
		io.KeyMap[ImGuiKey_Escape]     = static_cast<I32>(E_KEYCODES::KC_ESCAPE);

		// \todo add rest of mappings
	}


	TDE2_API IImGUIContext* CreateImGUIContext(IWindowSystem* pWindowSystem, IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager,
											   IResourceManager* pResourceManager, IInputContext* pInputContext, E_RESULT_CODE& result)
	{
		CImGUIContext* pImGUIContextInstance = new (std::nothrow) CImGUIContext();

		if (!pImGUIContextInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return pImGUIContextInstance;
		}

		result = pImGUIContextInstance->Init(pWindowSystem, pRenderer, pGraphicsObjectManager, pResourceManager, pInputContext);

		if (result != RC_OK)
		{
			delete pImGUIContextInstance;

			pImGUIContextInstance = nullptr;
		}

		return dynamic_cast<IImGUIContext*>(pImGUIContextInstance);
	}
}