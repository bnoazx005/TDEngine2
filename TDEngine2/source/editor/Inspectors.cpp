#include "../../include/editor/Inspectors.h"
#include "../../include/core/IImGUIContext.h"
#include "../../include/ecs/IComponent.h"
#include "../../include/ecs/CTransform.h"
#include "../../include/ecs/components/CBoundsComponent.h"
#include "../../include/ecs/IWorld.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/graphics/CQuadSprite.h"
#include "../../include/graphics/CStaticMeshContainer.h"
#include "../../include/graphics/CSkinnedMeshContainer.h"
#include "../../include/graphics/effects/CParticleEmitterComponent.h"
#include "../../include/graphics/UI/CCanvasComponent.h"
#include "../../include/graphics/UI/CLayoutElementComponent.h"
#include "../../include/graphics/UI/CUIElementMeshDataComponent.h"
#include "../../include/graphics/UI/CImageComponent.h"
#include "../../include/graphics/UI/CInputReceiverComponent.h"
#include "../../include/graphics/UI/CLabelComponent.h"
#include "../../include/graphics/UI/C9SliceImageComponent.h"
#include "../../include/graphics/UI/GroupLayoutComponents.h"
#include "../../include/graphics/animation/CAnimationContainerComponent.h"
#include "../../include/graphics/animation/CMeshAnimatorComponent.h"
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
#include "../../include/scene/components/CLODStrategyComponent.h"
#include "../../include/editor/ecs/EditorComponents.h"
#include "../../include/editor/CLevelEditorWindow.h"
#include "../../include/editor/CEditorActionsManager.h"
#include "../../include/utils/CFileLogger.h"
#include "../../include/metadata.h"
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
		result = result | editor.RegisterInspector(CSkinnedMeshContainer::GetTypeId(), DrawSkinnedMeshContainerGUI);
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
		result = result | editor.RegisterInspector(CInputReceiver::GetTypeId(), DrawInputReceiverGUI);
		result = result | editor.RegisterInspector(CLabel::GetTypeId(), DrawLabelGUI);
		result = result | editor.RegisterInspector(C9SliceImage::GetTypeId(), Draw9SliceImageGUI);
		result = result | editor.RegisterInspector(CGridGroupLayout::GetTypeId(), DrawGridGroupLayoutGUI);
		result = result | editor.RegisterInspector(CMeshAnimatorComponent::GetTypeId(), DrawMeshAnimatorGUI);

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

		/// Scenes
		result = result | editor.RegisterInspector(CSelectedEntityComponent::GetTypeId(), [](auto) { /* Do nothing for hidden components */ });
		result = result | editor.RegisterInspector(CLODStrategyComponent::GetTypeId(), DrawLODStrategyGUI);

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
			TVector3 rotation = ToEulerAngles(transform.GetRotation()) * CMathConstants::Rad2Deg;
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

				transform.SetRotation(TQuaternion(rotation * CMathConstants::Deg2Rad)); 
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

			/// \note Mesh identifier
			{
				std::string meshId = meshContainer.GetMeshName();

				imguiContext.BeginHorizontal();
				imguiContext.Label("Mesh Id:");
				imguiContext.TextField("##MeshId", meshId, [&meshContainer](auto&& value) { meshContainer.SetMeshName(value); });
				imguiContext.EndHorizontal();
			}

			/// \note Sub-mesh idenfitier
			{
				auto&& submeshes = meshContainer.GetSubmeshesIdentifiers();

				std::string subMeshId = meshContainer.GetSubMeshId();

				imguiContext.BeginHorizontal();
				imguiContext.Label("Sub-mesh Id:");

				const I32 index = static_cast<I32>(std::distance(submeshes.cbegin(), std::find(submeshes.cbegin(), submeshes.cend(), subMeshId)));
				std::string newResult = submeshes[imguiContext.Popup("##SubMeshId", index, submeshes)];

				if (newResult != subMeshId) /// \note Update sub-mesh identifier
				{
					meshContainer.SetSubMeshId(newResult);
				}

				imguiContext.EndHorizontal();
			}

			/// \note Material
			{
				std::string materialId = meshContainer.GetMaterialName();
				std::string prevMaterialid = materialId;

				imguiContext.BeginHorizontal();
				imguiContext.Label("Material Id:");
				imguiContext.TextField("##MaterialId", materialId);

				if (prevMaterialid != materialId)
				{
					meshContainer.SetMaterialName(materialId);
				}

				imguiContext.EndHorizontal();
			}
		}
	}

	void CDefeaultInspectorsRegistry::DrawSkinnedMeshContainerGUI(const TEditorContext& editorContext)
	{
		IImGUIContext& imguiContext = editorContext.mImGUIContext;
		IComponent& component = editorContext.mComponent;

		if (imguiContext.CollapsingHeader("Skinned Mesh Container", true))
		{
			CSkinnedMeshContainer& meshContainer = dynamic_cast<CSkinnedMeshContainer&>(component);

			/// \note Mesh identifier
			{
				std::string meshId = meshContainer.GetMeshName();

				imguiContext.BeginHorizontal();
				imguiContext.Label("Mesh Id:");
				imguiContext.TextField("##MeshId", meshId, [&meshContainer](auto&& value) { meshContainer.SetMeshName(value); });
				imguiContext.EndHorizontal();
			}

			/// \note Sub-mesh idenfitier
			{
				auto&& submeshes = meshContainer.GetSubmeshesIdentifiers();

				std::string subMeshId = meshContainer.GetSubMeshId();

				imguiContext.BeginHorizontal();
				imguiContext.Label("Sub-mesh Id:");

				const I32 index = static_cast<I32>(std::distance(submeshes.cbegin(), std::find(submeshes.cbegin(), submeshes.cend(), subMeshId)));
				std::string newResult = submeshes[imguiContext.Popup("##SubMeshId", index, submeshes)];

				if (newResult != subMeshId) /// \note Update sub-mesh identifier
				{
					meshContainer.SetSubMeshId(newResult);
				}

				imguiContext.EndHorizontal();
			}

			/// \note Material
			{
				std::string materialId = meshContainer.GetMaterialName();

				imguiContext.BeginHorizontal();
				imguiContext.Label("Material Id:");
				imguiContext.TextField("##MaterialId", materialId, [&meshContainer](auto&& value) { meshContainer.SetMaterialName(value); });
				imguiContext.EndHorizontal();
			}

			/// \note Skeleton
			{
				std::string skeletonid = meshContainer.GetSkeletonName();

				imguiContext.BeginHorizontal();
				imguiContext.Label("Skeleton Id:");
				imguiContext.TextField("##SkeletonId", skeletonid, [&meshContainer](auto&& value) { meshContainer.SetSkeletonName(value); });
				imguiContext.EndHorizontal();
			}
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

			bool inheritSizesFlag = canvas.DoesInheritSizesFromMainCamera();

			{
				imguiContext.Checkbox("Inherit sizes from camera", inheritSizesFlag);

				if (inheritSizesFlag != canvas.DoesInheritSizesFromMainCamera())
				{
					canvas.SetInheritSizesFromMainCamera(inheritSizesFlag);
				}
			}
			
			imguiContext.BeginHorizontal();
			imguiContext.Label("Width");
			imguiContext.IntField("##Width", width, [&canvas, &width, inheritSizesFlag] { if (inheritSizesFlag) return; canvas.SetWidth(static_cast<U32>(std::max<I32>(0, width))); });
			imguiContext.EndHorizontal();

			imguiContext.BeginHorizontal();
			imguiContext.Label("Height");
			imguiContext.IntField("##Height", height, [&canvas, &height, inheritSizesFlag] { if (inheritSizesFlag) return; canvas.SetHeight(static_cast<U32>(std::max<I32>(0, height))); });
			imguiContext.EndHorizontal();
		}
	}

	void CDefeaultInspectorsRegistry::DrawMeshAnimatorGUI(const TEditorContext& editorContext)
	{
		IImGUIContext& imguiContext = editorContext.mImGUIContext;
		IComponent& component = editorContext.mComponent;

		if (imguiContext.CollapsingHeader("Mesh Animator", true))
		{
			CMeshAnimatorComponent& meshAnimator = dynamic_cast<CMeshAnimatorComponent&>(component);

			// \todo Implement this drawer
		}
	}


	static void SetPositionForLayoutElement(IImGUIContext& imguiContext, CLayoutElement& layoutElement)
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

		TVector2 delta = imguiContext.GetMouseDragDelta(0);
		delta.y = -delta.y;

		layoutElement.SetMinOffset(layoutElement.GetMinOffset() + delta);

		if (Length(maxAnchor - minAnchor) > 1e-3f)
		{
			layoutElement.SetMaxOffset(layoutElement.GetMaxOffset() - delta);
		}
	}


	/*!
		vertexIndex is an index of rect's corner

		0 *---* 1
		  |   |
		3 *---* 2
	*/

	static void SetRectangleSizesForLayoutElement(CLayoutElement& layoutElement, const TVector2& delta, int vertexIndex)
	{
		const TVector2 minAnchor = layoutElement.GetMinAnchor();
		const TVector2 maxAnchor = layoutElement.GetMaxAnchor();

		if (Length(maxAnchor - minAnchor) < 1e-3f)
		{
			switch (vertexIndex)
			{
				case 0:
					layoutElement.SetMinOffset(layoutElement.GetMinOffset() + delta);
					layoutElement.SetMaxOffset(layoutElement.GetMaxOffset() + TVector2(-delta.x, delta.y));
					break;
				case 1:
					layoutElement.SetMaxOffset(layoutElement.GetMaxOffset() + delta);
					break;
				case 2:
					layoutElement.SetMinOffset(layoutElement.GetMinOffset() + TVector2(0.0f, delta.y));
					layoutElement.SetMaxOffset(layoutElement.GetMaxOffset() + TVector2(delta.x, -delta.y));
					break;
				case 3:
					layoutElement.SetMinOffset(layoutElement.GetMinOffset() + delta);
					layoutElement.SetMaxOffset(layoutElement.GetMaxOffset() - delta);
					break;
			}

			return;
		}

		switch (vertexIndex)
		{
			case 0:
				layoutElement.SetMinOffset(layoutElement.GetMinOffset() + TVector2(delta.x, 0.0f));
				layoutElement.SetMaxOffset(layoutElement.GetMaxOffset() + TVector2(delta.x, -delta.y));
				break;
			case 1:
				layoutElement.SetMaxOffset(layoutElement.GetMaxOffset() + TVector2(delta.x, -delta.y));
				break;
			case 2:
				layoutElement.SetMinOffset(layoutElement.GetMinOffset() + TVector2(0.0f, delta.y));
				layoutElement.SetMaxOffset(layoutElement.GetMaxOffset() + TVector2(delta.x, 0.0f));
				break;
			case 3:
				layoutElement.SetMinOffset(layoutElement.GetMinOffset() + delta);
				break;
		}
	}


	static void DrawLayoutElementPivot(IImGUIContext& imguiContext, CLayoutElement& layoutElement, F32 handleRadius, const TRectF32& worldRect, F32 canvasHeight)
	{
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
				auto&& mousePosition = PointToNormalizedCoords(worldRect, imguiContext.GetInvertedMousePosition(), false);
				layoutElement.SetPivot(TVector2(mousePosition.x, mousePosition.y));
			}
		});
	}


	static TRectF32 DrawLayoutElementAnchors(IImGUIContext& imguiContext, CLayoutElement& layoutElement, F32 handleRadius, const TVector2& anchorSizes, const TRectF32& worldRect, F32 canvasHeight)
	{
		enum class E_ANCHOR_TYPE : U8
		{
			LEFT_BOTTOM,
			RIGHT_BOTTOM,
			RIGHT_TOP,
			LEFT_TOP,
		};

		/// \note Draw anchors
		auto parentWorldRect = layoutElement.GetParentWorldRect();
		auto anchorsRelativeWorldRect = layoutElement.GetAnchorWorldRect();

		auto&& anchorsRectPoints = anchorsRelativeWorldRect.GetPoints();

		static const std::array<TVector2, 4> anchorRectsOffsets{ TVector2(-5.0f * handleRadius, 0.0f), ZeroVector2, TVector2(0.0f, -5.0f * handleRadius), TVector2(-5.0f * handleRadius) };

		for (U8 i = 0; i < anchorsRectPoints.size(); ++i)
		{
			auto p = anchorsRectPoints[i];
			p = TVector2(p.x, canvasHeight - p.y); // transform from UI space to screen space

			const F32 s0 = (i % 3 == 0 ? -1.0f : 1.0f);
			const F32 s1 = (i < 2 ? 1.0f : -1.0f);

			imguiContext.DisplayIDGroup(static_cast<U32>(10 + i), [&imguiContext, &anchorSizes, &parentWorldRect, &layoutElement, s0, s1, p, handleRadius, i]
			{
				imguiContext.DrawTriangle(p + TVector2(s0 * anchorSizes.x, s1 * anchorSizes.y), p + TVector2(s0 * anchorSizes.y, s1 * anchorSizes.x), p, TColorUtils::mWhite, false, 1.f);

				imguiContext.SetCursorScreenPos(p + anchorRectsOffsets[i]);
				imguiContext.Button(Wrench::StringUtils::GetEmptyStr(), TVector2(5.0f * handleRadius), nullptr, true);

				if (imguiContext.IsItemActive() && imguiContext.IsMouseDragging(0))
				{
					auto normalizedAnchorPos = PointToNormalizedCoords(parentWorldRect, imguiContext.GetInvertedMousePosition());
					
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

		TRectF32 rect = worldRect;
		rect.y = canvasHeight - (rect.y + rect.height);

		return rect;
	}


	static void DrawLayoutElementFrameHandle(IImGUIContext& imguiContext, CLayoutElement& layoutElement, F32 handleRadius, const TRectF32& worldRect, F32 canvasHeight)
	{
		/// \note Draw corner vertices
		U32 pointIndex = 0;

		for (auto&& currPoint : worldRect.GetPoints())
		{
			imguiContext.DisplayIDGroup(3 + pointIndex++, [&imguiContext, &layoutElement, currPoint, handleRadius, canvasHeight, pointIndex]
			{
				imguiContext.DrawCircle(currPoint, handleRadius, true, TColorUtils::mBlue);

				imguiContext.SetCursorScreenPos(currPoint - TVector2(5.0f * handleRadius));
				imguiContext.Button(Wrench::StringUtils::GetEmptyStr(), TVector2(10.0f * handleRadius), nullptr, true);

				if (imguiContext.IsItemActive() && imguiContext.IsMouseDragging(0))
				{
					SetRectangleSizesForLayoutElement(layoutElement, -imguiContext.GetMouseDragDelta(0), pointIndex);
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
					SetPositionForLayoutElement(imguiContext, layoutElement);
				}
			}
		});
	}


	static void DrawLayoutElementHandles(const TEditorContext& editorContext, CLayoutElement& layoutElement)
	{
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

			DrawLayoutElementPivot(imguiContext, layoutElement, handleRadius, worldRect, canvasHeight);
			worldRect = DrawLayoutElementAnchors(imguiContext, layoutElement, handleRadius, anchorSizes, worldRect, canvasHeight);
			DrawLayoutElementFrameHandle(imguiContext, layoutElement, handleRadius, worldRect, canvasHeight);
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


	static const std::array<std::tuple<TVector2, TVector2, TVector2, TVector2>, LayoutPresetsCount> LayoutAnchorsPresets
	{
		std::make_tuple<TVector2, TVector2, TVector2, TVector2>({ 0.0f, 1.0f }, { 0.0f, 1.0f }, { 100.0f }, { 0.0f, -100.0f }),       ///< Top Left
		std::make_tuple<TVector2, TVector2, TVector2, TVector2>({ 1.0f }, { 1.0f }, { 100.0f }, { -100.0f }),                   ///< Top Right
		std::make_tuple<TVector2, TVector2, TVector2, TVector2>({ 0.0f }, { 0.0f }, { 100.0f }, { 0.0f, 0.0f }),                   ///< Bottom Left
		std::make_tuple<TVector2, TVector2, TVector2, TVector2>({ 1.0f, 0.0f }, { 1.0f, 0.0f }, { 100.0f }, { -100.0f, 0.0f }),       ///< Bottom Right
		std::make_tuple<TVector2, TVector2, TVector2, TVector2>({ 0.0f, 0.5f }, { 0.0f, 0.5f }, { 100.0f }, { 0.0f, 0.0f }),       ///< Center Left
		std::make_tuple<TVector2, TVector2, TVector2, TVector2>({ 0.5f, 1.0f }, { 0.5f, 1.0f }, { 100.0f }, { 0.0f, 0.0f }),       ///< Center Top
		std::make_tuple<TVector2, TVector2, TVector2, TVector2>({ 1.0f, 0.5f }, { 1.0f, 0.5f }, { 100.0f }, { 0.0f, 0.0f }),       ///< Center Right
		std::make_tuple<TVector2, TVector2, TVector2, TVector2>({ 0.5f, 0.0f }, { 0.5f, 0.0f }, { 100.0f }, { 0.0f, 0.0f }),       ///< Center Bottom
		std::make_tuple<TVector2, TVector2, TVector2, TVector2>({ 0.5f }, { 0.5f }, { 100.0f }, { 0.0f, 0.0f }),                   ///< Center
		std::make_tuple<TVector2, TVector2, TVector2, TVector2>({ 0.0f }, { 0.0f, 1.0f }, { 100.0f, 0.0f }, { 0.0f, 0.0f }),       ///< Left Wide
		std::make_tuple<TVector2, TVector2, TVector2, TVector2>({ 0.5f, 0.0f }, { 0.5f, 1.0f }, { 100.0f, 0.0f }, { 0.0f, 0.0f }), ///< VCenter Wide
		std::make_tuple<TVector2, TVector2, TVector2, TVector2>({ 0.0f, 0.5f }, { 1.0f, 0.5f }, { 0.0f, 100.0f }, { 0.0f, 0.0f }), ///< HCenter Wide
		std::make_tuple<TVector2, TVector2, TVector2, TVector2>({ 1.0f, 0.0f }, { 1.0f }, { 100.0f, 0.0f }, { 0.0f, 0.0f }),       ///< Right Wide
		std::make_tuple<TVector2, TVector2, TVector2, TVector2>({ 0.0f, 1.0f }, { 1.0f }, { 0.0f, 100.0f }, { 0.0f, 0.0f }),       ///< Top Wide
		std::make_tuple<TVector2, TVector2, TVector2, TVector2>({ 0.0f }, { 1.0f, 0.0f }, { 0.0f, 100.0f }, { 0.0f, 0.0f }),       ///< Bottom Wide
		std::make_tuple<TVector2, TVector2, TVector2, TVector2>({ 0.0f }, { 1.0f }, { 100.0f }, { 0.0f, 0.0f }),                   ///< Full Rect
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
			layoutElement.SetDirty(true);

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

								layoutElement.SetMinOffset(std::get<3>(anchorsPreset));
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

	void CDefeaultInspectorsRegistry::DrawInputReceiverGUI(const TEditorContext& editorContext)
	{
		IImGUIContext& imguiContext = editorContext.mImGUIContext;
		IComponent& component = editorContext.mComponent;

		if (imguiContext.CollapsingHeader("Input Receiver", true))
		{
			CInputReceiver& inputReceiver = dynamic_cast<CInputReceiver&>(component);

			{
				bool ignoreInput = inputReceiver.IsIgnoreInputFlag();

				imguiContext.BeginHorizontal();
				imguiContext.Label("Ignore Input");
				imguiContext.Checkbox("##IgnoreInput", ignoreInput);
				imguiContext.EndHorizontal();

				if (inputReceiver.IsIgnoreInputFlag() != ignoreInput)
				{
					inputReceiver.SetIgnoreInputFlag(ignoreInput);
				}
			}

			imguiContext.Label(Wrench::StringUtils::Format("On Pressed: {0}", inputReceiver.IsPressed()));
		}
	}

	void CDefeaultInspectorsRegistry::DrawLabelGUI(const TEditorContext& editorContext)
	{
		IImGUIContext& imguiContext = editorContext.mImGUIContext;
		IComponent& component = editorContext.mComponent;

		if (imguiContext.CollapsingHeader("Label", true))
		{
			CLabel& label = dynamic_cast<CLabel&>(component);

			{
				std::string text = label.GetText();

				imguiContext.BeginHorizontal();
				imguiContext.Label("Text");
				imguiContext.TextField("##Text", text, [&label](auto&& value) { label.SetText(value); });
				imguiContext.EndHorizontal();
			}

			/// font parameters
			bool isFontSectionOpened = false;
			bool isFontSectionSelected = false;

			if (std::get<0>(std::tie(isFontSectionOpened, isFontSectionSelected) = imguiContext.BeginTreeNode("Font Parameters", false)))
			{
				{
					std::string fontId = label.GetFontId();

					imguiContext.BeginHorizontal();
					imguiContext.Label("Font Resource");
					imguiContext.SetItemWidth(imguiContext.GetWindowWidth() * 0.5f, [&imguiContext, &fontId, &label]
					{
						imguiContext.TextField("##FontId", fontId, [&label](auto&& value) { label.SetFontId(value); });
					});
					imguiContext.EndHorizontal();
				}

				imguiContext.EndTreeNode();
			}

			/// text parameters
			{
				static std::vector<std::string> textAlignTypes;
				if (textAlignTypes.empty())
				{
					for (auto&& currValueInfo : Meta::EnumTrait<E_FONT_ALIGN_POLICY>::GetFields())
					{
						textAlignTypes.push_back(currValueInfo.name);
					}
				}

				imguiContext.BeginHorizontal();

				I32 currAlignType = static_cast<U32>(label.GetAlignType());

				imguiContext.Label("Text Align");
				currAlignType = imguiContext.Popup("##AlignType", currAlignType, textAlignTypes);

				label.SetAlignType(static_cast<E_FONT_ALIGN_POLICY>(currAlignType));

				imguiContext.EndHorizontal();
			}

			/// \note Text overflow policy
			{
				static std::vector<std::string> overflowPolicies;
				if (overflowPolicies.empty())
				{
					for (auto&& currValueInfo : Meta::EnumTrait<E_TEXT_OVERFLOW_POLICY>::GetFields())
					{
						overflowPolicies.push_back(currValueInfo.name);
					}
				}

				imguiContext.BeginHorizontal();

				I32 currTextOverflowType = static_cast<U32>(label.GetOverflowPolicyType());

				imguiContext.SetItemWidth(imguiContext.GetWindowWidth() * 0.4f, [&currTextOverflowType, &imguiContext]
				{
					imguiContext.Label("Text Overflow Rule");
					currTextOverflowType = imguiContext.Popup("##TextOverflow", currTextOverflowType, overflowPolicies);
				});

				label.SetOverflowPolicyType(static_cast<E_TEXT_OVERFLOW_POLICY>(currTextOverflowType));

				imguiContext.EndHorizontal();
			}
		}
	}

	void CDefeaultInspectorsRegistry::Draw9SliceImageGUI(const TEditorContext& editorContext)
	{
		IImGUIContext& imguiContext = editorContext.mImGUIContext;
		IComponent& component = editorContext.mComponent;

		if (imguiContext.CollapsingHeader("9SliceImage", true))
		{
			C9SliceImage& slicedImage = dynamic_cast<C9SliceImage&>(component);

			/// \note Margin field
			{
				F32 marginValue = slicedImage.GetRelativeBorderSize();

				imguiContext.BeginHorizontal();
				imguiContext.Label("Relative Margin: ");
				imguiContext.FloatSlider("##margin", marginValue, 0.0f, 0.5f, [&marginValue, &slicedImage] { slicedImage.SetRelativeBorderSize(marginValue); });
				imguiContext.EndHorizontal();
			}

			const F32 previewSizes = imguiContext.GetWindowWidth() * 0.7f;

			imguiContext.Label("Image Preview:");
			
			const TVector2 cursorPos = imguiContext.GetCursorScreenPos();

			imguiContext.Image(slicedImage.GetImageResourceId(), TVector2(previewSizes));
			imguiContext.DrawRect(TRectF32(cursorPos.x, cursorPos.y, previewSizes, previewSizes), TColorUtils::mRed, false);

			/// \note Draw handles
			{
				const TVector2 leftXSlicerStart = cursorPos + TVector2(previewSizes * slicedImage.GetLeftXSlicer(), 0.0f);
				imguiContext.DrawLine(leftXSlicerStart, leftXSlicerStart + TVector2(0.0f, previewSizes), TColorUtils::mGreen);

				const TVector2 rightXSlicerStart = cursorPos + TVector2(previewSizes * slicedImage.GetRightXSlicer(), 0.0f);
				imguiContext.DrawLine(rightXSlicerStart, rightXSlicerStart + TVector2(0.0f, previewSizes), TColorUtils::mGreen);

				const TVector2 bottomYSlicerStart = cursorPos + TVector2(0.0f, previewSizes * (1.0f - slicedImage.GetBottomYSlicer()));
				imguiContext.DrawLine(bottomYSlicerStart, bottomYSlicerStart + TVector2(previewSizes, 0.0f), TColorUtils::mGreen);

				const TVector2 topYSlicerStart = cursorPos + TVector2(0.0f, previewSizes * (1.0f - slicedImage.GetTopYSlicer()));
				imguiContext.DrawLine(topYSlicerStart, topYSlicerStart + TVector2(previewSizes, 0.0f), TColorUtils::mGreen);
			}			

			/// \note left x slicer's slider
			{
				F32 xStart = slicedImage.GetLeftXSlicer();

				imguiContext.BeginHorizontal();
				imguiContext.Label("Left X: ");
				imguiContext.FloatSlider("##xStart", xStart, 0.0f, 1.0f, [&xStart, &slicedImage] { slicedImage.SetLeftXSlicer(xStart); });
				imguiContext.EndHorizontal();
			}

			/// \note right x slicer's slider
			{
				F32 xEnd = slicedImage.GetRightXSlicer();

				imguiContext.BeginHorizontal();
				imguiContext.Label("Right X: ");
				imguiContext.FloatSlider("##xEnd", xEnd, 0.0f, 1.0f, [&xEnd, &slicedImage] { slicedImage.SetRightXSlicer(xEnd); });
				imguiContext.EndHorizontal();
			}

			/// \note bottom y slicer's slider
			{
				F32 yStart = slicedImage.GetBottomYSlicer();

				imguiContext.BeginHorizontal();
				imguiContext.Label("Bottom Y: ");
				imguiContext.FloatSlider("##yStart", yStart, 0.0f, 1.0f, [&yStart, &slicedImage] { slicedImage.SetBottomYSlicer(yStart); });
				imguiContext.EndHorizontal();
			}

			/// \note top y slicer's slider
			{
				F32 yEnd = slicedImage.GetTopYSlicer();

				imguiContext.BeginHorizontal();
				imguiContext.Label("Top Y: ");
				imguiContext.FloatSlider("##yEnd", yEnd, 0.0f, 1.0f, [&yEnd, &slicedImage] { slicedImage.SetTopYSlicer(yEnd); });
				imguiContext.EndHorizontal();
			}
		}
	}

	void CDefeaultInspectorsRegistry::DrawGridGroupLayoutGUI(const TEditorContext& editorContext)
	{
		IImGUIContext& imguiContext = editorContext.mImGUIContext;

		if (imguiContext.CollapsingHeader("Grid Group Layout", true))
		{
			CGridGroupLayout& gridGroupLayout = dynamic_cast<CGridGroupLayout&>(editorContext.mComponent);

			/// \note cell size of the grid's element
			{
				TVector2 cellSize = gridGroupLayout.GetCellSize();

				imguiContext.BeginHorizontal();
				imguiContext.Label("Cell Size: ");
				imguiContext.Vector2Field("##cellSize", cellSize, [&cellSize, &gridGroupLayout] { gridGroupLayout.SetCellSize(cellSize); });
				imguiContext.EndHorizontal();
			}

			/// \note spacing parameters of the grid
			{
				TVector2 spacing = gridGroupLayout.GetSpaceBetweenElements();

				imguiContext.BeginHorizontal();
				imguiContext.Label("Spacing: ");
				imguiContext.Vector2Field("##spacing", spacing, [&spacing, &gridGroupLayout] { gridGroupLayout.SetSpaceBetweenElements(spacing); });
				imguiContext.EndHorizontal();
			}

			/// \note align type of grid cells
			{
				static std::vector<std::string> alignTypes;
				if (alignTypes.empty())
				{
					for (auto&& currValueInfo : Meta::EnumTrait<E_UI_ELEMENT_ALIGNMENT_TYPE>::GetFields())
					{
						alignTypes.push_back(currValueInfo.name);
					}
				}

				imguiContext.BeginHorizontal();

				I32 currAlignType = static_cast<I32>(gridGroupLayout.GetElementsAlignType());

				imguiContext.Label("Elements Align");
				currAlignType = imguiContext.Popup("##CellAlignType", currAlignType, alignTypes);

				gridGroupLayout.SetElementsAlignType(static_cast<E_UI_ELEMENT_ALIGNMENT_TYPE>(currAlignType));

				imguiContext.EndHorizontal();
			}

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
	
	void CDefeaultInspectorsRegistry::DrawLODStrategyGUI(const TEditorContext& editorContext)
	{
		IImGUIContext& imguiContext = editorContext.mImGUIContext;
		IComponent& component = editorContext.mComponent;

		if (imguiContext.CollapsingHeader("LOD Strategy", true))
		{
			CLODStrategyComponent& lodStrategy = dynamic_cast<CLODStrategyComponent&>(component);

			lodStrategy.ForEachInstance([&lodStrategy, &imguiContext](USIZE id, TLODInstanceInfo& lodInfo)
			{
				imguiContext.DisplayIDGroup(static_cast<I32>(id), [&lodStrategy, &imguiContext, &lodInfo]
				{
					/// Distance of switching
					{
						F32 distance = lodInfo.mSwitchDistance;

						imguiContext.BeginHorizontal();
						imguiContext.Label("Switch Distance: ");
						imguiContext.FloatField("##SwitchDistance", distance, [&lodStrategy, &distance, &lodInfo]
						{ 
							if (distance != lodInfo.mSwitchDistance)
							{
								lodInfo.mSwitchDistance = distance;
								lodStrategy.Sort();
							}

						});
						imguiContext.EndHorizontal();
					}

					/// \note Mesh identifier
					{
						bool isMeshParamActive = (lodInfo.mActiveParams & E_LOD_INSTANCE_ACTIVE_PARAMS::MESH_ID) == E_LOD_INSTANCE_ACTIVE_PARAMS::MESH_ID;

						imguiContext.BeginHorizontal();
						imguiContext.Checkbox("##IsMeshParamActive", isMeshParamActive);
						imguiContext.Label("Mesh Id:");
						imguiContext.TextField("##MeshId", lodInfo.mMeshId);
						imguiContext.EndHorizontal();

						if (isMeshParamActive)
						{
							lodInfo.mActiveParams = lodInfo.mActiveParams | E_LOD_INSTANCE_ACTIVE_PARAMS::MESH_ID;
						}
						else
						{
							lodInfo.mActiveParams = static_cast<E_LOD_INSTANCE_ACTIVE_PARAMS>(static_cast<U8>(lodInfo.mActiveParams) & ~static_cast<U8>(E_LOD_INSTANCE_ACTIVE_PARAMS::MESH_ID));
						}
					}

					/// \note Sub-mesh idenfitier
					{
						bool isSubmeshParamActive = (lodInfo.mActiveParams & E_LOD_INSTANCE_ACTIVE_PARAMS::SUBMESH_ID) == E_LOD_INSTANCE_ACTIVE_PARAMS::SUBMESH_ID;

						imguiContext.BeginHorizontal();
						imguiContext.Checkbox("##IsSubMeshParamActive", isSubmeshParamActive);
						imguiContext.Label("Sub-mesh Id:");
						imguiContext.TextField("##SubMeshId", lodInfo.mSubMeshId);
						imguiContext.EndHorizontal();

						if (isSubmeshParamActive)
						{
							lodInfo.mActiveParams = lodInfo.mActiveParams | E_LOD_INSTANCE_ACTIVE_PARAMS::SUBMESH_ID;
						}
						else
						{
							lodInfo.mActiveParams = static_cast<E_LOD_INSTANCE_ACTIVE_PARAMS>(static_cast<U8>(lodInfo.mActiveParams) & ~static_cast<U8>(E_LOD_INSTANCE_ACTIVE_PARAMS::SUBMESH_ID));
						}
					}

					/// \note Material
					{
						bool isMaterialParamActive = (lodInfo.mActiveParams & E_LOD_INSTANCE_ACTIVE_PARAMS::MATERIAL_ID) == E_LOD_INSTANCE_ACTIVE_PARAMS::MATERIAL_ID;

						imguiContext.BeginHorizontal();
						imguiContext.Checkbox("##IsMaterialParamActive", isMaterialParamActive);
						imguiContext.Label("Material Id:");
						imguiContext.TextField("##MaterialId", lodInfo.mMaterialId);
						imguiContext.EndHorizontal();

						if (isMaterialParamActive)
						{
							lodInfo.mActiveParams = lodInfo.mActiveParams | E_LOD_INSTANCE_ACTIVE_PARAMS::MATERIAL_ID;
						}
						else
						{
							lodInfo.mActiveParams = static_cast<E_LOD_INSTANCE_ACTIVE_PARAMS>(static_cast<U8>(lodInfo.mActiveParams) & ~static_cast<U8>(E_LOD_INSTANCE_ACTIVE_PARAMS::MATERIAL_ID));
						}
					}
				});

				if (imguiContext.Button(Wrench::StringUtils::Format("Remove LOD {0}", id), TVector2(imguiContext.GetWindowWidth() * 0.45f, 20.0f)))
				{
					E_RESULT_CODE result = lodStrategy.RemoveLODInstance(static_cast<U32>(id));
					TDE2_ASSERT(RC_OK == result);
					
					return false; /// Interrupt the iteration loop to prevent corruption of the iterator
				}

				return true;
			});

			if (imguiContext.Button("Add LOD", TVector2(imguiContext.GetWindowWidth() * 0.95f, 25.0f)))
			{
				lodStrategy.AddLODInstance({});
			}
		}
	}
}

#endif