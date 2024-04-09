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
#include "../../include/graphics/UI/CToggleComponent.h"
#include "../../include/graphics/UI/CUISliderComponent.h"
#include "../../include/graphics/UI/CScrollableUIAreaComponent.h"
#include "../../include/graphics/UI/CDropDownComponent.h"
#include "../../include/graphics/animation/CAnimationContainerComponent.h"
#include "../../include/graphics/animation/CMeshAnimatorComponent.h"
#include "../../include/graphics/CPerspectiveCamera.h"
#include "../../include/graphics/COrthoCamera.h"
#include "../../include/physics/2D/CBoxCollisionObject2D.h"
#include "../../include/physics/2D/CCircleCollisionObject2D.h"
#include "../../include/physics/2D/CTrigger2D.h"
#include "../../include/scene/components/ShadowMappingComponents.h"
#include "../../include/scene/components/CPointLight.h"
#include "../../include/scene/components/CDirectionalLight.h"
#include "../../include/scene/components/CSpotLight.h"
#include "../../include/scene/components/AudioComponents.h"
#include "../../include/scene/components/CLODStrategyComponent.h"
#include "../../include/scene/components/CWeatherComponent.h"
#include "../../include/scene/components/CSplashScreenItemComponent.h"
#include "../../include/editor/ecs/EditorComponents.h"
#include "../../include/editor/CLevelEditorWindow.h"
#include "../../include/editor/CEditorActionsManager.h"
#include "../../include/editor/EditorUtils.h"
#include "../../include/editor/CEditorSettings.h"
#include "../../include/utils/CFileLogger.h"
#define META_EXPORT_UI_SECTION
#include "../../include/metadata.h"
#include <array>
#include <tuple>
#include <functional>


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	template <typename TFunctionType>
	void Header(const std::string& headerText, const TEditorContext& editorContext, TFunctionType action)
	{
		IImGUIContext& imguiContext = editorContext.mImGUIContext;

		if (imguiContext.CollapsingHeader(headerText, true))
		{
			imguiContext.DisplayIDGroup(static_cast<I32>(editorContext.mCurrEntityId), [&imguiContext, &editorContext]
			{
				imguiContext.DisplayContextMenu(Wrench::StringUtils::GetEmptyStr(), [&editorContext](IImGUIContext& imgui)
				{
					if (CTransform::GetTypeId() != editorContext.mComponent.GetComponentTypeId()) /// CTransform couldn't be removed from any entity
					{
						imgui.MenuItem("Remove Component", Wrench::StringUtils::GetEmptyStr(), [&editorContext]
						{
							if (auto pEntity = editorContext.mWorld.FindEntity(editorContext.mCurrEntityId))
							{
								pEntity->RemoveComponent(editorContext.mComponent.GetComponentTypeId());
							}
						});
					}
				});
			});

			action(editorContext);
		}
	}


	static void DrawEmptyGUI(const TEditorContext& editorContext)
	{
	}

	static void DrawTransformGUI(const TEditorContext& editorContext)
	{
		Header("Transform", editorContext, [](const TEditorContext& editorContext)
		{
			IImGUIContext& imguiContext = editorContext.mImGUIContext;
			IComponent& component = editorContext.mComponent;
			IEditorActionsHistory& actionsHistory = editorContext.mActionsHistory;

			CTransform& transform = dynamic_cast<CTransform&>(component);

			TVector3 pivot = transform.GetPivot();
			TVector3 position = transform.GetPosition();
			TVector3 rotation = ToEulerAngles(transform.GetRotation()) * CMathConstants::Rad2Deg;
			TVector3 scale = transform.GetScale();

			imguiContext.BeginHorizontal();
			imguiContext.Label("Pivot");
			imguiContext.Vector3Field("##Pivot", pivot, [&transform, &pivot, &rotation, &scale, &editorContext, &actionsHistory]
			{
				//transform.SetPivot(pivot);
			});
			imguiContext.EndHorizontal();

			imguiContext.BeginHorizontal();
			imguiContext.Label("Position");
			imguiContext.Vector3Field("##1", position, [&transform, &position, &rotation, &scale, &editorContext, &actionsHistory]
			{
				E_RESULT_CODE result = RC_OK;

				if (auto pAction = CreateTransformObjectAction(&editorContext.mWorld, editorContext.mCurrEntityId, { position, TQuaternion(rotation * CMathConstants::Deg2Rad), scale }, result))
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

				if (auto pAction = CreateTransformObjectAction(&editorContext.mWorld, editorContext.mCurrEntityId, { position, TQuaternion(rotation * CMathConstants::Deg2Rad), scale }, result))
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

				if (auto pAction = CreateTransformObjectAction(&editorContext.mWorld, editorContext.mCurrEntityId, { position, TQuaternion(rotation * CMathConstants::Deg2Rad), scale }, result))
				{
					PANIC_ON_FAILURE(actionsHistory.PushAndExecuteAction(pAction));
				}

				transform.SetScale(scale);
			});
			imguiContext.EndHorizontal();
		});
	}

	static void DrawBoundsGUI(const TEditorContext& editorContext)
	{
		Header("Bounds", editorContext, [](const TEditorContext& editorContext)
		{
			IImGUIContext& imguiContext = editorContext.mImGUIContext;
			IComponent& component = editorContext.mComponent;

			CBoundsComponent& bounds = dynamic_cast<CBoundsComponent&>(component);

			// \todo Implement this drawer
		});
	}

	static void DrawQuadSpriteGUI(const TEditorContext& editorContext)
	{
		Header("QuadSprite", editorContext, [](const TEditorContext& editorContext)
		{
			IImGUIContext& imguiContext = editorContext.mImGUIContext;
			IComponent& component = editorContext.mComponent;

			CQuadSprite& sprite = dynamic_cast<CQuadSprite&>(component);

			imguiContext.Label(sprite.GetMaterialName());
			// \todo Implement this drawer
		});
	}

	static void DrawStaticMeshContainerGUI(const TEditorContext& editorContext)
	{
		Header("Static Mesh Container", editorContext, [](const TEditorContext& editorContext)
		{
			IImGUIContext& imguiContext = editorContext.mImGUIContext;
			IComponent& component = editorContext.mComponent;

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
				if (!submeshes.empty())
				{
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
		});
	}

	static void DrawSkinnedMeshContainerGUI(const TEditorContext& editorContext)
	{
		Header("Skinned Mesh Container", editorContext, [](const TEditorContext& editorContext)
		{
			IImGUIContext& imguiContext = editorContext.mImGUIContext;
			IComponent& component = editorContext.mComponent;

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
				std::string newResult = submeshes.empty() ? Wrench::StringUtils::GetEmptyStr() : submeshes[imguiContext.Popup("##SubMeshId", index, submeshes)];

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
		});
	}

	static void DrawShadowReceiverGUI(const TEditorContext& editorContext)
	{
		Header("Shadow Receiver", editorContext, [](const TEditorContext& editorContext)
		{
			IImGUIContext& imguiContext = editorContext.mImGUIContext;
			IComponent& component = editorContext.mComponent;

			CShadowReceiverComponent& shadowReceiver = dynamic_cast<CShadowReceiverComponent&>(component);

			// \todo Implement this drawer
		});
	}

	static void DrawShadowCasterGUI(const TEditorContext& editorContext)
	{
		Header("Shadow Caster", editorContext, [](const TEditorContext& editorContext)
		{
			IImGUIContext& imguiContext = editorContext.mImGUIContext;
			IComponent& component = editorContext.mComponent;

			// \todo Implement this drawer
		});
	}

	static void DrawDirectionalLightGUI(const TEditorContext& editorContext)
	{
		Header("Directional Light", editorContext, [](const TEditorContext& editorContext)
		{
			IImGUIContext& imguiContext = editorContext.mImGUIContext;
			IComponent& component = editorContext.mComponent;

			CDirectionalLight& dirLight = dynamic_cast<CDirectionalLight&>(component);

			// \todo Implement this drawer

		});
	}

	static void DrawPointLightGUI(const TEditorContext& editorContext)
	{
		Header("Point Light", editorContext, [](const TEditorContext& editorContext)
		{
			IImGUIContext& imguiContext = editorContext.mImGUIContext;
			IComponent& component = editorContext.mComponent;

			CPointLight& pointLight = dynamic_cast<CPointLight&>(component);

			/// \note color
			{
				TColor32F color = pointLight.GetColor();

				imguiContext.BeginHorizontal();
				imguiContext.Label("Color: ");
				imguiContext.ColorPickerField("##Color", color, [&pointLight, &color] { pointLight.SetColor(color); });
				imguiContext.EndHorizontal();
			}

			/// \note range
			{
				F32 range = pointLight.GetRange();

				imguiContext.BeginHorizontal();
				imguiContext.Label("Range: ");
				imguiContext.FloatField("##Range", range, [&pointLight, &range] { pointLight.SetRange(range); });
				imguiContext.EndHorizontal();
			}

			/// \note intensity
			{
				F32 intensity = pointLight.GetIntensity();

				imguiContext.BeginHorizontal();
				imguiContext.Label("Intensity: ");
				imguiContext.FloatField("##Intensity", intensity, [&pointLight, &intensity] { pointLight.SetIntensity(intensity); });
				imguiContext.EndHorizontal();
			}
		});
	}

	static void DrawSpotLightGUI(const TEditorContext& editorContext)
	{
		Header("Spot Light", editorContext, [](const TEditorContext& editorContext)
		{
			IImGUIContext& imguiContext = editorContext.mImGUIContext;
			IComponent& component = editorContext.mComponent;

			CSpotLight& spotLight = dynamic_cast<CSpotLight&>(component);

			/// \note color
			{
				TColor32F color = spotLight.GetColor();

				imguiContext.BeginHorizontal();
				imguiContext.Label("Color: ");
				imguiContext.ColorPickerField("##Color", color, [&spotLight, &color] { spotLight.SetColor(color); });
				imguiContext.EndHorizontal();
			}

			/// \note angle
			{
				F32 angle = spotLight.GetAngle();

				imguiContext.BeginHorizontal();
				imguiContext.Label("Angle: ");
				imguiContext.FloatField("##Angle", angle, [&spotLight, &angle] { spotLight.SetAngle(angle); });
				imguiContext.EndHorizontal();
			}

			/// \note range
			{
				F32 range = spotLight.GetRange();

				imguiContext.BeginHorizontal();
				imguiContext.Label("Range: ");
				imguiContext.FloatField("##Range", range, [&spotLight, &range] { spotLight.SetRange(range); });
				imguiContext.EndHorizontal();
			}

			/// \note intensity
			{
				F32 intensity = spotLight.GetIntensity();

				imguiContext.BeginHorizontal();
				imguiContext.Label("Intensity: ");
				imguiContext.FloatField("##Intensity", intensity, [&spotLight, &intensity] { spotLight.SetIntensity(intensity); });
				imguiContext.EndHorizontal();
			}
		});
	}

	static void DrawAnimationContainerGUI(const TEditorContext& editorContext)
	{
		Header("Animation Container", editorContext, [](const TEditorContext& editorContext)
		{
			const TVector2 buttonSizes(100.0f, 25.0f);

			IImGUIContext& imguiContext = editorContext.mImGUIContext;
			IComponent& component = editorContext.mComponent;

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
		});
	}

	static void DrawSkyboxGUI(const TEditorContext& editorContext)
	{
		IImGUIContext& imguiContext = editorContext.mImGUIContext;
		IComponent& component = editorContext.mComponent;

		Header("Skybox", editorContext, [](const TEditorContext& editorContext) {});
	}

	static void DrawParticleEmitterGUI(const TEditorContext& editorContext)
	{
		Header("Particle Emitter", editorContext, [](const TEditorContext& editorContext)
		{
			const TVector2 buttonSizes(100.0f, 25.0f);

			IImGUIContext& imguiContext = editorContext.mImGUIContext;
			IComponent& component = editorContext.mComponent;

			CParticleEmitter& particleEmitter = dynamic_cast<CParticleEmitter&>(component);

			/// \note particle effect's identifier
			{
				std::string particleEffectId = particleEmitter.GetParticleEffectId();

				imguiContext.BeginHorizontal();
				imguiContext.Label("Effect Id:");
				imguiContext.TextField("##EffectId", particleEffectId, [&particleEmitter](auto&& value) { particleEmitter.SetParticleEffect(value); });
				imguiContext.EndHorizontal();
			}

			// \note Stop
			imguiContext.Button("Reset", buttonSizes, [&particleEmitter]
			{
				particleEmitter.mResetStateOnNextFrame = true;
			});
		});
	}

	static void DrawCanvasGUI(const TEditorContext& editorContext)
	{
		Header("Canvas", editorContext, [](const TEditorContext& editorContext)
		{
			IImGUIContext& imguiContext = editorContext.mImGUIContext;
			IComponent& component = editorContext.mComponent;

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

			I32 priority = canvas.GetPriority();

			imguiContext.BeginHorizontal();
			imguiContext.Label("Priority");
			imguiContext.IntField("##Priority", priority, [&canvas, &priority] { canvas.SetPriority(priority); });
			imguiContext.EndHorizontal();
		});
	}

	static void DrawMeshAnimatorGUI(const TEditorContext& editorContext)
	{
		Header("Mesh Animator", editorContext, [](const TEditorContext& editorContext)
		{
			IImGUIContext& imguiContext = editorContext.mImGUIContext;
			IComponent& component = editorContext.mComponent;

			CMeshAnimatorComponent& meshAnimator = dynamic_cast<CMeshAnimatorComponent&>(component);

			// \todo Implement this drawer
		});
	}


	static void DrawBaseCameraGUI(const TEditorContext& editorContext)
	{
		IImGUIContext& imguiContext = editorContext.mImGUIContext;
		CBaseCamera& camera = dynamic_cast<CBaseCamera&>(editorContext.mComponent);

		/// \note ZNear
		{
			F32 zNear = camera.GetNearPlane();

			imguiContext.BeginHorizontal();
			imguiContext.Label("Near Plane: ");
			imguiContext.FloatField("##ZNear", zNear, [&camera, &zNear] { camera.SetNearPlane(zNear); });
			imguiContext.EndHorizontal();
		}

		/// \note ZFar
		{
			F32 zFar = camera.GetFarPlane();

			imguiContext.BeginHorizontal();
			imguiContext.Label("Far Plane: ");
			imguiContext.FloatField("##ZFar", zFar, [&camera, &zFar] { camera.SetFarPlane(zFar); });
			imguiContext.EndHorizontal();
		}
	}


	static void DrawPerspectiveCameraGUI(const TEditorContext& editorContext)
	{
		Header("Perspective Camera", editorContext, [](const TEditorContext& editorContext)
		{
			IImGUIContext& imguiContext = editorContext.mImGUIContext;
			IComponent& component = editorContext.mComponent;

			CPerspectiveCamera& camera = dynamic_cast<CPerspectiveCamera&>(component);

			/// \note Fov
			{
				F32 fov = camera.GetFOV() * CMathConstants::Rad2Deg;

				imguiContext.BeginHorizontal();
				imguiContext.Label("Field of View: ");
				imguiContext.FloatField("##FOV", fov, [&camera, &fov] { camera.SetFOV(fov * CMathConstants::Deg2Rad); });
				imguiContext.EndHorizontal();
			}

			/// \note Aspect
			{
				F32 aspect = camera.GetAspect();

				imguiContext.BeginHorizontal();
				imguiContext.Label("Aspect: ");
				imguiContext.FloatField("##Aspect", aspect, [&camera, &aspect] { camera.SetAspect(aspect); });
				imguiContext.EndHorizontal();
			}

			DrawBaseCameraGUI(editorContext);
		});
	}

	static void DrawOrthographicCameraGUI(const TEditorContext& editorContext)
	{
		Header("Orthographic Camera", editorContext, [](const TEditorContext& editorContext)
		{
			IImGUIContext& imguiContext = editorContext.mImGUIContext;
			IComponent& component = editorContext.mComponent;

			COrthoCamera& camera = dynamic_cast<COrthoCamera&>(component);

			/// \note Width
			{
				F32 width = camera.GetWidth();

				imguiContext.BeginHorizontal();
				imguiContext.Label("Width: ");
				imguiContext.FloatField("##Width", width, [&camera, &width] { camera.SetWidth(width); });
				imguiContext.EndHorizontal();
			}

			/// \note Height
			{
				F32 height = camera.GetHeight();

				imguiContext.BeginHorizontal();
				imguiContext.Label("Height: ");
				imguiContext.FloatField("##Height", height, [&camera, &height] { camera.SetHeight(height); });
				imguiContext.EndHorizontal();
			}

			DrawBaseCameraGUI(editorContext);
		});
	}


	typedef std::vector<std::pair<CSnapGuidesContainer::TSnapGuideline, TVector2>> TPOISnapGuidelines;


	static TPOISnapGuidelines GetSnapGuidelines(const CSnapGuidesContainer* pGuidelinesContainer, const TRectF32& worldRect, const TVector2& pivot)
	{
		TPOISnapGuidelines guidelines;

		std::vector<TVector2> pointsOfInterest;

		auto&& rectPoints = worldRect.GetPoints();
		std::copy(rectPoints.begin(), rectPoints.end(), std::back_inserter(pointsOfInterest));

		pointsOfInterest.emplace_back(worldRect.GetLeftBottom() + pivot * worldRect.GetSizes());

		for (auto&& currPoint : pointsOfInterest)
		{
			auto&& output = pGuidelinesContainer->GetNearestSnapGuides(currPoint, CEditorSettings::Get()->mLevelEditorSettings.mGuidelinesDisplayThreshold);

			std::transform(output.begin(), output.end(), std::back_inserter(guidelines), [currPoint](const CSnapGuidesContainer::TSnapGuideline& guideline)
			{
				return std::make_pair(guideline, currPoint);
			});
		}

		return std::move(guidelines);
	}


	static void SetPositionForLayoutElement(IImGUIContext& imguiContext, CLayoutElement& layoutElement, const CSnapGuidesContainer* pGuidelinesContainer)
	{
		TVector2 delta = imguiContext.GetMouseDragDelta(0);
		delta.y = -delta.y;

		layoutElement.SetPositionOffset(delta);
		
		if (!CEditorSettings::Get()->mLevelEditorSettings.mIsGuidelinesSnapEnabled)
		{
			return;
		}

		const TRectF32& worldRect = layoutElement.GetWorldRect();

		TPOISnapGuidelines&& guidelines = GetSnapGuidelines(pGuidelinesContainer, worldRect, ZeroVector2);
		F32 smallestSnapDistance = CEditorSettings::Get()->mLevelEditorSettings.mSnapDistance;

		TVector2 offset = ZeroVector2;

		bool isHorizontalFound = false;
		bool isVerticalFound = false;

		for (auto&& currGuideInfo : guidelines)
		{
			const auto& guide = currGuideInfo.first;
			const auto& point = currGuideInfo.second;

			const TVector2 dir = Normalize(guide.mEnd - guide.mStart);
			const TVector2 normal(dir.y, dir.x);

			const F32 dist = Length(Dot(normal, (point - guide.mStart)) * normal);

			if (dist < smallestSnapDistance)
			{
				smallestSnapDistance = dist;

				const TVector2 snapDelta = guide.mStart - point;

				if (!isHorizontalFound && guide.IsHorizontal())
				{
					offset.y += snapDelta.y;
					isHorizontalFound = true;
				}

				if (!isVerticalFound && guide.IsVertical())
				{
					offset.x += snapDelta.x;
					isVerticalFound = true;
				}
			}
		}

		layoutElement.SetPositionOffset(delta + offset);
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


	static void DrawLayoutElementFrameHandle(IImGUIContext& imguiContext, CLayoutElement& layoutElement, F32 handleRadius, const TRectF32& worldRect, 
		F32 canvasHeight, const CSnapGuidesContainer* pGuidelinesContainer)
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
		imguiContext.DisplayIDGroup(2, [&imguiContext, &layoutElement, pGuidelinesContainer, worldRect, canvasHeight]
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
					SetPositionForLayoutElement(imguiContext, layoutElement, pGuidelinesContainer);
				}
			}
		});
	}


	static void DrawGuidelines(IImGUIContext& imguiContext, const CSnapGuidesContainer* pSnapGuidesContainer, const CLayoutElement& layoutElement, F32 canvasHeight)
	{
		if (!CEditorSettings::Get()->mLevelEditorSettings.mIsGuidelinesSnapEnabled)
		{
			return;
		}

		constexpr F32 lineLength = 1000.0f;

		auto worldRect = layoutElement.GetWorldRect();

		for (auto&& currSnapGuide : GetSnapGuidelines(pSnapGuidesContainer, worldRect, layoutElement.GetPivot()))
		{
			const auto& startPoint = currSnapGuide.first.mStart;

			const TVector2 dir = Normalize(currSnapGuide.first.mEnd - startPoint);

			imguiContext.DrawLine(
				TVector2(startPoint.x, canvasHeight - startPoint.y) - lineLength * dir,
				TVector2(startPoint.x, canvasHeight - startPoint.y) + lineLength * dir, 
				TColorUtils::mLightBlue, 1.0f);
		}
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
			ZeroVector2,
			true, true, false, true, true
		};

		bool opened = true;

		if (imguiContext.BeginWindow("LayoutElementEditor", opened, params))
		{
			auto worldRect = layoutElement.GetWorldRect();			

			DrawGuidelines(imguiContext, editorContext.mpGuidesController, layoutElement, canvasHeight);

			DrawLayoutElementPivot(imguiContext, layoutElement, handleRadius, worldRect, canvasHeight);
			worldRect = DrawLayoutElementAnchors(imguiContext, layoutElement, handleRadius, anchorSizes, worldRect, canvasHeight);
			DrawLayoutElementFrameHandle(imguiContext, layoutElement, handleRadius, worldRect, canvasHeight, editorContext.mpGuidesController);
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


	/// \brief Min anchor, max anchor, max offset, min offset
	static const std::array<std::tuple<TVector2, TVector2, TVector2, TVector2>, LayoutPresetsCount> LayoutAnchorsPresets
	{
		std::make_tuple<TVector2, TVector2, TVector2, TVector2>({ 0.0f, 1.0f }, { 0.0f, 1.0f }, { 100.0f }, { 0.0f, -100.0f }),			///< Top Left
		std::make_tuple<TVector2, TVector2, TVector2, TVector2>({ 1.0f }, { 1.0f }, { 100.0f }, { -100.0f }),							///< Top Right
		std::make_tuple<TVector2, TVector2, TVector2, TVector2>({ 0.0f }, { 0.0f }, { 100.0f }, { 0.0f }),								///< Bottom Left
		std::make_tuple<TVector2, TVector2, TVector2, TVector2>({ 1.0f, 0.0f }, { 1.0f, 0.0f }, { 100.0f }, { -100.0f, 0.0f }),			///< Bottom Right
		std::make_tuple<TVector2, TVector2, TVector2, TVector2>({ 0.0f, 0.5f }, { 0.0f, 0.5f }, { 100.0f }, { 0.0f, 0.0f }),			///< Center Left
		std::make_tuple<TVector2, TVector2, TVector2, TVector2>({ 0.5f, 1.0f }, { 0.5f, 1.0f }, { 100.0f }, { -50.0f, -100.0f }),       ///< Center Top
		std::make_tuple<TVector2, TVector2, TVector2, TVector2>({ 1.0f, 0.5f }, { 1.0f, 0.5f }, { 100.0f }, { -100.0f, -50.0f }),       ///< Center Right
		std::make_tuple<TVector2, TVector2, TVector2, TVector2>({ 0.5f, 0.0f }, { 0.5f, 0.0f }, { 100.0f }, { -50.0f, 0.0f }),			///< Center Bottom
		std::make_tuple<TVector2, TVector2, TVector2, TVector2>({ 0.5f }, { 0.5f }, { 100.0f }, { -50.0f }),							///< Center
		std::make_tuple<TVector2, TVector2, TVector2, TVector2>({ 0.0f }, { 0.0f, 1.0f }, { 100.0f, 0.0f }, { 0.0f, 0.0f }),			///< Left Wide
		std::make_tuple<TVector2, TVector2, TVector2, TVector2>({ 0.5f, 0.0f }, { 0.5f, 1.0f }, { 100.0f, 0.0f }, { -50.0f, 0.0f }),	///< VCenter Wide
		std::make_tuple<TVector2, TVector2, TVector2, TVector2>({ 0.0f, 0.5f }, { 1.0f, 0.5f }, { 0.0f, 100.0f }, { 0.0f, -50.0f }),	///< HCenter Wide
		std::make_tuple<TVector2, TVector2, TVector2, TVector2>({ 1.0f, 0.0f }, { 1.0f }, { 100.0f, 0.0f }, { -100.0f, 0.0f }),			///< Right Wide
		std::make_tuple<TVector2, TVector2, TVector2, TVector2>({ 0.0f, 1.0f }, { 1.0f }, { 0.0f, 100.0f }, { 0.0f, -100.0f }),			///< Top Wide
		std::make_tuple<TVector2, TVector2, TVector2, TVector2>({ 0.0f }, { 1.0f, 0.0f }, { 0.0f, 100.0f }, { 0.0f, 0.0f }),			///< Bottom Wide
		std::make_tuple<TVector2, TVector2, TVector2, TVector2>({ 0.0f }, { 1.0f }, { 0.0f }, { 0.0f }),								///< Full Rect
	};


	static void DrawLayoutPresetIcon(const TEditorContext& editorContext, const std::string& text, const TLayoutPresetParams& params = {}, bool isCurrentSelectedPreset = false)
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
			TVector2 { presetRectSize, innerRectSize },						 ///< VCenter Wide
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

		imguiContext.Button(isCurrentSelectedPreset ? "##LayoutPresetButton" : text + "##Button", TVector2(imguiContext.GetWindowWidth() * 0.4f, iconSize), params.mOnClick, true);

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


	static void DrawLayoutElementGUI(const TEditorContext& editorContext)
	{
		Header("Layout Element", editorContext, [](const TEditorContext& editorContext)
		{
			IImGUIContext& imguiContext = editorContext.mImGUIContext;
			IComponent& component = editorContext.mComponent;

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
				}, true);

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

			/// \note rotation angle
			{
				F32 angle = layoutElement.GetRotationAngle();

				imguiContext.BeginHorizontal();
				imguiContext.Label("Angle: ");
				imguiContext.FloatField("##Angle", angle, [&layoutElement, &angle] { layoutElement.SetRotationAngle(angle); });
				imguiContext.EndHorizontal();
			}

			// \note Scale
			{
				TVector2 scale = layoutElement.GetScale();

				imguiContext.BeginHorizontal();
				imguiContext.Label("Scale"); /// \todo Replace it 
				imguiContext.Vector2Field("##scale", scale, [&scale, &layoutElement] { layoutElement.SetScale(scale); });
				imguiContext.EndHorizontal();
			}
		});
	}

	static void DrawUIElementMeshDataGUI(const TEditorContext& editorContext)
	{
		Header("UI Element Mesh Data", editorContext, [](const TEditorContext& editorContext)
		{
			IImGUIContext& imguiContext = editorContext.mImGUIContext;
			IComponent& component = editorContext.mComponent;

			//CLayoutElement& layoutElement = dynamic_cast<CLayoutElement&>(component);

			// \todo Implement this drawer
		});
	}

	static void DrawImageGUI(const TEditorContext& editorContext)
	{
		Header("Image", editorContext, [](const TEditorContext& editorContext)
		{
			IImGUIContext& imguiContext = editorContext.mImGUIContext;
			IComponent& component = editorContext.mComponent;

			//CLayoutElement& layoutElement = dynamic_cast<CLayoutElement&>(component);

			CImage& image = dynamic_cast<CImage&>(component);

			/// \note Sprite identifier
			{
				std::string imageId = image.GetImageId();

				imguiContext.BeginHorizontal();
				imguiContext.Label("Sprite: ");
				imguiContext.TextField("##ImageId", imageId, [&image](auto&& value) { image.SetImageId(value); });
				imguiContext.EndHorizontal();
			}

			{
				TColor32F color = image.GetColor();

				imguiContext.BeginHorizontal();
				imguiContext.Label("Color: ");
				imguiContext.ColorPickerField("##ImageId", color, [&image, &color]() { image.SetColor(color); });
				imguiContext.EndHorizontal();
			}
		});
	}

	static void DrawInputReceiverGUI(const TEditorContext& editorContext)
	{
		Header("Input Receiver", editorContext, [](const TEditorContext& editorContext)
		{
			IImGUIContext& imguiContext = editorContext.mImGUIContext;
			IComponent& component = editorContext.mComponent;

			CInputReceiver& inputReceiver = dynamic_cast<CInputReceiver&>(component);

			{
				bool ignoreInput = inputReceiver.mIsIgnoreInput;

				imguiContext.BeginHorizontal();
				imguiContext.Label("Ignore Input");
				imguiContext.Checkbox("##IgnoreInput", ignoreInput);
				imguiContext.EndHorizontal();

				if (inputReceiver.mIsIgnoreInput != ignoreInput)
				{
					inputReceiver.mIsIgnoreInput = ignoreInput;
				}
			}

			{
				bool isBypassInputEnabled = inputReceiver.mIsInputBypassEnabled;

				imguiContext.BeginHorizontal();
				imguiContext.Label("Bypass Input");
				imguiContext.Checkbox("##BypassInput", isBypassInputEnabled);
				imguiContext.EndHorizontal();

				if (inputReceiver.mIsInputBypassEnabled != isBypassInputEnabled)
				{
					inputReceiver.mIsInputBypassEnabled = isBypassInputEnabled;
				}
			}

			if (std::get<0>(imguiContext.BeginTreeNode("Debug Info")))
			{
				imguiContext.Label(Wrench::StringUtils::Format(
					"On Pressed: {0}\nNormalized Input Pos: ({1}; {2})\nHovered: {3}\nFocused: {4}\nMouse delta: {5}",
					inputReceiver.mCurrState ? "pressed" : "none",
					inputReceiver.mNormalizedInputPosition.x,
					inputReceiver.mNormalizedInputPosition.y,
					inputReceiver.mIsHovered ? "true" : "false",
					inputReceiver.mIsFocused ? "true" : "false",
					inputReceiver.mMouseShiftVec.ToString()));

				imguiContext.EndTreeNode();
			}
		});
	}

	static void DrawLabelGUI(const TEditorContext& editorContext)
	{
		Header("Label", editorContext, [](const TEditorContext& editorContext)
		{
			IImGUIContext& imguiContext = editorContext.mImGUIContext;
			IComponent& component = editorContext.mComponent;

			CLabel& label = dynamic_cast<CLabel&>(component);

			{
				std::string text = label.GetText();

				imguiContext.BeginHorizontal();
				imguiContext.Label("Text");
				imguiContext.TextField("##Text", text, [&label](auto&& value) 
				{ 
					label.SetText(value); 
				});
				imguiContext.EndHorizontal();
			}

			/// font parameters
			bool isFontSectionOpened = false;
			bool isFontSectionSelected = false;

			if (std::get<0>(std::tie(isFontSectionOpened, isFontSectionSelected) = imguiContext.BeginTreeNode("Font Parameters", TColorUtils::mWhite, false)))
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
					for (auto&& currValueInfo : Meta::EnumTrait<E_FONT_ALIGN_POLICY>::fields)
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

			/// text height
			{
				I32 textHeight = label.GetTextHeight();

				imguiContext.BeginHorizontal();
				imguiContext.Label("Text Height: ");
				imguiContext.IntField("##TextHeight", textHeight, [&label, &textHeight]() { label.SetTextHeight(static_cast<U32>(std::max<I32>(0, textHeight))); });
				imguiContext.EndHorizontal();
			}

			/// text color
			{
				TColor32F color = label.GetColor();

				imguiContext.BeginHorizontal();
				imguiContext.Label("Text Color: ");
				imguiContext.ColorPickerField("##TextColor", color, [&label, &color]() { label.SetColor(color); });
				imguiContext.EndHorizontal();
			}

			/// \note Text overflow policy
			{
				static std::vector<std::string> overflowPolicies;
				if (overflowPolicies.empty())
				{
					for (auto&& currValueInfo : Meta::EnumTrait<E_TEXT_OVERFLOW_POLICY>::fields)
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
		});
	}

	static void Draw9SliceImageGUI(const TEditorContext& editorContext)
	{
		Header("9SliceImage", editorContext, [](const TEditorContext& editorContext)
		{
			IImGUIContext& imguiContext = editorContext.mImGUIContext;
			IComponent& component = editorContext.mComponent;

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

			{
				TColor32F color = slicedImage.GetColor();

				imguiContext.BeginHorizontal();
				imguiContext.Label("Color: ");
				imguiContext.ColorPickerField("##ImageId", color, [&slicedImage, &color]() { slicedImage.SetColor(color); });
				imguiContext.EndHorizontal();
			}
		});
	}

	static void DrawGridGroupLayoutGUI(const TEditorContext& editorContext)
	{
		Header("Grid Group Layout", editorContext, [](const TEditorContext& editorContext)
		{
			IImGUIContext& imguiContext = editorContext.mImGUIContext;

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
					for (auto&& currValueInfo : Meta::EnumTrait<E_UI_ELEMENT_ALIGNMENT_TYPE>::fields)
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

		});
	}

	static void DrawToggleGUI(const TEditorContext& editorContext)
	{
		Header("Toggle", editorContext, [](const TEditorContext& editorContext)
		{
			IImGUIContext& imguiContext = editorContext.mImGUIContext;
			IComponent& component = editorContext.mComponent;

			CToggle& toggle = dynamic_cast<CToggle&>(component);

			/// \note State of a toggle
			{
				bool isOn = toggle.GetState();

				imguiContext.BeginHorizontal();
				imguiContext.Label("Is On: ");

				if (imguiContext.Checkbox("##state", isOn))
				{
					toggle.SetState(isOn);
				}

				imguiContext.EndHorizontal();
			}

			{
				TEntityId currMarkerEntityId = toggle.GetMarkerEntityId();

				imguiContext.BeginHorizontal();
				imguiContext.Label("Marker Entity: ");

				CImGUIExtensions::EntityRefField(imguiContext, editorContext.mWorld, Wrench::StringUtils::GetEmptyStr(), currMarkerEntityId, [&currMarkerEntityId, &toggle]
				{
					toggle.SetMarkerEntityId(currMarkerEntityId);
				});

				imguiContext.EndHorizontal();
			}
		});
	}

	static void DrawUISliderGUI(const TEditorContext& editorContext)
	{
		Header("UI Slider", editorContext, [](const TEditorContext& editorContext)
		{
			IImGUIContext& imguiContext = editorContext.mImGUIContext;
			IComponent& component = editorContext.mComponent;

			CUISlider& slider = dynamic_cast<CUISlider&>(component);

			/// \note State of a toggle
			/*{
				bool isOn = slider.GetState();

				imguiContext.BeginHorizontal();
				imguiContext.Label("Is On: ");

				if (imguiContext.Checkbox("##state", isOn))
				{
					toggle.SetState(isOn);
				}

				imguiContext.EndHorizontal();
			}

			{
				TEntityId currMarkerEntityId = toggle.GetMarkerEntityId();

				imguiContext.BeginHorizontal();
				imguiContext.Label("Marker Entity: ");

				CImGUIExtensions::EntityRefField(imguiContext, editorContext.mWorld, Wrench::StringUtils::GetEmptyStr(), currMarkerEntityId, [&currMarkerEntityId, &toggle]
				{
					toggle.SetMarkerEntityId(currMarkerEntityId);
				});

				imguiContext.EndHorizontal();
			}*/
		});
	}

	static void DrawUIMaskGUI(const TEditorContext& editorContext)
	{
		Header("UI Mask", editorContext, [](const TEditorContext& editorContext)
		{
		});
	}

	static void DrawScrollUIAreaGUI(const TEditorContext& editorContext)
	{
		Header("Scroll UI Area", editorContext, [](const TEditorContext& editorContext)
		{
			/// \todo implement later
		});
	}

	static void DrawDropDownGUI(const TEditorContext& editorContext)
	{
		Header("Drop Down", editorContext, [](const TEditorContext& editorContext)
		{
			/// \todo implement later
		});
	}

	static void DrawBoxCollision2DGUI(const TEditorContext& editorContext)
	{
		Header("BoxCollision2D", editorContext, [](const TEditorContext& editorContext)
		{
			IImGUIContext& imguiContext = editorContext.mImGUIContext;
			IComponent& component = editorContext.mComponent;

			CBoxCollisionObject2D& box2Dcollision = dynamic_cast<CBoxCollisionObject2D&>(component);

			// \todo Implement this drawer
		});
	}

	static void DrawCircleCollision2DGUI(const TEditorContext& editorContext)
	{
		Header("CircleCollision2D", editorContext, [](const TEditorContext& editorContext)
		{
			IImGUIContext& imguiContext = editorContext.mImGUIContext;
			IComponent& component = editorContext.mComponent;

			CCircleCollisionObject2D& circle2Dcollision = dynamic_cast<CCircleCollisionObject2D&>(component);
			// \todo Implement this drawer
		});
	}

	static void DrawTrigger2DGUI(const TEditorContext& editorContext)
	{
		Header("Trigger2D", editorContext, [](const TEditorContext& editorContext)
		{
			IImGUIContext& imguiContext = editorContext.mImGUIContext;
			IComponent& component = editorContext.mComponent;

			CTrigger2D& trigger = dynamic_cast<CTrigger2D&>(component);

			// \todo Implement this drawer
		});
	}

	static void DrawAudioSourceGUI(const TEditorContext& editorContext)
	{
		Header("Audio Source", editorContext, [](const TEditorContext& editorContext)
		{
			IImGUIContext& imguiContext = editorContext.mImGUIContext;
			IComponent& component = editorContext.mComponent;

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
		});
	}

	static void DrawAudioListenerGUI(const TEditorContext& editorContext)
	{
		Header("Audio Listener", editorContext, [](const TEditorContext& editorContext)
		{
			IImGUIContext& imguiContext = editorContext.mImGUIContext;
			IComponent& component = editorContext.mComponent;

		});
	}

	static void DrawLODStrategyGUI(const TEditorContext& editorContext)
	{
		Header("LOD Strategy", editorContext, [](const TEditorContext& editorContext)
		{
			IImGUIContext& imguiContext = editorContext.mImGUIContext;
			IComponent& component = editorContext.mComponent;

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
		});
	}


	static void DrawWeatherGUI(const TEditorContext& editorContext)
	{
		Header("Weather", editorContext, [](const TEditorContext& editorContext)
		{
			IImGUIContext& imguiContext = editorContext.mImGUIContext;
			IComponent& component = editorContext.mComponent;

			CWeatherComponent& weatherComponent = dynamic_cast<CWeatherComponent&>(component);

			/// Ambient color
			{
				TColor32F ambientCloudsColor = weatherComponent.mAmbientCloudColor;

				imguiContext.BeginHorizontal();
				imguiContext.Label("Ambient Color: ");
				imguiContext.ColorPickerField("##AmbientColor", ambientCloudsColor, [&ambientCloudsColor, &weatherComponent]
				{
					weatherComponent.mAmbientCloudColor = ambientCloudsColor;
				});
				imguiContext.EndHorizontal();
			}

			/// Light absorption
			{
				F32 lightAbsorption = weatherComponent.mSunLightAbsorption;

				imguiContext.BeginHorizontal();
				imguiContext.Label("Light Absorption: ");
				imguiContext.FloatSlider("##LightAbsorption", lightAbsorption, 0.0f, 0.02f, [&lightAbsorption, &weatherComponent]
				{
					if (lightAbsorption != weatherComponent.mSunLightAbsorption)
					{
						weatherComponent.mSunLightAbsorption = lightAbsorption;
					}

				});
				imguiContext.EndHorizontal();
			}

			/// Atmosphere Start Radius
			{
				F32 atmosphereRadius = weatherComponent.mAtmosphereStartRadius;

				imguiContext.BeginHorizontal();
				imguiContext.Label("Atmosphere Start Radius: ");
				imguiContext.FloatField("##AtmoStartRadius", atmosphereRadius, [&atmosphereRadius, &weatherComponent]
				{
					if (atmosphereRadius != weatherComponent.mAtmosphereStartRadius)
					{
						weatherComponent.mAtmosphereStartRadius = atmosphereRadius;
					}

				});
				imguiContext.EndHorizontal();
			}

			/// Atmosphere Thickness
			{
				F32 atmosphereThickness = weatherComponent.mAtmosphereThickness;

				imguiContext.BeginHorizontal();
				imguiContext.Label("Atmosphere Thickness: ");
				imguiContext.FloatField("##AtmoThickness", atmosphereThickness, [&atmosphereThickness, &weatherComponent]
				{
					if (atmosphereThickness != weatherComponent.mAtmosphereThickness)
					{
						weatherComponent.mAtmosphereThickness = atmosphereThickness;
					}

				});
				imguiContext.EndHorizontal();
			}

			/// Coverage
			{
				F32 coverage = weatherComponent.mCoverage;

				imguiContext.BeginHorizontal();
				imguiContext.Label("Coverage: ");
				imguiContext.FloatSlider("##Coverage", coverage, 0.0f, 1.0f, [&coverage, &weatherComponent]
				{
					if (coverage != weatherComponent.mCoverage)
					{
						weatherComponent.mCoverage = coverage;
					}

				});
				imguiContext.EndHorizontal();
			}

			/// Curliness
			{
				F32 curliness = weatherComponent.mCurliness;

				imguiContext.BeginHorizontal();
				imguiContext.Label("Curliness: ");
				imguiContext.FloatSlider("##Curliness", curliness, 0.0f, 10.0f, [&curliness, &weatherComponent]
				{
					if (curliness != weatherComponent.mCurliness)
					{
						weatherComponent.mCurliness = curliness;
					}

				});
				imguiContext.EndHorizontal();
			}

			/// Crispiness
			{
				F32 crispiness = weatherComponent.mCrispiness;

				imguiContext.BeginHorizontal();
				imguiContext.Label("Crispiness: ");
				imguiContext.FloatSlider("##Crispiness", crispiness, 0.0f, 100.0f, [&crispiness, &weatherComponent]
				{
					if (crispiness != weatherComponent.mCrispiness)
					{
						weatherComponent.mCrispiness = crispiness;
					}

				});
				imguiContext.EndHorizontal();
			}

			/// Density factor
			{
				F32 densityFactor = weatherComponent.mDensityFactor;

				imguiContext.BeginHorizontal();
				imguiContext.Label("Density Factor: ");
				imguiContext.FloatSlider("##DensityFactor", densityFactor, 0.0f, 1.0f, [&densityFactor, &weatherComponent]
				{
					if (densityFactor != weatherComponent.mDensityFactor)
					{
						weatherComponent.mDensityFactor = densityFactor;
					}

				});
				imguiContext.EndHorizontal();
			}

			/// Wind direction
			{
				TVector2 windDirection = weatherComponent.mWindDirection;

				imguiContext.BeginHorizontal();
				imguiContext.Label("Wind Direction: ");
				imguiContext.Vector2Field("##WindDirection", windDirection, [&windDirection, &weatherComponent]
				{
					if (windDirection != weatherComponent.mWindDirection)
					{
						weatherComponent.mWindDirection = windDirection;
					}

				});
				imguiContext.EndHorizontal();
			}

			/// Wind Scale factor
			{
				F32 windScaleFactor = weatherComponent.mWindScaleFactor;

				imguiContext.BeginHorizontal();
				imguiContext.Label("Wind Scale Factor: ");
				imguiContext.FloatField("##WindScaleFactor", windScaleFactor, [&windScaleFactor, &weatherComponent]
				{
					if (windScaleFactor != weatherComponent.mWindScaleFactor)
					{
						weatherComponent.mWindScaleFactor = windScaleFactor;
					}

				});
				imguiContext.EndHorizontal();
			}
		});
	}


	static void DrawSplashScreenItemGUI(const TEditorContext& editorContext)
	{
		Header("Splash Screen Item", editorContext, [](const TEditorContext& editorContext)
		{
			IImGUIContext& imguiContext = editorContext.mImGUIContext;
			IComponent& component = editorContext.mComponent;

			CSplashScreenItemComponent& splashScreenItem = dynamic_cast<CSplashScreenItemComponent&>(component);

			/// Duration
			{
				F32 duration = splashScreenItem.mDuration;

				imguiContext.BeginHorizontal();
				imguiContext.Label("Duration: ");
				imguiContext.FloatField("##Duration", duration, [&duration, &splashScreenItem]
				{
					if (duration != splashScreenItem.mDuration)
					{
						splashScreenItem.mDuration = duration;
					}

				});
				imguiContext.EndHorizontal();
			}
		});
	}


	E_RESULT_CODE CDefaultInspectorsRegistry::RegisterBuiltinInspectors(CLevelEditorWindow& editor)
	{
		E_RESULT_CODE result = editor.RegisterInspector(CTransform::GetTypeId(), DrawTransformGUI);
		result = result | editor.RegisterInspector(CDeactivatedComponent::GetTypeId(), DrawEmptyGUI);
		result = result | editor.RegisterInspector(CDeactivatedGroupComponent::GetTypeId(), DrawEmptyGUI);
		result = result | editor.RegisterInspector(CBoundsComponent::GetTypeId(), DrawBoundsGUI);

		/// Graphics
		result = result | editor.RegisterInspector(CQuadSprite::GetTypeId(), DrawQuadSpriteGUI);
		result = result | editor.RegisterInspector(CStaticMeshContainer::GetTypeId(), DrawStaticMeshContainerGUI);
		result = result | editor.RegisterInspector(CSkinnedMeshContainer::GetTypeId(), DrawSkinnedMeshContainerGUI);
		result = result | editor.RegisterInspector(CShadowReceiverComponent::GetTypeId(), DrawShadowReceiverGUI);
		result = result | editor.RegisterInspector(CShadowCasterComponent::GetTypeId(), DrawShadowCasterGUI);
		result = result | editor.RegisterInspector(CDirectionalLight::GetTypeId(), DrawDirectionalLightGUI);
		result = result | editor.RegisterInspector(CPointLight::GetTypeId(), DrawPointLightGUI);
		result = result | editor.RegisterInspector(CSpotLight::GetTypeId(), DrawSpotLightGUI);
		result = result | editor.RegisterInspector(CAnimationContainerComponent::GetTypeId(), DrawAnimationContainerGUI);
		result = result | editor.RegisterInspector(CSkyboxComponent::GetTypeId(), DrawSkyboxGUI);
		result = result | editor.RegisterInspector(CParticleEmitter::GetTypeId(), DrawParticleEmitterGUI);
		result = result | editor.RegisterInspector(CMeshAnimatorComponent::GetTypeId(), DrawMeshAnimatorGUI);
		result = result | editor.RegisterInspector(CPerspectiveCamera::GetTypeId(), DrawPerspectiveCameraGUI);
		result = result | editor.RegisterInspector(COrthoCamera::GetTypeId(), DrawOrthographicCameraGUI);

		result = result | editor.RegisterInspector(CCanvas::GetTypeId(), DrawCanvasGUI);
		result = result | editor.RegisterInspector(CLayoutElement::GetTypeId(), DrawLayoutElementGUI);
		result = result | editor.RegisterInspector(CUIElementMeshData::GetTypeId(), DrawUIElementMeshDataGUI);
		result = result | editor.RegisterInspector(CImage::GetTypeId(), DrawImageGUI);
		result = result | editor.RegisterInspector(CInputReceiver::GetTypeId(), DrawInputReceiverGUI);
		result = result | editor.RegisterInspector(CLabel::GetTypeId(), DrawLabelGUI);
		result = result | editor.RegisterInspector(C9SliceImage::GetTypeId(), Draw9SliceImageGUI);
		result = result | editor.RegisterInspector(CGridGroupLayout::GetTypeId(), DrawGridGroupLayoutGUI);
		result = result | editor.RegisterInspector(CToggle::GetTypeId(), DrawToggleGUI);
		result = result | editor.RegisterInspector(CUISlider::GetTypeId(), DrawUISliderGUI);
		result = result | editor.RegisterInspector(CUIMaskComponent::GetTypeId(), DrawUIMaskGUI);
		result = result | editor.RegisterInspector(CScrollableUIArea::GetTypeId(), DrawScrollUIAreaGUI);
		result = result | editor.RegisterInspector(CDropDown::GetTypeId(), DrawDropDownGUI);

		/// 2D Physics
		result = result | editor.RegisterInspector(CBoxCollisionObject2D::GetTypeId(), DrawBoxCollision2DGUI);
		result = result | editor.RegisterInspector(CCircleCollisionObject2D::GetTypeId(), DrawCircleCollision2DGUI);
		result = result | editor.RegisterInspector(CTrigger2D::GetTypeId(), DrawTrigger2DGUI);

		/// Audio
		result = result | editor.RegisterInspector(CAudioListenerComponent::GetTypeId(), DrawAudioListenerGUI);
		result = result | editor.RegisterInspector(CAudioSourceComponent::GetTypeId(), DrawAudioSourceGUI);

		/// Scenes
		result = result | editor.RegisterInspector(CSelectedEntityComponent::GetTypeId(), [](auto) { /* Do nothing for hidden components */ });
		result = result | editor.RegisterInspector(CLODStrategyComponent::GetTypeId(), DrawLODStrategyGUI);
		result = result | editor.RegisterInspector(CWeatherComponent::GetTypeId(), DrawWeatherGUI);
		result = result | editor.RegisterInspector(CSplashScreenItemComponent::GetTypeId(), DrawSplashScreenItemGUI);

		return result;
	}

	void CDefaultInspectorsRegistry::DrawInspectorHeader(const std::string& headerText, const TEditorContext& editorContext, const std::function<void(const TEditorContext&)>& action)
	{
		Header(headerText, editorContext, action);
	}
}

#endif