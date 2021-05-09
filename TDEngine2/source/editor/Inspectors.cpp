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

	void CDefeaultInspectorsRegistry::DrawLayoutElementGUI(const TEditorContext& editorContext)
	{
		IImGUIContext& imguiContext = editorContext.mImGUIContext;
		IComponent& component = editorContext.mComponent;

		if (imguiContext.CollapsingHeader("Layout Element", true))
		{
			CLayoutElement& layoutElement = dynamic_cast<CLayoutElement&>(component);

#if 1		/// \todo Move to OnDrawGizmo
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

			auto worldRect = layoutElement.GetWorldRect();
			auto&& pivot = layoutElement.GetPivot();

			TVector2 worldPosition { worldRect.GetLeftBottom() + worldRect.GetSizes() * pivot };
			worldPosition.y = canvasHeight - worldPosition.y;

			imguiContext.DrawCircle(worldPosition, handleRadius, true, TColorUtils::mWhite);
			imguiContext.DrawCircle(worldPosition, 2.0f * handleRadius, false, TColorUtils::mWhite, 2.5f);

			worldRect.y = CMathUtils::Max(0.0f, worldRect.y - worldRect.height);

			imguiContext.DrawRect(worldRect, TColorUtils::mGreen, false, 2.f);

			for (auto&& currPoint : worldRect.GetPoints())
			{
				imguiContext.DrawCircle(currPoint, handleRadius, true, TColorUtils::mBlue);
			}

			/// \note Draw anchors
			auto parentWorldRect = layoutElement.GetParentWorldRect();

			auto&& parentRectPoints = parentWorldRect.GetPoints();

			for (U8 i = 0; i < parentRectPoints.size(); ++i)
			{
				auto&& p = parentRectPoints[i];

				const F32 s0 = (i % 3 == 0 ? -1.0f : 1.0f);
				const F32 s1 = (i < 2 ? -1.0f : 1.0f);

				imguiContext.DrawTriangle(p + TVector2(s0 * anchorSizes.x, s1 * anchorSizes.y), p + TVector2(s0 * anchorSizes.y, s1 * anchorSizes.x), p, TColorUtils::mWhite, false, 1.f);
			}
#endif
			static const std::string anchorPresetsPopupId = "Anchor Presets";

			/// \note Anchors presets
			imguiContext.Button("Presets", TVector2(50.0f, 50.0f), [&imguiContext]
			{
				imguiContext.ShowModalWindow(anchorPresetsPopupId);
			});

			if (imguiContext.BeginModalWindow(anchorPresetsPopupId, true))
			{
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