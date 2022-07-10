#include "../include/CImGUIContext.h"
#include "../deps/imgui-1.85/imgui.h"
#include "../deps/imgui-1.85/ImGuizmo.h"
#include <utils/Types.h>
#include <core/IWindowSystem.h>
#include <core/IGraphicsContext.h>
#include <core/IInputContext.h>
#include <core/IResourceManager.h>
#include <graphics/CBaseTexture2D.h>
#include <graphics/IGraphicsObjectManager.h>
#include <graphics/IIndexBuffer.h>
#include <graphics/IVertexBuffer.h>
#include <graphics/CBaseMaterial.h>
#include <graphics/IRenderer.h>
#include <graphics/CRenderQueue.h>
#include <graphics/IVertexDeclaration.h>
#include <graphics/IDebugUtility.h>
#include <platform/win32/CWin32WindowSystem.h>
#include <platform/unix/CUnixWindowSystem.h>
#include <utils/CFileLogger.h>
#include <utils/CGradientColor.h>
#include <math/MathUtils.h>
#include <math/TQuaternion.h>
#include <vector>
#include <cstring>
#define DEFER_IMPLEMENTATION
#include "deferOperation.hpp"
#define META_IMPLEMENTATION
#include "metadata.h"


namespace TDEngine2
{
	CImGUIContext::CImGUIContext():
		CBaseObject()
	{
	}

	E_RESULT_CODE CImGUIContext::Init(const TImGUIContextInitParams& params)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!params.mpWindowSystem ||
			!params.mpGraphicsObjectManager ||
			!params.mpResourceManager ||
			!params.mpRenderer ||
			!params.mpInputContext)
		{
			return RC_INVALID_ARGS;	
		}
		
		mpWindowSystem          = params.mpWindowSystem;
		mpGraphicsContext       = params.mpGraphicsObjectManager->GetGraphicsContext();
		mpGraphicsObjectManager = params.mpGraphicsObjectManager;
		mpResourceManager       = params.mpResourceManager;
		mpInputContext          = params.mpInputContext;
		mpEditorUIRenderQueue	= params.mpRenderer->GetRenderQueue(E_RENDER_QUEUE_GROUP::RQG_DEBUG);

		if (!mpGraphicsContext) // \note the really strange case, but if it's happened we should check for it
		{
			return RC_FAIL;
		}

		// \note Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		mpIOContext = &ImGui::GetIO();

		static std::string backendId = Meta::EnumTrait<E_GRAPHICS_CONTEXT_GAPI_TYPE>::ToString(mpGraphicsContext->GetContextInfo().mGapiType);
		mpIOContext->BackendRendererName = backendId.c_str();

		E_RESULT_CODE result = _initInternalImGUIContext(*mpIOContext);
		if (result != RC_OK)
		{
			return result;
		}
		
		// \note Setup Dear ImGui style
		ImGui::StyleColorsDark();

		ImGui::GetStyle().AntiAliasedLines = false;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CImGUIContext::ConfigureForWin32Platform(const CWin32WindowSystem* pWindowSystem)
	{
		if (!mpIOContext)
		{
			return RC_FAIL;
		}

#if defined(TDE2_USE_WINPLATFORM)
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

#if defined(TDE2_USE_UNIXPLATFORM)
		mpIOContext->BackendPlatformName = "UnixPlatform";

#endif

		return RC_OK;
	}

	void CImGUIContext::BeginFrame(F32 dt)
	{
		if (!mpIOContext)
		{
			return;
		}

		mpIOContext->DeltaTime = dt;

		// \note update current display size
		TRectU32&& windowRect = mpWindowSystem->GetClientRect();
		mpIOContext->DisplaySize = ImVec2(static_cast<F32>(windowRect.width), static_cast<F32>(windowRect.height));

		_updateInputState(*mpIOContext, mpInputContext.Get());

		ImGui::NewFrame();
		ImGuizmo::BeginFrame();

		ImGuizmo::SetRect(0, 0, mpIOContext->DisplaySize.x, mpIOContext->DisplaySize.y);

		mUsedResourcesRegistry.clear();
	}

	void CImGUIContext::EndFrame()
	{
		if (!mpIOContext)
		{
			return;
		}

		ImGui::Render();
		
		for (U8 i = 0; i < static_cast<U8>(ImGuiMouseButton_COUNT); ++i)
		{
			ImGui::ResetMouseDragDelta(i);
		}

		_engineInternalRender(ImGui::GetDrawData(), mpEditorUIRenderQueue);
	}

	E_ENGINE_SUBSYSTEM_TYPE CImGUIContext::GetType() const
	{
		return EST_IMGUI_CONTEXT;
	}

	void CImGUIContext::Label(const std::string& text)
	{
		ImGui::Text(text.c_str());
		_prepareLayout();
	}

	void CImGUIContext::Label(const std::string& text, const TColor32F& color)
	{
		ImGui::TextColored({ color.r, color.g, color.b, color.a }, text.c_str());
		_prepareLayout();
	}

	void CImGUIContext::Label(const std::string& text, const TVector2& pos, const TColor32F& color)
	{
		GetCurrActiveDrawList()->AddText(pos, PackColor32F(color), &text.front(), &text.front() + text.length());
		_prepareLayout();
	}

	bool CImGUIContext::Button(const std::string& text, const TVector2& sizes, const TImGUIContextAction& onClicked, bool makeInvisible, bool allowOverlapping)
	{
		const bool result = makeInvisible ? ImGui::InvisibleButton(text.c_str(), sizes) : ImGui::Button(text.c_str(), sizes);

		if (result && onClicked)
		{
			onClicked();
		}

		if (allowOverlapping) 
		{
			ImGui::SetItemAllowOverlap();
		}

		_prepareLayout();

		return result;
	}

	bool CImGUIContext::Checkbox(const std::string& text, bool& isSelected)
	{
		bool result = ImGui::Checkbox(text.c_str(), &isSelected);
		_prepareLayout();

		return result;
	}

	void CImGUIContext::IntSlider(const std::string& text, I32& value, I32 minValue, I32 maxValue,
								  const TImGUIContextAction& onValueChanged)
	{		
		if (ImGui::SliderInt(text.c_str(), &value, minValue, maxValue) && onValueChanged)
		{
			onValueChanged();
		}

		_prepareLayout();
	}

	void CImGUIContext::FloatSlider(const std::string& text, F32& value, F32 minValue, F32 maxValue,
									const TImGUIContextAction& onValueChanged)
	{
		if (ImGui::SliderFloat(text.c_str(), &value, minValue, maxValue) && onValueChanged)
		{
			onValueChanged();
		}

		_prepareLayout();
	}

	void CImGUIContext::FloatField(const std::string& text, F32& value, const TImGUIContextAction& onValueChanged)
	{
		if (ImGui::InputFloat(text.c_str(), &value, 0.0f, 0.0f, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue) && onValueChanged)
		{
			onValueChanged();
		}

		_prepareLayout();
	}

	void CImGUIContext::IntField(const std::string& text, I32& value, const TImGUIContextAction& onValueChanged)
	{
		if (ImGui::InputInt(text.c_str(), &value, 1, 100, onValueChanged ? ImGuiInputTextFlags_EnterReturnsTrue : 0x0) && onValueChanged)
		{
			onValueChanged();
		}

		_prepareLayout();
	}

	bool CImGUIContext::TextField(const std::string& text, std::string& value, const TImGUIContextParamAction<std::string>& onValueChanged, 
								const TImGUIContextAction& onCancel, bool setFocus)
	{
		C8 buffer[512]{ '\0' };
		memcpy(buffer, value.c_str(), value.size());

		bool hasValueChanged = ImGui::InputText(text.c_str(), buffer, sizeof(buffer), onValueChanged ? ImGuiInputTextFlags_EnterReturnsTrue : ImGuiInputTextFlags_None);

		value.assign(buffer);

		if (hasValueChanged && onValueChanged)
		{
			onValueChanged(value);

			if (setFocus)
			{
				ImGui::SetKeyboardFocusHere(-1);
			}
		}

		if (onCancel && ImGui::IsItemDeactivated() && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)))
		{
			onCancel();
		}

		_prepareLayout();

		if (setFocus)
		{
			ImGui::SetItemDefaultFocus();
			ImGui::SetKeyboardFocusHere();
		}

		return hasValueChanged;
	}

	void CImGUIContext::Vector2Field(const std::string& text, TVector2& value, const TImGUIContextAction& onValueChanged)
	{
		F32 rawValue[2] { value.x, value.y };

		if (ImGui::InputFloat2(text.c_str(), rawValue, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue) && onValueChanged)
		{
			value = TVector2(rawValue);
			onValueChanged();
		}

		_prepareLayout();
	}

	void CImGUIContext::Vector3Field(const std::string& text, TVector3& value, const TImGUIContextAction& onValueChanged)
	{
		F32 rawValue[3] { value.x, value.y, value.z };
		
		if (ImGui::InputFloat3(text.c_str(), rawValue, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue) && onValueChanged)
		{
			value = TVector3(rawValue);
			onValueChanged();
		}
		
		_prepareLayout();
	}

	void CImGUIContext::Vector4Field(const std::string& text, TVector4& value, const TImGUIContextAction& onValueChanged)
	{
		F32 rawValue[4]{ value.x, value.y, value.z, value.w };

		if (ImGui::InputFloat4(text.c_str(), rawValue, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue) && onValueChanged)
		{
			value = TVector4(rawValue);
			onValueChanged();
		}

		_prepareLayout();
	}

	void CImGUIContext::ColorPickerField(const std::string& text, TColor32F& color, const TImGUIContextAction& onValueChanged)
	{
		F32 rawValue[4]{ color.r, color.g, color.b, color.a };

		if (ImGui::ColorEdit4(text.c_str(), rawValue, ImGuiInputTextFlags_EnterReturnsTrue) && onValueChanged)
		{
			memcpy(&color, rawValue, sizeof(rawValue));
			onValueChanged();
		}

		_prepareLayout();
	}

	   
	static void DrawGradientColorRamp(CImGUIContext* pImGUIContext, CGradientColor& color, const TRectF32& rect)
	{
		const U32 packedWhiteColor = PackABGRColor32F(TColorUtils::mWhite);

		auto pDrawList = pImGUIContext->GetCurrActiveDrawList();

		const TVector2 origin = rect.GetLeftBottom();
		const TVector2 sizes = rect.GetSizes();

		pDrawList->AddRectFilled(origin, origin + sizes, packedWhiteColor);

		for (auto it = color.begin(); it != color.end() - 1; ++it)
		{
			const auto& currColorSample = *it;
			const auto& nextColorSample = *(it + 1);

			const U32 leftColor = PackABGRColor32F(std::get<TColor32F>(currColorSample));
			const U32 rightColor = PackABGRColor32F(std::get<TColor32F>(nextColorSample));

			const F32 currColorTime = std::get<F32>(currColorSample);
			const F32 nextColorTime = std::get<F32>(nextColorSample);

			pDrawList->AddRectFilledMultiColor(origin + TVector2(currColorTime * sizes.x, 0.0f), origin + TVector2(nextColorTime * sizes.x, sizes.y), leftColor, rightColor, rightColor, leftColor);
		}
	}

	/// The method returns true if a user clicks over a preview button, false in other cases
	static bool DrawGradientColorPreview(CImGUIContext* pImGUIContext, const std::string& text, CGradientColor& color, const TVector2& sizes)
	{
		if (pImGUIContext->BeginChildWindow(text, sizes))
		{
			pImGUIContext->BeginHorizontal();
			pImGUIContext->Label(text);

			const TVector2 cursorPos = pImGUIContext->GetCursorScreenPos();
			DrawGradientColorRamp(pImGUIContext, color, { cursorPos.x, cursorPos.y, pImGUIContext->GetWindowWidth(), pImGUIContext->GetWindowHeight() });

			pImGUIContext->EndHorizontal();
		}

		pImGUIContext->EndChildWindow();

		return ImGui::IsItemClicked();
	}

	static void DrawGradientColorMarkers(CImGUIContext* pImGUIContext, CGradientColor& color, const TRectF32& rect, const TVector2& markerSizes)
	{
		enum class E_GRADIENT_COLOR_WINDOW_PROPS : U32
		{
			SELECTED_MARKER_INDEX = 0x42f,
		};

		TVector2 origin = rect.GetLeftBottom();
		const TVector2 sizes = rect.GetSizes();

		origin = origin + TVector2(0.0f, 1.2f * sizes.y);

		auto pWindowStorage = ImGui::GetStateStorage();

		U32 selectedMarkerIndex = static_cast<U32>(pWindowStorage->GetInt(static_cast<ImGuiID>(E_GRADIENT_COLOR_WINDOW_PROPS::SELECTED_MARKER_INDEX)));

		TVector2 dragVector;

		/// \note Create an area for creation of new points
		if (ImGui::IsMouseDoubleClicked(0))
		{
			const float t = (ImGui::GetMousePos().x - origin.x) / sizes.x;
			color.AddPoint({ t, color.Sample(t) }); /// \note For a new point use interpolated color's value 
		}

		auto pDrawList = pImGUIContext->GetCurrActiveDrawList();

		for (auto it = color.begin(); it != color.end(); ++it)
		{
			const auto& currColorSample = *it;

			const TVector2 pos = origin + TVector2(std::get<F32>(currColorSample) * sizes.x, 0.0f);

			pDrawList->AddTriangleFilled(pos, pos + TVector2(10.0f), pos + TVector2(-10.0f, 10.0f), PackABGRColor32F(TColorUtils::mWhite));

			ImGui::SetCursorScreenPos(pos - TVector2(markerSizes.x * 0.5f, 0.0f));
			ImGui::PushID(static_cast<I32>(std::distance(color.begin(), it)));
			ImGui::InvisibleButton("", markerSizes);
			ImGui::PopID();

			if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
			{
				selectedMarkerIndex = static_cast<U32>(std::distance(color.begin(), it));
			}

			if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) // \note Provide deletion of points by right mouse click
			{
				const U32 deletedMarkerIndex = static_cast<U32>(std::distance(color.begin(), it));
				color.RemovePoint(deletedMarkerIndex);

				if (selectedMarkerIndex == deletedMarkerIndex)
				{
					--selectedMarkerIndex;
				}

				pWindowStorage->SetInt(static_cast<U32>(E_GRADIENT_COLOR_WINDOW_PROPS::SELECTED_MARKER_INDEX), selectedMarkerIndex);

				return;
			}

			if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
			{
				dragVector = ImGui::GetIO().MouseDelta.x;
			}

			if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
			{
				color.SortPoints();
			}
		}

		F32 currEditableColor[4];

		auto colorSample = color.GetPoint(selectedMarkerIndex);

		TColor32F& selectedMarkerColor = std::get<TColor32F>(*colorSample);

		currEditableColor[0] = selectedMarkerColor.r;
		currEditableColor[1] = selectedMarkerColor.g;
		currEditableColor[2] = selectedMarkerColor.b;
		currEditableColor[3] = selectedMarkerColor.a;

		/// \note Draw color picker
		pImGUIContext->SetCursorScreenPos(origin + TVector2(0.0f, 25.0f));
		ImGui::ColorEdit4("##editableColor", currEditableColor, ImGuiInputTextFlags_EnterReturnsTrue);

		color.SetColor(selectedMarkerIndex, std::get<F32>(*colorSample) + dragVector.x / sizes.x, TColor32F{ currEditableColor });

		pWindowStorage->SetInt(static_cast<U32>(E_GRADIENT_COLOR_WINDOW_PROPS::SELECTED_MARKER_INDEX), selectedMarkerIndex);
	}

	static bool DrawGradientColorEditor(CImGUIContext* pImGUIContext, CGradientColor& color, const TVector2& windowSizes, bool isOpened)
	{
		constexpr F32 pickerHeight = 50.0f;
		const TVector2 markerSizes{ 20.0f, 20.0f };

		const IImGUIContext::TWindowParams params
		{
			windowSizes,
			windowSizes,
			windowSizes
		};

		if (pImGUIContext->BeginWindow("GradientColorEditor", isOpened, params))
		{
			pImGUIContext->SetCursorScreenPos(pImGUIContext->GetCursorScreenPos() + TVector2(5.0f, 0.0f));
			TVector2 cursorPos = pImGUIContext->GetCursorScreenPos();

			const TRectF32 pickerRect { cursorPos.x, cursorPos.y, pImGUIContext->GetWindowWidth() - 25.0f, pickerHeight };

			DrawGradientColorRamp(pImGUIContext, color, pickerRect);
			DrawGradientColorMarkers(pImGUIContext, color, pickerRect, markerSizes);

			pImGUIContext->EndWindow();
		}

		return isOpened;
	}


	void CImGUIContext::GradientColorPicker(const std::string& text, CGradientColor& color, const TImGUIContextAction& onValueChanged)
	{
		constexpr F32 gradientColorPickerHeight = 25.0f;

		if (DrawGradientColorPreview(this, text, color, { GetWindowWidth(), gradientColorPickerHeight })) // \note Open full editor if a user clicks over a preview
		{
			mIsGradientColorEditorOpened = !mIsGradientColorEditorOpened;
		}

		if (mIsGradientColorEditorOpened)
		{
			mIsGradientColorEditorOpened = DrawGradientColorEditor(this, color, TVector2(350.0f, 150.0f), mIsGradientColorEditorOpened);
		}
	}

	void CImGUIContext::DisplayMainMenu(const std::function<void(IImGUIContext&)>& onDrawCallback)
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (onDrawCallback)
			{
				onDrawCallback(*this);
			}
		}

		ImGui::EndMainMenuBar();
	}

	void CImGUIContext::DisplayContextMenu(const std::string& id, const std::function<void(IImGUIContext&)>& onDrawCallback)
	{
		if (ImGui::BeginPopupContextItem(id.empty() ? nullptr : id.c_str()))
		{
			if (onDrawCallback)
			{
				onDrawCallback(*this);
			}

			ImGui::EndPopup();
		}
	}

	void CImGUIContext::MenuGroup(const std::string& name, const std::function<void(IImGUIContext&)>& onDrawCallback)
	{
		if (ImGui::BeginMenu(name.c_str()))
		{
			if (onDrawCallback)
			{
				onDrawCallback(*this);
			}

			ImGui::EndMenu();
		}
	}
	
	bool CImGUIContext::MenuItem(const std::string& name, const std::string& shortcut, const TImGUIContextAction& onClicked)
	{
		if (ImGui::MenuItem(name.c_str(), shortcut.c_str()))
		{
			if (onClicked)
			{
				onClicked();
			}

			return true;
		}

		return false;
	}

	void CImGUIContext::Histogram(const std::string& name, const std::vector<F32>& values, F32 minScale, F32 maxScale, const TVector2& sizes, const std::string& overlayedText)
	{
		ImGui::PlotHistogram(name.c_str(), &values.front(), static_cast<I32>(values.size()), 0, overlayedText.c_str(), minScale, maxScale, ImVec2(sizes.x, sizes.y));
	}

	void CImGUIContext::VerticalSeparator(F32 initialLeftColumnWidth, const std::function<void(F32)>& leftRegionCallback, const std::function<void(F32)>& rightRegionCallback)
	{
		if (ImGui::BeginTable("##vert_separated", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp))
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::SetNextItemWidth(initialLeftColumnWidth);

			if (leftRegionCallback)
			{
				leftRegionCallback(ImGui::GetContentRegionAvail().x);
			}

			ImGui::TableSetColumnIndex(1);

			if (rightRegionCallback)
			{
				rightRegionCallback(ImGui::GetContentRegionAvail().x);
			}

			ImGui::EndTable();
		}
	}

	void CImGUIContext::Histogram(const std::string& name, const F32* pValues, U32 valuesCount, F32 minScale, F32 maxScale,
								  const TVector2& sizes, const std::string& overlayedText)
	{
		ImGui::PlotHistogram(name.c_str(), pValues, valuesCount, 0, overlayedText.c_str(), minScale, maxScale, ImVec2(sizes.x, sizes.y));
	}

	void CImGUIContext::DrawLine(const TVector2& start, const TVector2& end, const TColor32F& color, F32 thickness)
	{
		GetCurrActiveDrawList()->AddLine(start, end, PackABGRColor32F(color), thickness);
	}

	void CImGUIContext::DrawCubicBezier(const TVector2& p0, const TVector2& t0, const TVector2& p1, const TVector2& t1, const TColor32F& color, F32 thickness)
	{
		GetCurrActiveDrawList()->AddBezierCubic(p0, t0, t1, p1, PackABGRColor32F(color), thickness);
	}

	void CImGUIContext::DrawRect(const TRectF32& rect, const TColor32F& color, bool isFilled, F32 thickness)
	{
		if (isFilled)
		{
			GetCurrActiveDrawList()->AddRectFilled(ImVec2(rect.x, rect.y), ImVec2(rect.x + rect.width, rect.y + rect.height), PackABGRColor32F(color));
			return;
		}

		GetCurrActiveDrawList()->AddRect(ImVec2(rect.x, rect.y), ImVec2(rect.x + rect.width, rect.y + rect.height), PackABGRColor32F(color), 0.0f, 15, thickness);
	}

	void CImGUIContext::DrawCircle(const TVector2& center, F32 radius, bool isFilled, const TColor32F& color, F32 thickness)
	{
		if (isFilled)
		{
			GetCurrActiveDrawList()->AddCircleFilled(center, radius, PackABGRColor32F(color), 12);
			return;
		}

		GetCurrActiveDrawList()->AddCircle(center, radius, PackABGRColor32F(color), 12, thickness);
	}

	void CImGUIContext::DrawTriangle(const TVector2& p0, const TVector2& p1, const TVector2& p2, const TColor32F& color, bool isFilled, F32 thickness)
	{
		if (isFilled)
		{
			GetCurrActiveDrawList()->AddTriangleFilled(p0, p1, p2, PackABGRColor32F(color));
			return;
		}
		
		GetCurrActiveDrawList()->AddTriangle(p0, p1, p2, PackABGRColor32F(color), thickness);
	}

	void CImGUIContext::DrawText(const TVector2& pos, const TColor32F& color, const std::string& text)
	{
		GetCurrActiveDrawList()->AddText(pos, PackABGRColor32F(color), text.c_str());
	}

	bool CImGUIContext::DrawGizmo(E_GIZMO_TYPE type, const TMatrix4& view, const TMatrix4& proj, const TMatrix4& transform,
									const std::function<void(const TVector3&, const TQuaternion&, const TVector3)>& onUpdate)
	{
		F32 mat[16];
		std::memcpy(mat, transform.arr, sizeof(mat));

		ImGuizmo::OPERATION gizmoType = ImGuizmo::OPERATION::TRANSLATE;

		switch (type)
		{
			case E_GIZMO_TYPE::TRANSLATION:
				gizmoType = ImGuizmo::OPERATION::TRANSLATE;
				break;
			case E_GIZMO_TYPE::ROTATION:
				gizmoType = ImGuizmo::OPERATION::ROTATE;
				break;
			case E_GIZMO_TYPE::SCALING:
				gizmoType = ImGuizmo::OPERATION::SCALE;
				break;
			default:
				TDE2_UNIMPLEMENTED();
				break;
		}
		
		if (ImGuizmo::Manipulate(view.arr, proj.arr, gizmoType, ImGuizmo::MODE::WORLD, mat))
		{
			F32 position[3] { 0.0f };
			F32 rotation[3] { 0.0f };
			F32 scale[3] { 0.0f };
			
			ImGuizmo::DecomposeMatrixToComponents(mat, position, rotation, scale);

			if (onUpdate)
			{
				onUpdate(TVector3(position), TQuaternion(TVector3(rotation)), TVector3(scale));
			}

			return true;
		}

		return false;
	}

	void CImGUIContext::DrawPlotGrid(const std::string& name, const TPlotGridParams& params, const std::function<void(const TVector2&)>& onGridCallback)
	{
		const F32 width = params.mWidth;
		const F32 height = params.mHeight;

		BeginChildWindow(name.c_str(), { width, height });

		const TVector2 pos = GetCursorScreenPos();

		if (params.mIsBackgroundEnabled)
		{
			DrawRect({ pos.x, pos.y, width, height }, params.mBackgroundColor);
		}

		// draw grid
		if (params.mIsGridEnabled)
		{
			const F32 xStep = width / static_cast<F32>(params.mColsCount);
			const F32 xFrameStep = params.mFrame.width / static_cast<F32>(params.mColsCount);

			for (U8 i = 0; i < params.mColsCount; ++i)
			{
				const TVector2& p0 = pos + (i * xStep) * RightVector2;

				DrawLine(p0, p0 + TVector2(0.0f, height), params.mAxesColor);

				const F32 currCoord = params.mFrame.x + i * xFrameStep;

				DrawText(pos + (i * xStep + 10.f) * RightVector2 + TVector2(0.0f, height - 20.0f), params.mAxesColor, Wrench::StringUtils::Format("{0}", currCoord));
			}

			const F32 yStep = height / static_cast<F32>(params.mRowsCount);
			const F32 yFrameStep = params.mFrame.height / static_cast<F32>(params.mRowsCount);
			const F32 yMaxLimit = params.mFrame.height + params.mFrame.y;

			for (U8 i = 0; i < params.mRowsCount; ++i)
			{
				const TVector2& p0 = pos + (i * yStep) * UpVector2;

				const F32 currCoord = yMaxLimit - i * yFrameStep;

				DrawLine(p0, p0 + TVector2(width, 0.0f), params.mAxesColor);
				DrawText(pos + (i * yStep + 10.f) * UpVector2, params.mAxesColor, Wrench::StringUtils::Format("{0}", currCoord));
			}
		}

		if (onGridCallback)
		{
			onGridCallback(GetCursorScreenPos());
		}

		EndChildWindow();
	}

	void CImGUIContext::DisplayIDGroup(I32 id, const TImGUIContextAction& idGroupCallback)
	{
		ImGui::PushID(id);

		if (idGroupCallback)
		{
			idGroupCallback();
		}

		ImGui::PopID();
	}

	I32 CImGUIContext::Popup(const std::string& label, I32 currSelectedItem, const std::vector<std::string>& items)
	{
		I32 index = currSelectedItem;

		if (index >= static_cast<I32>(items.size()))
		{
			return -1;
		}

		if (ImGui::BeginCombo(label.c_str(), items[index].c_str()))
		{
			for (I32 i = 0; i < static_cast<I32>(items.size()); ++i)
			{
				if (ImGui::Selectable(items[i].c_str()))
				{
					index = i;
				}
			}

			ImGui::EndCombo();
		}

		return index;
	}

	void CImGUIContext::Image(TResourceId textureHandle, const TVector2& sizes, const TRectF32& uvRect)
	{
		mUsedResourcesRegistry.push_back(textureHandle);

		ImGui::Image(static_cast<ImTextureID>(&mUsedResourcesRegistry.back()), sizes, ImVec2(uvRect.x, uvRect.y), ImVec2(uvRect.width, uvRect.height));

		_prepareLayout();
	}

	bool CImGUIContext::SelectableItem(const std::string& id, bool isSelected, bool shouldClosePopup)
	{
		const bool result = ImGui::Selectable(id.c_str(), isSelected, shouldClosePopup ? 0x0 : ImGuiSelectableFlags_DontClosePopups);
		_prepareLayout();

		return result;
	}

	bool CImGUIContext::SelectableItem(const std::string& id, const TColor32F& color, bool isSelected, bool shouldClosePopup)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(color.r, color.g, color.b, color.a));
		const bool result = SelectableItem(id, isSelected, shouldClosePopup);
		ImGui::PopStyleColor();

		return result;
	}

	std::tuple<bool, bool> CImGUIContext::BeginTreeNode(const std::string& id, bool isSelected)
	{
		auto&& result = std::tuple<bool, bool>{ ImGui::TreeNodeEx(id.c_str(), isSelected ? ImGuiTreeNodeFlags_Selected : 0x0), ImGui::IsItemClicked() };
		_prepareLayout();

		return result;
	}

	void CImGUIContext::EndTreeNode()
	{
		ImGui::TreePop();
	}

	bool CImGUIContext::CollapsingHeader(const std::string& id, bool isOpened, bool isSelected, const TImGUIContextAction& itemClicked, const TImGUIContextAction& contentAction)
	{
		if (!ImGui::CollapsingHeader(id.c_str(), ImGuiTreeNodeFlags_DefaultOpen | (isSelected ? ImGuiTreeNodeFlags_Selected : 0x0)))
		{
			return false;
		}

		if (ImGui::IsItemClicked() && itemClicked)
		{
			itemClicked();
		}

		if (contentAction)
		{
			contentAction();
		}

		return true;
	}

	bool CImGUIContext::BeginWindow(const std::string& name, bool& isOpened, const TWindowParams& params)
	{
		ImGuiWindowFlags flags = 0x0;
		flags |= (params.mIsAutoResizeable ? ImGuiWindowFlags_AlwaysAutoResize : 0x0);

		if (params.mMaxSizes != ZeroVector2)
		{
			TVector2 minSizes = params.mMinSizes;
			TVector2 maxSizes = params.mMaxSizes;

			ImGui::SetNextWindowSizeConstraints(ImVec2(minSizes.x, minSizes.y), ImVec2(maxSizes.x, maxSizes.y));
		}

		if (params.mSizes != ZeroVector2)
		{
			TVector2 sizes = params.mSizes;
			ImGui::SetNextWindowSize(ImVec2(sizes.x, sizes.y));
		}

		flags |= (params.mIsInputTransparent ? (ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMouseInputs) : 0x0);

		if (params.mIsFullscreen)
		{
			flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar |
					ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
					ImGuiWindowFlags_NoBringToFrontOnFocus;

			ImGui::SetNextWindowSize(mpIOContext->DisplaySize);
			ImGui::SetNextWindowPos(ImVec2(0, 0));
		}

		if (params.mIsTransparent)
		{
			ImGui::SetNextWindowBgAlpha(0.0f);
		}

		bool result = ImGui::Begin(name.c_str(), &isOpened, flags);

		mpDrawListsContext.push(ImGui::GetWindowDrawList());

		return result;
	}

	void CImGUIContext::EndWindow()
	{
		mpDrawListsContext.pop();
		ImGui::End();
	}

	void CImGUIContext::BeginHorizontal()
	{
		mIsHorizontalGroupEnabled = true;
	}

	void CImGUIContext::EndHorizontal()
	{
		TDE2_ASSERT(mIsHorizontalGroupEnabled);
		
		mIsHorizontalGroupEnabled = false;
		ImGui::NewLine();
	}

	bool CImGUIContext::BeginChildWindow(const std::string& name, const TVector2& sizes)
	{
		bool result = ImGui::BeginChild(name.c_str(), ImVec2(sizes.x, sizes.y));

		mpDrawListsContext.push(ImGui::GetWindowDrawList());

		return result;
	}

	void CImGUIContext::EndChildWindow()
	{
		mpDrawListsContext.pop();
		ImGui::EndChild();
	}

	void CImGUIContext::ShowModalWindow(const std::string& name)
	{
		ImGui::OpenPopup(name.c_str());
	}

	void CImGUIContext::CloseCurrentModalWindow()
	{
		ImGui::CloseCurrentPopup();
	}

	bool CImGUIContext::BeginModalWindow(const std::string& name, bool isNonBlocking)
	{
		return isNonBlocking ? ImGui::BeginPopupContextItem(name.c_str()) : ImGui::BeginPopupModal(name.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	}

	void CImGUIContext::EndModalWindow()
	{
		ImGui::EndPopup();
	}

	void CImGUIContext::Tooltip(const std::string& message)
	{
		if (!ImGui::IsItemHovered())
		{
			return;
		}

		ImGui::BeginTooltip();
		ImGui::SetTooltip(message.c_str());
		ImGui::EndTooltip();
	}

	void CImGUIContext::SetCursorScreenPos(const TVector2& pos)
	{
		ImGui::SetCursorScreenPos(pos);
	}

	void CImGUIContext::SetItemWidth(F32 width, const TImGUIContextAction& action)
	{
		ImGui::PushItemWidth(width);

		if (action)
		{
			action();
		}

		ImGui::PopItemWidth();
	}

	void CImGUIContext::RegisterDragAndDropSource(const TImGUIContextAction& action)
	{
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
		{
			if (action)
			{
				action();
			}

			ImGui::EndDragDropSource();
		}
	}

	void CImGUIContext::RegisterDragAndDropTarget(const TImGUIContextAction& action)
	{
		if (ImGui::BeginDragDropTarget())
		{
			if (action)
			{
				action();
			}

			ImGui::EndDragDropTarget();
		}
	}

	F32 CImGUIContext::GetWindowWidth() const
	{
		return ImGui::GetWindowWidth();
	}

	F32 CImGUIContext::GetWindowHeight() const
	{
		return ImGui::GetWindowHeight();
	}

	TVector2 CImGUIContext::GetCursorScreenPos() const
	{
		return ImGui::GetCursorScreenPos();
	}

	TVector2 CImGUIContext::GetTextSizes(const std::string& text) const
	{
		return ImGui::CalcTextSize(&text.front(), &text.front() + text.length());
	}

	bool CImGUIContext::IsItemActive() const
	{
		return ImGui::IsItemActive();
	}
		
	bool CImGUIContext::IsMouseOverUI() const
	{
		return mpIOContext->WantCaptureMouse;
	}

	bool CImGUIContext::IsItemHovered() const
	{
		return ImGui::IsItemHovered();
	}

	bool CImGUIContext::IsMouseDoubleClicked(U8 buttonId) const
	{
		return ImGui::IsMouseDoubleClicked(static_cast<I32>(buttonId));
	}

	bool CImGUIContext::IsMouseClicked(U8 buttonId) const
	{
		return ImGui::IsMouseClicked(static_cast<I32>(buttonId));
	}

	bool CImGUIContext::IsMouseReleased(U8 buttonId) const
	{
		return ImGui::IsMouseReleased(static_cast<I32>(buttonId));
	}

	bool CImGUIContext::IsMouseDragging(U8 buttonId) const
	{
		return ImGui::IsMouseDragging(static_cast<I32>(buttonId));
	}

	TVector2 CImGUIContext::GetMousePosition() const
	{
		return mpIOContext->MousePos;
	}

	TVector2 CImGUIContext::GetInvertedMousePosition() const
	{
		return TVector2(mpIOContext->MousePos.x, -(mpIOContext->MousePos.y - mpIOContext->DisplaySize.y));
	}

	TVector2 CImGUIContext::GetScrollPosition() const
	{
		return TVector2(ImGui::GetScrollX(), ImGui::GetScrollY());
	}

	TVector2 CImGUIContext::GetMouseDragDelta(U8 buttonId) const
	{
		return ImGui::GetMouseDragDelta(static_cast<I32>(buttonId));
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
		if ((result = _initGraphicsResources(io, mpGraphicsContext, mpGraphicsObjectManager, mpResourceManager.Get())) != RC_OK)
		{
			return result;
		}

		_initInputMappings(io);

		return RC_OK;
	}

	
	static constexpr USIZE VertexBufferChunkSize = 8192 * sizeof(ImDrawVert);
	static constexpr USIZE IndexBufferChunkSize = 65536 * sizeof(U16);


	void CImGUIContext::_updateInputState(ImGuiIO& io, IInputContext* pInputContext)
	{
		IDesktopInputContext* pDesktopInputCtx = dynamic_cast<IDesktopInputContext*>(mpInputContext.Get());

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

		for (U16 keyCode = static_cast<U16>(E_KEYCODES::KC_ESCAPE); keyCode != static_cast<U16>(E_KEYCODES::KC_NONE); ++keyCode)
		{
			const E_KEYCODES internalKeyCode = static_cast<E_KEYCODES>(keyCode);
			
			if (IsAlphaNumericKeyCode(internalKeyCode))
			{
				continue;
			}

			io.KeysDown[keyCode] = pDesktopInputCtx->IsKey(internalKeyCode);
		}

#if TDE2_EDITORS_ENABLED
		pDesktopInputCtx->SetOnCharInputCallback([this, &io](TUtf8CodePoint characterCode)
		{
			io.AddInputCharactersUTF8(reinterpret_cast<const C8*>(&characterCode));
		});
#endif

		io.MouseWheel += CMathUtils::Clamp(-1.0f, 1.0f, pDesktopInputCtx->GetMouseShiftVec().z);
		// \todo Implement support of gamepads
	}

	E_RESULT_CODE CImGUIContext::_initGraphicsResources(ImGuiIO& io, IGraphicsContext* pGraphicsContext, IGraphicsObjectManager* pGraphicsManager,
														IResourceManager* pResourceManager)
	{
		io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

		E_RESULT_CODE result = RC_OK;

		auto vertexBufferResult = pGraphicsManager->CreateVertexBuffer(BUT_DYNAMIC, VertexBufferChunkSize, nullptr);
		if (vertexBufferResult.HasError())
		{
			return vertexBufferResult.GetError();
		}

		mpVertexBuffer = vertexBufferResult.Get();

		auto indexBufferResult = pGraphicsManager->CreateIndexBuffer(BUT_DYNAMIC, IFT_INDEX16, IndexBufferChunkSize, nullptr);
		if (indexBufferResult.HasError())
		{
			return indexBufferResult.GetError();
		}

		mpIndexBuffer = indexBufferResult.Get();

		mpEditorUIVertexDeclaration = pGraphicsManager->CreateVertexDeclaration().Get();
		mpEditorUIVertexDeclaration->AddElement({ TDEngine2::FT_FLOAT4, 0, TDEngine2::VEST_POSITION, false });
		mpEditorUIVertexDeclaration->AddElement({ TDEngine2::FT_NORM_UBYTE4, 0, TDEngine2::VEST_COLOR, false });

		// \note load default editor's material (depth test and writing to the depth buffer are disabled)
		TMaterialParameters editorUIMaterialParams { "DefaultEditorUI", true, { false, false } };

		auto& blendingParams = editorUIMaterialParams.mBlendingParams;
		blendingParams.mScrValue       = E_BLEND_FACTOR_VALUE::SOURCE_ALPHA;
		blendingParams.mDestValue      = E_BLEND_FACTOR_VALUE::ONE_MINUS_SOURCE_ALPHA;
		blendingParams.mScrAlphaValue  = E_BLEND_FACTOR_VALUE::ONE_MINUS_SOURCE_ALPHA;
		blendingParams.mDestAlphaValue = E_BLEND_FACTOR_VALUE::ZERO;

		auto& rasterizerParams = editorUIMaterialParams.mRasterizerParams;
		rasterizerParams.mIsScissorTestEnabled = true;

		mDefaultEditorMaterialHandle = pResourceManager->Create<IMaterial>("DefaultEditorUIMaterial.material", editorUIMaterialParams);

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

		mFontTextureHandle = mpResourceManager->Create<ITexture2D>("imgui_defaultfont_texatlas",
																		 TTexture2DParameters
																		 {
																			 static_cast<U32>(width),
																			 static_cast<U32>(height),
																			 FT_NORM_UBYTE4, 1, 1, 0
																		 });

		if (TResourceId::Invalid == mFontTextureHandle)
		{
			return RC_FAIL;
		}

		auto pRawTextureResource = mpResourceManager->GetResource<ITexture2D>(mFontTextureHandle);
		if (!pRawTextureResource)
		{
			return RC_FAIL;
		}

		io.Fonts->TexID = static_cast<ImTextureID>(&mFontTextureHandle);
		
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
				pCurrIndexPtr  += pCommandList->IdxBuffer.Size;
			}

			if (vertices.size() > mpVertexBuffer->GetSize() / sizeof(ImDrawVert))
			{
				mpVertexBuffer = mpGraphicsObjectManager->CreateVertexBuffer(E_BUFFER_USAGE_TYPE::BUT_DYNAMIC, mpVertexBuffer->GetSize() + VertexBufferChunkSize, nullptr).Get();
				mpVertexBuffer->Map(BMT_WRITE_DISCARD);
			}

			if (indices.size() > mpIndexBuffer->GetSize() / sizeof(ImDrawIdx))
			{
				mpIndexBuffer = mpGraphicsObjectManager->CreateIndexBuffer(E_BUFFER_USAGE_TYPE::BUT_DYNAMIC, E_INDEX_FORMAT_TYPE::IFT_INDEX16, mpIndexBuffer->GetSize() + IndexBufferChunkSize, nullptr).Get();
				mpIndexBuffer->Map(BMT_WRITE_DISCARD);
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

		TPtr<IMaterial> pMaterial = mpResourceManager->GetResource<IMaterial>(mDefaultEditorMaterialHandle);

		for (I32 n = 0; n < pImGUIData->CmdListsCount; ++n)
		{
			const ImDrawList* pCommandList = pImGUIData->CmdLists[n];

			for (I32 currCommandIndex = 0; currCommandIndex < pCommandList->CmdBuffer.Size; ++currCommandIndex)
			{
				const ImDrawCmd* pCurrCommand = &pCommandList->CmdBuffer[currCommandIndex];
				
				const TResourceId textureHandle = *static_cast<const TResourceId*>(pCurrCommand->TextureId);
				const U32 textureHandleHash = static_cast<U32>(textureHandle);

				if (mUsingMaterials.find(static_cast<U32>(textureHandle)) == mUsingMaterials.cend()) // \note create a new instance
				{
					mUsingMaterials.emplace(static_cast<U32>(textureHandle), pMaterial->CreateInstance()->GetInstanceId());
				}
				
				TPtr<ITexture> pTexture = mpResourceManager->GetResource<ITexture>(textureHandle);

				pMaterial->SetTextureResource("Texture", pTexture.Get(), mUsingMaterials[textureHandleHash]);

				TDrawIndexedCommand* pCurrDrawCommand = pRenderQueue->SubmitDrawCommand<TDrawIndexedCommand>((0xFFFFFFF0 - batchId));

				auto&& uvRect = pTexture ? pTexture->GetNormalizedTextureRect() : TRectF32{ 0.0f, 0.0f, 1.0f, 1.0f };

				pCurrDrawCommand->mObjectData.mModelMatrix = Transpose(projectionMatrix); // \note assign it as ModelMat and don't use global ProjMat
				pCurrDrawCommand->mObjectData.mTextureTransformDesc = TVector4(uvRect.x, uvRect.y, uvRect.width, uvRect.height);

				pCurrDrawCommand->mpVertexDeclaration      = mpEditorUIVertexDeclaration;
				pCurrDrawCommand->mpVertexBuffer           = mpVertexBuffer;
				pCurrDrawCommand->mpIndexBuffer            = mpIndexBuffer;
				pCurrDrawCommand->mMaterialHandle          = mDefaultEditorMaterialHandle;
				pCurrDrawCommand->mPrimitiveType           = E_PRIMITIVE_TOPOLOGY_TYPE::PTT_TRIANGLE_LIST;
				pCurrDrawCommand->mNumOfIndices            = pCurrCommand->ElemCount;
				pCurrDrawCommand->mStartIndex              = pCurrCommand->IdxOffset + currIndexOffset;
				pCurrDrawCommand->mStartVertex             = pCurrCommand->VtxOffset + currVertexOffset;
				pCurrDrawCommand->mMaterialInstanceId      = mUsingMaterials[textureHandleHash];

				const TVector2 clipMin(pCurrCommand->ClipRect.x - clipRect.x, pCurrCommand->ClipRect.y - clipRect.y);
				const TVector2 clipMax(pCurrCommand->ClipRect.z - clipRect.x, pCurrCommand->ClipRect.w - clipRect.y);
				
				if (clipMax.x < clipMin.x || clipMax.y < clipMin.y)
				{
					continue;
				}

				pCurrDrawCommand->mScissorRect = TRectU32(
					static_cast<U32>(clipMin.x), 
					static_cast<U32>(clipMin.y), 
					static_cast<U32>(CMathUtils::Abs(clipMax.x - clipMin.x)), 
					static_cast<U32>(CMathUtils::Abs(clipMax.y - clipMin.y)));

				++batchId;
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
		io.KeyMap[ImGuiKey_A]          = static_cast<I32>(E_KEYCODES::KC_A);
		io.KeyMap[ImGuiKey_C]          = static_cast<I32>(E_KEYCODES::KC_C);
		io.KeyMap[ImGuiKey_V]          = static_cast<I32>(E_KEYCODES::KC_V);
		io.KeyMap[ImGuiKey_X]          = static_cast<I32>(E_KEYCODES::KC_X);
		io.KeyMap[ImGuiKey_Y]          = static_cast<I32>(E_KEYCODES::KC_Y);
		io.KeyMap[ImGuiKey_Z]          = static_cast<I32>(E_KEYCODES::KC_Z);
	}

	void CImGUIContext::_prepareLayout()
	{
		if (mIsHorizontalGroupEnabled)
		{
			ImGui::SameLine();
		}
	}

	ImDrawList* CImGUIContext::GetCurrActiveDrawList() const
	{
		TDE2_ASSERT(!mpDrawListsContext.empty());
		return mpDrawListsContext.top();
	}

	void CImGUIContext::_setDragAndDropData(const std::string& id, const void* pData, U32 size)
	{
		ImGui::SetDragDropPayload(id.c_str(), pData, size);
	}
		
	const void* CImGUIContext::_getDragAndDropData(const std::string& id) const
	{
		if (const ImGuiPayload* pPayloadData = ImGui::AcceptDragDropPayload(id.c_str()))
		{
			return pPayloadData->Data;
		}

		return nullptr;
	}


	TDE2_API IImGUIContext* CreateImGUIContext(const TImGUIContextInitParams& params, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IImGUIContext, CImGUIContext, result, params);
	}
}