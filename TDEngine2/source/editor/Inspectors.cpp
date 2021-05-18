#include "../../include/editor/Inspectors.h"
#include "../../include/core/IImGUIContext.h"
#include "../../include/ecs/IComponent.h"
#include "../../include/ecs/CTransform.h"
#include "../../include/ecs/components/CBoundsComponent.h"
#include "../../include/ecs/IWorld.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/graphics/CQuadSprite.h"
#include "../../include/graphics/CStaticMeshContainer.h"
#include "../../include/graphics/effects/CParticleEmitterComponent.h"
#include "../../include/graphics/UI/CCanvasComponent.h"
#include "../../include/graphics/UI/CLayoutElementComponent.h"
#include "../../include/graphics/UI/CUIElementMeshDataComponent.h"
#include "../../include/graphics/UI/CImageComponent.h"
#include "../../include/physics/2D/CBoxCollisionObject2D.h"
#include "../../include/physics/2D/CCircleCollisionObject2D.h"
#include "../../include/physics/2D/CTrigger2D.h"
#include "../../include/physics/3D/CBoxCollisionObject3D.h"
#include "../../include/physics/3D/CSphereCollisionObject3D.h"
#include "../../include/physics/3D/CConvexHullCollisionObject3D.h"
#include "../../include/physics/3D/CTrigger3D.h"
#include "../../include/scene/components/ShadowMappingComponents.h"
#include "../../include/scene/components/CPointLight.h"
#include "../../include/scene/components/CDirectionalLight.h"
#include "../../include/scene/components/AudioComponents.h"
#include "../../include/editor/ecs/EditorComponents.h"
#include "../../include/editor/CLevelEditorWindow.h"
#include "../../include/editor/CEditorActionsManager.h"
#include "../../include/graphics/animation/CAnimationContainerComponent.h"
#include "../../include/utils/CFileLogger.h"
#include <array>
#include <tuple>
#include <functional>


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	E_RESULT_CODE CDefeaultInspectorsRegistry::RegisterBuiltinInspectors(CLevelEditorWindow& editor)
	{
		E_RESULT_CODE result = editor.RegisterInspector(CTransform::GetTypeId(), DrawTransformGUI);
		result = result | editor.RegisterInspector(CBoundsComponent::GetTypeId(), DrawBoundsGUI);

		/// Graphics
		result = result | editor.RegisterInspector(CQuadSprite::GetTypeId(), DrawQuadSpriteGUI);
		result = result | editor.RegisterInspector(CStaticMeshContainer::GetTypeId(), DrawStaticMeshContainerGUI);
		result = result | editor.RegisterInspector(CShadowReceiverComponent::GetTypeId(), DrawShadowReceiverGUI);
		result = result | editor.RegisterInspector(CShadowCasterComponent::GetTypeId(), DrawShadowCasterGUI);
		result = result | editor.RegisterInspector(CDirectionalLight::GetTypeId(), DrawDirectionalLightGUI);
		result = result | editor.RegisterInspector(CPointLight::GetTypeId(), DrawPointLightGUI);
		result = result | editor.RegisterInspector(CAnimationContainerComponent::GetTypeId(), DrawAnimationContainerGUI);
		result = result | editor.RegisterInspector(CSkyboxComponent::GetTypeId(), DrawSkyboxGUI);
		result = result | editor.RegisterInspector(CParticleEmitter::GetTypeId(), DrawParticleEmitterGUI);
		result = result | editor.RegisterInspector(CCanvas::GetTypeId(), DrawCanvasGUI);
		result = result | editor.RegisterInspector(CLayoutElement::GetTypeId(), DrawLayoutElementGUI);
		result = result | editor.RegisterInspector(CUIElementMeshData::GetTypeId(), DrawUIElementMeshDataGUI);
		result = result | editor.RegisterInspector(CImage::GetTypeId(), DrawImageGUI);

		/// 2D Physics
		result = result | editor.RegisterInspector(CBoxCollisionObject2D::GetTypeId(), DrawBoxCollision2DGUI);
		result = result | editor.RegisterInspector(CCircleCollisionObject2D::GetTypeId(), DrawCircleCollision2DGUI);
		result = result | editor.RegisterInspector(CTrigger2D::GetTypeId(), DrawTrigger2DGUI);

		/// 3D Physics
		result = result | editor.RegisterInspector(CBoxCollisionObject3D::GetTypeId(), DrawBoxCollision3DGUI);
		result = result | editor.RegisterInspector(CSphereCollisionObject3D::GetTypeId(), DrawSphereCollision3DGUI);
		result = result | editor.RegisterInspector(CConvexHullCollisionObject3D::GetTypeId(), DrawConvexHullCollision3DGUI);
		result = result | editor.RegisterInspector(CTrigger3D::GetTypeId(), DrawTrigger3DGUI);

		/// Audio
		result = result | editor.RegisterInspector(CAudioListenerComponent::GetTypeId(), DrawAudioListenerGUI);
		result = result | editor.RegisterInspector(CAudioSourceComponent::GetTypeId(), DrawAudioSourceGUI);

		result = result | editor.RegisterInspector(CSelectedEntityComponent::GetTypeId(), [](auto) { /* Do nothing for hidden components */ });

		return result;
	}

	void CDefeaultInspectorsRegistry::DrawTransformGUI(const TEditorContext& editorContext)
	{
		IImGUIContext& imguiContext = editorContext.mImGUIContext;
		IComponent& component = editorContext.mComponent;
		IEditorActionsHistory& actionsHistory = editorContext.mActionsHistory;

		if (imguiContext.CollapsingHeader("Transform", true, false))
		{
			CTransform& transform = dynamic_cast<CTransform&>(component);

			TVector3 position = transform.GetPosition();
			TVector3 rotation = ToEulerAngles(transform.GetRotation());
			TVector3 scale = transform.GetScale();

			imguiContext.BeginHorizontal();
			imguiContext.Label("Position");
			imguiContext.Vector3Field("##1", position, [&transform, &position, &rotation, &scale, &editorContext, &actionsHistory]
			{ 
				E_RESULT_CODE result = RC_OK;

				if (auto pAction = CreateTransformObjectAction(&editorContext.mWorld, editorContext.mCurrEntityId, { position, rotation, scale }, result))
				{
					PANIC_ON_FAILURE(actionsHistory.PushAndExecuteAction(pAction));
				}

				transform.SetPosition(position); 
			});
			imguiContext.EndHorizontal();

			imguiContext.BeginHorizontal();
			imguiContext.Label("Rotation");
			imguiContext.Vector3Field("##2", rotation, [&transform, &position, &rotation, &scale, &editorContext, &actionsHistory]
			{ 
				E_RESULT_CODE result = RC_OK;

				if (auto pAction = CreateTransformObjectAction(&editorContext.mWorld, editorContext.mCurrEntityId, { position, rotation, scale }, result))
				{
					PANIC_ON_FAILURE(actionsHistory.PushAndExecuteAction(pAction));
				}

				transform.SetRotation(TQuaternion(rotation)); 
			});
			imguiContext.EndHorizontal();

			imguiContext.BeginHorizontal();
			imguiContext.Label("Scale   ");
			imguiContext.Vector3Field("##3", scale, [&transform, &position, &rotation, &scale, &editorContext, &actionsHistory]
			{ 
				E_RESULT_CODE result = RC_OK;

				if (auto pAction = CreateTransformObjectAction(&editorContext.mWorld, editorContext.mCurrEntityId, { position, rotation, scale }, result))
				{
					PANIC_ON_FAILURE(actionsHistory.PushAndExecuteAction(pAction));
				}

				transform.SetScale(scale);
			});
			imguiContext.EndHorizontal();
		}
	}

	void CDefeaultInspectorsRegistry::DrawBoundsGUI(const TEditorContext& editorContext)
	{
		IImGUIContext& imguiContext = editorContext.mImGUIContext;
		IComponent& component = editorContext.mComponent;

		if (imguiContext.CollapsingHeader("Bounds", true))
		{
			CBoundsComponent& bounds = dynamic_cast<CBoundsComponent&>(component);

			// \todo Implement this drawer
		}
	}

	void CDefeaultInspectorsRegistry::DrawQuadSpriteGUI(const TEditorContext& editorContext)
	{
		IImGUIContext& imguiContext = editorContext.mImGUIContext;
		IComponent& component = editorContext.mComponent;

		if (imguiContext.CollapsingHeader("QuadSprite", true))
		{
			CQuadSprite& sprite = dynamic_cast<CQuadSprite&>(component);

			imguiContext.Label(sprite.GetMaterialName());
			// \todo Implement this drawer
		}
	}

	void CDefeaultInspectorsRegistry::DrawStaticMeshContainerGUI(const TEditorContext& editorContext)
	{
		IImGUIContext& imguiContext = editorContext.mImGUIContext;
		IComponent& component = editorContext.mComponent;

		if (imguiContext.CollapsingHeader("Static Mesh Container", true))
		{
			CStaticMeshContainer& meshContainer = dynamic_cast<CStaticMeshContainer&>(component);

			imguiContext.Label(meshContainer.GetMeshName());
			imguiContext.Label(meshContainer.GetMaterialName());
			// \todo Implement this drawer
		}
	}

	void CDefeaultInspectorsRegistry::DrawShadowReceiverGUI(const TEditorContext& editorContext)
	{
		IImGUIContext& imguiContext = editorContext.mImGUIContext;
		IComponent& component = editorContext.mComponent;

		if (imguiContext.CollapsingHeader("Shadow Receiver", true))
		{
			CShadowReceiverComponent& shadowReceiver = dynamic_cast<CShadowReceiverComponent&>(component);

			// \todo Implement this drawer
		}
	}

	void CDefeaultInspectorsRegistry::DrawShadowCasterGUI(const TEditorContext& editorContext)
	{
		IImGUIContext& imguiContext = editorContext.mImGUIContext;
		IComponent& component = editorContext.mComponent;

		if (imguiContext.CollapsingHeader("Shadow Caster", true))
		{
			// \todo Implement this drawer
		}
	}

	void CDefeaultInspectorsRegistry::DrawDirectionalLightGUI(const TEditorContext& editorContext)
	{
		IImGUIContext& imguiContext = editorContext.mImGUIContext;
		IComponent& component = editorContext.mComponent;

		if (imguiContext.CollapsingHeader("Directional Light", true))
		{
			CDirectionalLight& dirLight = dynamic_cast<CDirectionalLight&>(component);

			// \todo Implement this drawer
			
		}
	}

	void CDefeaultInspectorsRegistry::DrawPointLightGUI(const TEditorContext& editorContext)
	{
		IImGUIContext& imguiContext = editorContext.mImGUIContext;
		IComponent& component = editorContext.mComponent;

		if (imguiContext.CollapsingHeader("Point Light", true))
		{
			CPointLight& pointLight = dynamic_cast<CPointLight&>(component);

			// \todo Implement this drawer
		}
	}

	void CDefeaultInspectorsRegistry::DrawAnimationContainerGUI(const TEditorContext& editorContext)
	{
		const TVector2 buttonSizes(100.0f, 25.0f);

		IImGUIContext& imguiContext = editorContext.mImGUIContext;
		IComponent& component = editorContext.mComponent;

		if (imguiContext.CollapsingHeader("Animation Container", true))
		{
			CAnimationContainerComponent& animationComponent = dynamic_cast<CAnimationContainerComponent&>(component);

			imguiContext.Label(Wrench::StringUtils::Format("Clip: {0}", animationComponent.GetAnimationClipId()));

			const F32 duration = animationComponent.IsPlaying() ? animationComponent.GetDuration() : 0.0f;
			F32 time = animationComponent.GetTime();

			imguiContext.BeginHorizontal();
			imguiContext.Label("Time: ");
			imguiContext.FloatSlider(Wrench::StringUtils::GetEmptyStr(), time, 0.0f, duration);
			imguiContext.EndHorizontal();

			// \note Toolbar
			imguiContext.BeginHorizontal();
			{
				// \note Pause/Play
				imguiContext.Button(animationComponent.IsPaused() || !animationComponent.IsPlaying() ? "Play" : "Pause", buttonSizes, [&animationComponent]
				{
					if (animationComponent.IsPlaying())
					{
						animationComponent.SetPausedFlag(!animationComponent.IsPaused());
						return;
					}

					animationComponent.Play();
				});

				// \note Stop
				imguiContext.Button("Stop", buttonSizes, [&animationComponent]
				{
					animationComponent.SetStoppedFlag(true);
				});
			}
			imguiContext.EndHorizontal();
		}
	}

	void CDefeaultInspectorsRegistry::DrawSkyboxGUI(const TEditorContext& editorContext)
	{
		IImGUIContext& imguiContext = editorContext.mImGUIContext;
		IComponent& component = editorContext.mComponent;

		if (imguiContext.CollapsingHeader("Skybox", true)) {}
	}

	void CDefeaultInspectorsRegistry::DrawParticleEmitterGUI(const TEditorContext& editorContext)
	{
		IImGUIContext& imguiContext = editorContext.mImGUIContext;
		IComponent& component = editorContext.mComponent;

		if (imguiContext.CollapsingHeader("Particle Emitter", true))
		{
			CParticleEmitter& particleEmitter = dynamic_cast<CParticleEmitter&>(component);

			// \todo Implement this drawer
		}
	}

	void CDefeaultInspectorsRegistry::DrawCanvasGUI(const TEditorContext& editorContext)
	{
		IImGUIContext& imguiContext = editorContext.mImGUIContext;
		IComponent& component = editorContext.mComponent;

		if (imguiContext.CollapsingHeader("Canvas", true))
		{
			CCanvas& canvas = dynamic_cast<CCanvas&>(component);
			
			I32 width = canvas.GetWidth();
			I32 height = canvas.GetHeight();
			
			imguiContext.BeginHorizontal();
			imguiContext.Label("Width");
			imguiContext.IntField("##Width", width, [&canvas, &width] { canvas.SetWidth(static_cast<U32>(std::max<I32>(0, width))); });
			imguiContext.EndHorizontal();

			imguiContext.BeginHorizontal();
			imguiContext.Label("Height");
			imguiContext.IntField("##Height", height, [&canvas, &height] { canvas.SetHeight(static_cast<U32>(std::max<I32>(0, height))); });
			imguiContext.EndHorizontal();
		}
	}


	static void SetPositionForLayoutElement(CLayoutElement& layoutElement, const TVector2& position)
	{
		auto parentWorldRect = layoutElement.GetParentWorldRect();

		const TVector2 parentLBRect = parentWorldRect.GetLeftBottom();
		const TVector2 parentRectSize = parentWorldRect.GetSizes();

		const TVector2 minAnchor = layoutElement.GetMinAnchor();
		const TVector2 maxAnchor = layoutElement.GetMaxAnchor();

		const TVector2 lbWorldPoint = parentLBRect + parentRectSize * minAnchor;
		const TVector2 rtWorldPoint = parentLBRect + parentRectSize * maxAnchor;

		const F32 maxOffsetSign = Length(maxAnchor - minAnchor) < 1e-3f ? 1.0f : -1.0f;

		const TRectF32 rect
		{
			lbWorldPoint + layoutElement.GetMinOffset(),
			rtWorldPoint + maxOffsetSign * layoutElement.GetMaxOffset() /// \todo Is this a correct way to implement that?
		};

		TVector2 delta = position - rect.GetLeftBottom() - rect.GetSizes() * layoutElement.GetPivot();

		layoutElement.SetMinOffset(layoutElement.GetMinOffset() + delta);
		layoutElement.SetMaxOffset(layoutElement.GetMaxOffset() - delta);
	}


	static void DrawLayoutElementHandles(const TEditorContext& editorContext, CLayoutElement& layoutElement)
	{
		enum class E_ANCHOR_TYPE : U8
		{
			LEFT_BOTTOM,
			RIGHT_BOTTOM,
			RIGHT_TOP,
			LEFT_TOP,
		};


		IImGUIContext& imguiContext = editorContext.mImGUIContext;

		constexpr F32 handleRadius = 4.0f;
		static const TVector2 anchorSizes{ 10.0f, 20.0f };

		F32 canvasHeight = 0.0f;

		if (CEntity* pCanvasEntity = editorContext.mWorld.FindEntity(layoutElement.GetOwnerCanvasId()))
		{
			if (CCanvas* pCanvas = pCanvasEntity->GetComponent<CCanvas>())
			{
				canvasHeight = static_cast<F32>(pCanvas->GetHeight());
			}
		}

		static const IImGUIContext::TWindowParams params
		{
			ZeroVector2,
			ZeroVector2,
			ZeroVector2,
			true, true, false, true
		};

		bool opened = true;

		if (imguiContext.BeginWindow("LayoutElementEditor", opened, params))
		{
			auto worldRect = layoutElement.GetWorldRect();

			/// \note Draw a pivot
			imguiContext.DisplayIDGroup(1, [&imguiContext, handleRadius, worldRect, canvasHeight, &layoutElement]
			{
				auto&& pivot = layoutElement.GetPivot();

				TVector2 worldPosition{ worldRect.GetLeftBottom() + worldRect.GetSizes() * pivot };
				worldPosition.y = canvasHeight - worldPosition.y;

				imguiContext.SetCursorScreenPos(worldPosition - TVector2(5.0f * handleRadius));

				imguiContext.DrawCircle(worldPosition, handleRadius, true, TColorUtils::mWhite);
				imguiContext.DrawCircle(worldPosition, 2.0f * handleRadius, false, TColorUtils::mWhite, 2.5f);

				imguiContext.Button("##Pivot", TVector2(10.0f * handleRadius), nullptr, true);

				/// \note Move layoutElement's pivot
				if (imguiContext.IsItemActive() && imguiContext.IsMouseDragging(0))
				{
					auto&& mousePosition = PointToNormalizedCoords(worldRect, imguiContext.GetMousePosition(), false);
					layoutElement.SetPivot(TVector2(mousePosition.x, 1.0f - mousePosition.y));
				}
			});

			/// \note Draw anchors
			auto parentWorldRect = layoutElement.GetParentWorldRect();
			auto anchorsRelativeWorldRect = layoutElement.GetAnchorWorldRect();

			auto&& anchorsRectPoints = anchorsRelativeWorldRect.GetPoints();

			static const std::array<TVector2, 4> anchorRectsOffsets { TVector2(-5.0f * handleRadius, 0.0f), ZeroVector2, TVector2(0.0f, -5.0f * handleRadius), TVector2(-5.0f * handleRadius) };

			for (U8 i = 0; i < anchorsRectPoints.size(); ++i)
			{
				auto p = anchorsRectPoints[i];
				p = TVector2(p.x, canvasHeight - p.y); // transform from UI space to screen space

				const F32 s0 = (i % 3 == 0 ? -1.0f : 1.0f);
				const F32 s1 = (i < 2 ? 1.0f : -1.0f);

				imguiContext.DisplayIDGroup(static_cast<U32>(10 + i), [&imguiContext, &parentWorldRect, &layoutElement, s0, s1, p, handleRadius, i]
				{
					imguiContext.DrawTriangle(p + TVector2(s0 * anchorSizes.x, s1 * anchorSizes.y), p + TVector2(s0 * anchorSizes.y, s1 * anchorSizes.x), p, TColorUtils::mWhite, false, 1.f);

					imguiContext.SetCursorScreenPos(p + anchorRectsOffsets[i]);
					imguiContext.Button(Wrench::StringUtils::GetEmptyStr(), TVector2(5.0f * handleRadius), nullptr, false);

					if (imguiContext.IsItemActive() && imguiContext.IsMouseDragging(0))
					{
						auto normalizedAnchorPos = PointToNormalizedCoords(parentWorldRect, imguiContext.GetMousePosition());
						normalizedAnchorPos.y = 1.0f - normalizedAnchorPos.y;

						auto&& minAnchor = layoutElement.GetMinAnchor();
						auto&& maxAnchor = layoutElement.GetMaxAnchor();

						switch (static_cast<E_ANCHOR_TYPE>(i))
						{
							case E_ANCHOR_TYPE::LEFT_BOTTOM:
								layoutElement.SetMinAnchor(normalizedAnchorPos);
								break;
							case E_ANCHOR_TYPE::RIGHT_BOTTOM:
								layoutElement.SetMinAnchor(TVector2(minAnchor.x, normalizedAnchorPos.y));
								layoutElement.SetMaxAnchor(TVector2(normalizedAnchorPos.x, maxAnchor.y));
								break;
							case E_ANCHOR_TYPE::RIGHT_TOP:
								layoutElement.SetMaxAnchor(normalizedAnchorPos);
								break;
							case E_ANCHOR_TYPE::LEFT_TOP:
								layoutElement.SetMinAnchor(TVector2(normalizedAnchorPos.x, minAnchor.y));
								layoutElement.SetMaxAnchor(TVector2(maxAnchor.x, normalizedAnchorPos.y));
								break;
							default:
								TDE2_UNREACHABLE();
								break;
						}
					}
				});
			}

			worldRect.y = canvasHeight - (worldRect.y + worldRect.height);

			/// \note Draw corner vertices
			U32 pointIndex = 0;

			for (auto&& currPoint : worldRect.GetPoints())
			{
				imguiContext.DisplayIDGroup(3 + pointIndex++, [&imguiContext, &layoutElement, currPoint, handleRadius]
				{
					imguiContext.DrawCircle(currPoint, handleRadius, true, TColorUtils::mBlue);

					imguiContext.SetCursorScreenPos(currPoint - TVector2(5.0f * handleRadius));
					imguiContext.Button(Wrench::StringUtils::GetEmptyStr(), TVector2(10.0f * handleRadius), nullptr, false);

					if (imguiContext.IsItemActive() && imguiContext.IsMouseDragging(0))
					{
					}
				});
			}

			/// \note Draw a rectangle
			imguiContext.DisplayIDGroup(2, [&imguiContext, &layoutElement, worldRect, canvasHeight]
			{
				const TVector2 cursorPos = imguiContext.GetCursorScreenPos();

				imguiContext.DrawRect(worldRect, TColorUtils::mGreen, false, 2.f);

				auto&& sizes = worldRect.GetSizes();

				imguiContext.SetCursorScreenPos(worldRect.GetLeftBottom());

				if (sizes.x * sizes.y > 0.0f)
				{
					imguiContext.Button("##Rect", sizes, nullptr, true);

					/// \note Move layoutElement if its rectangle selected and are dragged
					if (imguiContext.IsItemActive() && imguiContext.IsMouseDragging(0))
					{
						auto mousePosition = imguiContext.GetMousePosition();
						SetPositionForLayoutElement(layoutElement, TVector2(mousePosition.x, canvasHeight - mousePosition.y));
					}
				}
			});
		}

		imguiContext.EndWindow();
	}


	struct TLayoutPresetParams
	{
		TVector2 mMinAnchor;
		TVector2 mMaxAnchor;

		std::function<void()> mOnClick = nullptr;
	};


	constexpr U32 LayoutPresetsCount = 17;


	static const std::array<std::string, LayoutPresetsCount> LayoutPresetsList
	{
		"Top Left",
		"Top Right",
		"Bottom Left",
		"Bottom Right",
		"Center Left",
		"Center Top",
		"Center Right",
		"Center Bottom",
		"Center",
		"Left Wide",
		"VCenter Wide",
		"HCenter Wide",
		"Right Wide",
		"Top Wide",
		"Bottom Wide",
		"Full Rect",
		"Custom",
	};


	static const std::array<std::tuple<TVector2, TVector2, TVector2>, LayoutPresetsCount> LayoutAnchorsPresets
	{
		std::make_tuple<TVector2, TVector2, TVector2>({ 0.0f, 1.0f }, { 0.0f, 1.0f }, { 100.0f }),       ///< Top Left
		std::make_tuple<TVector2, TVector2, TVector2>({ 1.0f }, { 1.0f }, { 100.0f }),                   ///< Top Right
		std::make_tuple<TVector2, TVector2, TVector2>({ 0.0f }, { 0.0f }, { 100.0f }),                   ///< Bottom Left
		std::make_tuple<TVector2, TVector2, TVector2>({ 1.0f, 0.0f }, { 1.0f, 0.0f }, { 100.0f }),       ///< Bottom Right
		std::make_tuple<TVector2, TVector2, TVector2>({ 0.0f, 0.5f }, { 0.0f, 0.5f }, { 100.0f }),       ///< Center Left
		std::make_tuple<TVector2, TVector2, TVector2>({ 0.5f, 1.0f }, { 0.5f, 1.0f }, { 100.0f }),       ///< Center Top
		std::make_tuple<TVector2, TVector2, TVector2>({ 1.0f, 0.5f }, { 1.0f, 0.5f }, { 100.0f }),       ///< Center Right
		std::make_tuple<TVector2, TVector2, TVector2>({ 0.5f, 0.0f }, { 0.5f, 0.0f }, { 100.0f }),       ///< Center Bottom
		std::make_tuple<TVector2, TVector2, TVector2>({ 0.5f }, { 0.5f }, { 100.0f }),                   ///< Center
		std::make_tuple<TVector2, TVector2, TVector2>({ 0.0f }, { 0.0f, 1.0f }, { 100.0f, 0.0f }),       ///< Left Wide
		std::make_tuple<TVector2, TVector2, TVector2>({ 0.5f, 0.0f }, { 0.5f, 1.0f }, { 100.0f, 0.0f }), ///< VCenter Wide
		std::make_tuple<TVector2, TVector2, TVector2>({ 0.0f, 0.5f }, { 1.0f, 0.5f }, { 0.0f, 100.0f }), ///< HCenter Wide
		std::make_tuple<TVector2, TVector2, TVector2>({ 1.0f, 0.0f }, { 1.0f }, { 100.0f, 0.0f }),       ///< Right Wide
		std::make_tuple<TVector2, TVector2, TVector2>({ 0.0f, 1.0f }, { 1.0f }, { 0.0f, 100.0f }),       ///< Top Wide
		std::make_tuple<TVector2, TVector2, TVector2>({ 0.0f }, { 1.0f, 0.0f }, { 0.0f, 100.0f }),       ///< Bottom Wide
		std::make_tuple<TVector2, TVector2, TVector2>({ 0.0f }, { 1.0f }, { 100.0f }),                   ///< Full Rect
	};


	static void DrawLayoutPresetIcon(const TEditorContext& editorContext, const std::string& text, const TLayoutPresetParams& params = {})
	{
		constexpr F32 iconSize = 14.0f;
		constexpr F32 iconBorderSize = 1.0f;
		constexpr F32 innerRectSize = (iconSize - 2.0f * iconBorderSize);
		constexpr F32 presetRectSize = innerRectSize / 3.0f;

		static const std::array<TVector2, LayoutPresetsCount> presetsRectSizes
		{
			TVector2 { presetRectSize },                                     ///< Top Left
			TVector2 { presetRectSize },                                     ///< Top Right
			TVector2 { presetRectSize },                                     ///< Bottom Left
			TVector2 { presetRectSize },                                     ///< Bottom Right
			TVector2 { presetRectSize },                                     ///< Center Left
			TVector2 { presetRectSize },                                     ///< Center Top
			TVector2 { presetRectSize },                                     ///< Center Right
			TVector2 { presetRectSize },                                     ///< Center Bottom
			TVector2 { presetRectSize },                                     ///< Center
			TVector2 { presetRectSize, innerRectSize },                      ///< Left Wide
			TVector2 { presetRectSize, innerRectSize },						///< VCenter Wide
			TVector2 { innerRectSize, presetRectSize },                      ///< HCenter Wide
			TVector2 { presetRectSize, innerRectSize },                      ///< Right Wide
			TVector2 { innerRectSize, presetRectSize },                      ///< Top Wide
			TVector2 { innerRectSize, presetRectSize },                      ///< Bottom Wide
			TVector2 { innerRectSize },                                      ///< Full Rect
			ZeroVector2
		};

		IImGUIContext& imguiContext = editorContext.mImGUIContext;

		TVector2 cursorPos = imguiContext.GetCursorScreenPos();

		imguiContext.BeginHorizontal();
		
		imguiContext.DrawRect({ cursorPos.x, cursorPos.y, iconSize, iconSize }, TColorUtils::mGray);
		imguiContext.DrawRect({ cursorPos.x + iconBorderSize, cursorPos.y + iconBorderSize, iconSize - 2.0f * iconBorderSize, iconSize - 2.0f * iconBorderSize }, TColorUtils::mBlack);

		for (U32 i = 0; i < LayoutPresetsCount; ++i)
		{
			const TVector2& minAnchor = std::get<0>(LayoutAnchorsPresets[i]);

			if (minAnchor != params.mMinAnchor || std::get<1>(LayoutAnchorsPresets[i]) != params.mMaxAnchor)
			{
				continue;
			}

			const TVector2& rectSizes = presetsRectSizes[i];

			imguiContext.DrawRect(
				{ 
					cursorPos.x + iconBorderSize + minAnchor.x * innerRectSize - ((minAnchor.x > 0.5f) ? rectSizes.x : 0.0f) - (std::abs(minAnchor.x - 0.5f) < 1e-3f ? presetRectSize * 0.5f : 0.0f),
					cursorPos.y + iconBorderSize + (1.0f - minAnchor.y) * innerRectSize - ((minAnchor.y < 0.5f) ? rectSizes.y : 0.0f) - (std::abs(minAnchor.y - 0.5f) < 1e-3f ? presetRectSize * 0.5f : 0.0f),
					rectSizes.x, 
					rectSizes.y 
				}, TColorUtils::mWhite);
		}

		imguiContext.Button(text + "##Button", TVector2(imguiContext.GetWindowWidth() * 0.4f, iconSize), params.mOnClick, true);

		imguiContext.SetCursorScreenPos(cursorPos + TVector2(iconSize + 5.0f, 0.0f));
		imguiContext.Label(text);

		imguiContext.EndHorizontal();
	}

	
	static U32 FindPresetIndexByAnchors(const TVector2& minAnchor, const TVector2& maxAnchor)
	{
		for (U32 i = 0; i < LayoutPresetsCount; ++i)
		{
			if (std::get<0>(LayoutAnchorsPresets[i]) == minAnchor && std::get<1>(LayoutAnchorsPresets[i]) == maxAnchor)
			{
				return i;
			}
		}

		return static_cast<U32>(LayoutPresetsCount - 1);
	}


	void CDefeaultInspectorsRegistry::DrawLayoutElementGUI(const TEditorContext& editorContext)
	{
		IImGUIContext& imguiContext = editorContext.mImGUIContext;
		IComponent& component = editorContext.mComponent;

		if (imguiContext.CollapsingHeader("Layout Element", true))
		{
			CLayoutElement& layoutElement = dynamic_cast<CLayoutElement&>(component);

			DrawLayoutElementHandles(editorContext, layoutElement);

			static const std::string anchorPresetsPopupId = "Anchor Presets";

			/// \note Anchors presets
			DrawLayoutPresetIcon(editorContext, LayoutPresetsList[FindPresetIndexByAnchors(layoutElement.GetMinAnchor(), layoutElement.GetMaxAnchor())],
				{ 
					layoutElement.GetMinAnchor(), layoutElement.GetMaxAnchor(), 
					[&imguiContext]
					{
						imguiContext.ShowModalWindow(anchorPresetsPopupId);
					} 
				});
			
			if (imguiContext.BeginModalWindow(anchorPresetsPopupId, true))
			{
				for (U32 i = 0; i < LayoutPresetsCount - 1; ++i)
				{
					DrawLayoutPresetIcon(editorContext, LayoutPresetsList[i], 
						{ 
							std::get<0>(LayoutAnchorsPresets[i]), 
							std::get<1>(LayoutAnchorsPresets[i]),
							[&imguiContext, &layoutElement, i]
							{
								auto&& anchorsPreset = LayoutAnchorsPresets[i];

								layoutElement.SetMinAnchor(std::get<0>(anchorsPreset));
								layoutElement.SetMaxAnchor(std::get<1>(anchorsPreset));

								layoutElement.SetMaxOffset(std::get<2>(anchorsPreset));

								imguiContext.CloseCurrentModalWindow();
							}
						});
				}

				imguiContext.EndModalWindow();
			}

			const F32 inputElementWidth = imguiContext.GetWindowWidth() * 0.2f;

			// \todo Implement this drawer
			{
				TVector2 minOffset = layoutElement.GetMinOffset();

				imguiContext.SetItemWidth(inputElementWidth, [&imguiContext, &minOffset, &layoutElement]
				{
					imguiContext.BeginHorizontal();
					imguiContext.Label("minOffX"); /// \todo Replace it 
					imguiContext.FloatField("##minOffsetX", minOffset.x, [&minOffset, &layoutElement] { layoutElement.SetMinOffset(minOffset); });

					imguiContext.Label("minOffY"); /// \todo Replace it 
					imguiContext.FloatField("##minOffsetY", minOffset.y, [&minOffset, &layoutElement] { layoutElement.SetMinOffset(minOffset); });
					imguiContext.EndHorizontal();
				});
			}

			{
				TVector2 maxOffset = layoutElement.GetMaxOffset();

				imguiContext.SetItemWidth(inputElementWidth, [&imguiContext, &maxOffset, &layoutElement]
				{
					imguiContext.BeginHorizontal();
					imguiContext.Label("maxOffX"); /// \todo Replace it 
					imguiContext.FloatField("##maxOffsetX", maxOffset.x, [&maxOffset, &layoutElement] { layoutElement.SetMaxOffset(maxOffset); });
					
					imguiContext.Label("maxOffY"); /// \todo Replace it 
					imguiContext.FloatField("##maxOffsetY", maxOffset.y, [&maxOffset, &layoutElement] { layoutElement.SetMaxOffset(maxOffset); });
					imguiContext.EndHorizontal();
				});
			}

			{
				TVector2 minAnchor = layoutElement.GetMinAnchor();

				imguiContext.SetItemWidth(inputElementWidth, [&imguiContext, &minAnchor, &layoutElement]
				{
					imguiContext.BeginHorizontal();
					imguiContext.Label("Min Anchor:\tX"); /// \todo Replace it 
					imguiContext.FloatField("##minAnchorX", minAnchor.x, [&minAnchor, &layoutElement] { layoutElement.SetMinAnchor(minAnchor); });
					
					imguiContext.Label("Y"); /// \todo Replace it 
					imguiContext.FloatField("##minAnchorY", minAnchor.y, [&minAnchor, &layoutElement] { layoutElement.SetMinAnchor(minAnchor); });
					imguiContext.EndHorizontal();
				});
			}

			{
				TVector2 maxAnchor = layoutElement.GetMaxAnchor();

				imguiContext.SetItemWidth(inputElementWidth, [&imguiContext, &maxAnchor, &layoutElement]
				{
					imguiContext.BeginHorizontal();
					imguiContext.Label("Max Anchor:\tX"); /// \todo Replace it
					imguiContext.FloatField("##maxAnchorX", maxAnchor.x, [&maxAnchor, &layoutElement] { layoutElement.SetMaxAnchor(maxAnchor); });

					imguiContext.Label("Y"); /// \todo Replace it 
					imguiContext.FloatField("##maxAnchorY", maxAnchor.y, [&maxAnchor, &layoutElement] { layoutElement.SetMaxAnchor(maxAnchor); });
					imguiContext.EndHorizontal();
				});
			}

			{
				TVector2 pivot = layoutElement.GetPivot();

				imguiContext.BeginHorizontal();
				imguiContext.Label("Pivot"); /// \todo Replace it 
				imguiContext.Vector2Field("##pivot", pivot, [&pivot, &layoutElement] { layoutElement.SetPivot(pivot); });
				imguiContext.EndHorizontal();
			}
		}
	}

	void CDefeaultInspectorsRegistry::DrawUIElementMeshDataGUI(const TEditorContext& editorContext)
	{
		IImGUIContext& imguiContext = editorContext.mImGUIContext;
		IComponent& component = editorContext.mComponent;

		if (imguiContext.CollapsingHeader("UI Element Mesh Data", true))
		{
			//CLayoutElement& layoutElement = dynamic_cast<CLayoutElement&>(component);

			// \todo Implement this drawer
		}
	}

	void CDefeaultInspectorsRegistry::DrawImageGUI(const TEditorContext& editorContext)
	{
		IImGUIContext& imguiContext = editorContext.mImGUIContext;
		IComponent& component = editorContext.mComponent;

		if (imguiContext.CollapsingHeader("Image", true))
		{
			//CLayoutElement& layoutElement = dynamic_cast<CLayoutElement&>(component);

			// \todo Implement this drawer
		}
	}


	void CDefeaultInspectorsRegistry::DrawBoxCollision2DGUI(const TEditorContext& editorContext)
	{
		IImGUIContext& imguiContext = editorContext.mImGUIContext;
		IComponent& component = editorContext.mComponent;

		if (imguiContext.CollapsingHeader("BoxCollision2D", true))
		{
			CBoxCollisionObject2D& box2Dcollision = dynamic_cast<CBoxCollisionObject2D&>(component);

			// \todo Implement this drawer
		}
	}

	void CDefeaultInspectorsRegistry::DrawCircleCollision2DGUI(const TEditorContext& editorContext)
	{
		IImGUIContext& imguiContext = editorContext.mImGUIContext;
		IComponent& component = editorContext.mComponent;

		if (imguiContext.CollapsingHeader("CircleCollision2D", true))
		{
			CCircleCollisionObject2D& circle2Dcollision = dynamic_cast<CCircleCollisionObject2D&>(component);
			// \todo Implement this drawer
		}
	}

	void CDefeaultInspectorsRegistry::DrawTrigger2DGUI(const TEditorContext& editorContext)
	{
		IImGUIContext& imguiContext = editorContext.mImGUIContext;
		IComponent& component = editorContext.mComponent;

		if (imguiContext.CollapsingHeader("Trigger2D", true))
		{
			CTrigger2D& trigger = dynamic_cast<CTrigger2D&>(component);

			// \todo Implement this drawer
		}
	}

	void CDefeaultInspectorsRegistry::DrawBoxCollision3DGUI(const TEditorContext& editorContext)
	{
		IImGUIContext& imguiContext = editorContext.mImGUIContext;
		IComponent& component = editorContext.mComponent;

		if (imguiContext.CollapsingHeader("BoxCollision3D", true))
		{
			CBaseCollisionObject3D& box3Dcollision = dynamic_cast<CBaseCollisionObject3D&>(component);

			// \todo Implement this drawer
		}
	}

	void CDefeaultInspectorsRegistry::DrawSphereCollision3DGUI(const TEditorContext& editorContext)
	{
		IImGUIContext& imguiContext = editorContext.mImGUIContext;
		IComponent& component = editorContext.mComponent;

		if (imguiContext.CollapsingHeader("SphereCollision3D", true))
		{
			CSphereCollisionObject3D& sphereCollision = dynamic_cast<CSphereCollisionObject3D&>(component);

			// \todo Implement this drawer
		}
	}

	void CDefeaultInspectorsRegistry::DrawConvexHullCollision3DGUI(const TEditorContext& editorContext)
	{
		IImGUIContext& imguiContext = editorContext.mImGUIContext;
		IComponent& component = editorContext.mComponent;

		if (imguiContext.CollapsingHeader("ConvexHullCollision3D", true))
		{
			CConvexHullCollisionObject3D& convexHullCollision = dynamic_cast<CConvexHullCollisionObject3D&>(component);

			// \todo Implement this drawer
		}
	}

	void CDefeaultInspectorsRegistry::DrawTrigger3DGUI(const TEditorContext& editorContext)
	{
		IImGUIContext& imguiContext = editorContext.mImGUIContext;
		IComponent& component = editorContext.mComponent;

		if (imguiContext.CollapsingHeader("Trigger3D", true))
		{
			// \todo Implement this drawer
		}
	}


	void CDefeaultInspectorsRegistry::DrawAudioSourceGUI(const TEditorContext& editorContext)
	{
		IImGUIContext& imguiContext = editorContext.mImGUIContext;
		IComponent& component = editorContext.mComponent;

		if (imguiContext.CollapsingHeader("Audio Source", true))
		{
			CAudioSourceComponent& audioSource = dynamic_cast<CAudioSourceComponent&>(component);

			imguiContext.Label(audioSource.GetAudioClipId());

			// boolean staffs
			bool isMuted = audioSource.IsMuted();
			imguiContext.Checkbox("Mute", isMuted);
			audioSource.SetMuted(isMuted);

			bool isLooped = audioSource.IsLooped();
			imguiContext.Checkbox("Loop", isLooped);
			audioSource.SetLooped(isLooped);

			// float
			F32 volume = audioSource.GetVolume();
			imguiContext.FloatSlider("Volume", volume, 0.0f, 1.0f, [&audioSource, &volume] { audioSource.SetVolume(volume); });

			F32 panning = audioSource.GetPanning();
			imguiContext.FloatSlider("Panning", panning, -1.0f, 1.0f, [&audioSource, &panning] { audioSource.SetPanning(panning); });
		}
	}

	void CDefeaultInspectorsRegistry::DrawAudioListenerGUI(const TEditorContext& editorContext)
	{
		IImGUIContext& imguiContext = editorContext.mImGUIContext;
		IComponent& component = editorContext.mComponent;

		if (imguiContext.CollapsingHeader("Audio Listener", true))
		{
		}
	}
}

#endif