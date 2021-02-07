#include "../../include/editor/Inspectors.h"
#include "../../include/core/IImGUIContext.h"
#include "../../include/ecs/IComponent.h"
#include "../../include/ecs/CTransform.h"
#include "../../include/ecs/components/CBoundsComponent.h"
#include "../../include/graphics/CQuadSprite.h"
#include "../../include/graphics/CStaticMeshContainer.h"
#include "../../include/physics/2D/CBoxCollisionObject2D.h"
#include "../../include/physics/2D/CCircleCollisionObject2D.h"
#include "../../include/physics/2D/CTrigger2D.h"
#include "../../include/physics/3D/CBoxCollisionObject3D.h"
#include "../../include/physics/3D/CSphereCollisionObject3D.h"
#include "../../include/physics/3D/CConvexHullCollisionObject3D.h"
#include "../../include/physics/3D/CTrigger3D.h"
#include "../../include/scene/components/ShadowMappingComponents.h"
#include "../../include/scene/components/CDirectionalLight.h"
#include "../../include/scene/components/AudioComponents.h"
#include "../../include/editor/ecs/EditorComponents.h"
#include "../../include/editor/CLevelEditorWindow.h"
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
		result = result | editor.RegisterInspector(CAnimationContainerComponent::GetTypeId(), DrawAnimationContainerGUI);
		result = result | editor.RegisterInspector(CSkyboxComponent::GetTypeId(), DrawSkyboxGUI);

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

		result = result | editor.RegisterInspector(CSelectedEntityComponent::GetTypeId(), [](auto&, auto&) { /* Do nothing for hidden components */ });

		return result;
	}

	void CDefeaultInspectorsRegistry::DrawTransformGUI(IImGUIContext& imguiContext, IComponent& component)
	{
		if (imguiContext.CollapsingHeader("Transform", true, false))
		{
			CTransform& transform = dynamic_cast<CTransform&>(component);

			TVector3 position = transform.GetPosition();
			TVector3 rotation = ToEulerAngles(transform.GetRotation());
			TVector3 scale = transform.GetScale();

			imguiContext.BeginHorizontal();
			imguiContext.Label("Position");
			imguiContext.Vector3Field("##1", position, [&transform, &position] { transform.SetPosition(position); });
			imguiContext.EndHorizontal();

			imguiContext.BeginHorizontal();
			imguiContext.Label("Rotation");
			imguiContext.Vector3Field("##2", rotation, [&transform, &rotation] { transform.SetRotation(TQuaternion(rotation)); });
			imguiContext.EndHorizontal();

			imguiContext.BeginHorizontal();
			imguiContext.Label("Scale   ");
			imguiContext.Vector3Field("##3", scale, [&transform, &scale] { transform.SetScale(scale); });
			imguiContext.EndHorizontal();
		}
	}

	void CDefeaultInspectorsRegistry::DrawBoundsGUI(IImGUIContext& imguiContext, IComponent& component)
	{
		if (imguiContext.CollapsingHeader("Bounds", true))
		{
			CBoundsComponent& bounds = dynamic_cast<CBoundsComponent&>(component);

			// \todo Implement this drawer
		}
	}

	void CDefeaultInspectorsRegistry::DrawQuadSpriteGUI(IImGUIContext& imguiContext, IComponent& component)
	{
		if (imguiContext.CollapsingHeader("QuadSprite", true))
		{
			CQuadSprite& sprite = dynamic_cast<CQuadSprite&>(component);

			imguiContext.Label(sprite.GetMaterialName());
			// \todo Implement this drawer
		}
	}

	void CDefeaultInspectorsRegistry::DrawStaticMeshContainerGUI(IImGUIContext& imguiContext, IComponent& component)
	{
		if (imguiContext.CollapsingHeader("Static Mesh Container", true))
		{
			CStaticMeshContainer& meshContainer = dynamic_cast<CStaticMeshContainer&>(component);

			imguiContext.Label(meshContainer.GetMeshName());
			imguiContext.Label(meshContainer.GetMaterialName());
			// \todo Implement this drawer
		}
	}

	void CDefeaultInspectorsRegistry::DrawShadowReceiverGUI(IImGUIContext& imguiContext, IComponent& component)
	{
		if (imguiContext.CollapsingHeader("Shadow Receiver", true))
		{
			CShadowReceiverComponent& shadowReceiver = dynamic_cast<CShadowReceiverComponent&>(component);

			// \todo Implement this drawer
		}
	}

	void CDefeaultInspectorsRegistry::DrawShadowCasterGUI(IImGUIContext& imguiContext, IComponent& component)
	{
		if (imguiContext.CollapsingHeader("Shadow Caster", true))
		{
			// \todo Implement this drawer
		}
	}

	void CDefeaultInspectorsRegistry::DrawDirectionalLightGUI(IImGUIContext& imguiContext, IComponent& component)
	{
		if (imguiContext.CollapsingHeader("Directional Light", true))
		{
			CDirectionalLight& dirLight = dynamic_cast<CDirectionalLight&>(component);

			// \todo Implement this drawer
			
		}
	}

	void CDefeaultInspectorsRegistry::DrawAnimationContainerGUI(IImGUIContext& imguiContext, IComponent& component)
	{
		const TVector2 buttonSizes(100.0f, 25.0f);

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

	void CDefeaultInspectorsRegistry::DrawSkyboxGUI(IImGUIContext& imguiContext, IComponent& component)
	{
		if (imguiContext.CollapsingHeader("Skybox", true)) {}
	}

	void CDefeaultInspectorsRegistry::DrawBoxCollision2DGUI(IImGUIContext& imguiContext, IComponent& component)
	{
		if (imguiContext.CollapsingHeader("BoxCollision2D", true))
		{
			CBoxCollisionObject2D& box2Dcollision = dynamic_cast<CBoxCollisionObject2D&>(component);

			// \todo Implement this drawer
		}
	}

	void CDefeaultInspectorsRegistry::DrawCircleCollision2DGUI(IImGUIContext& imguiContext, IComponent& component)
	{
		if (imguiContext.CollapsingHeader("CircleCollision2D", true))
		{
			CCircleCollisionObject2D& circle2Dcollision = dynamic_cast<CCircleCollisionObject2D&>(component);
			// \todo Implement this drawer
		}
	}

	void CDefeaultInspectorsRegistry::DrawTrigger2DGUI(IImGUIContext& imguiContext, IComponent& component)
	{
		if (imguiContext.CollapsingHeader("Trigger2D", true))
		{
			CTrigger2D& trigger = dynamic_cast<CTrigger2D&>(component);

			// \todo Implement this drawer
		}
	}

	void CDefeaultInspectorsRegistry::DrawBoxCollision3DGUI(IImGUIContext& imguiContext, IComponent& component)
	{
		if (imguiContext.CollapsingHeader("BoxCollision3D", true))
		{
			CBaseCollisionObject3D& box3Dcollision = dynamic_cast<CBaseCollisionObject3D&>(component);

			// \todo Implement this drawer
		}
	}

	void CDefeaultInspectorsRegistry::DrawSphereCollision3DGUI(IImGUIContext& imguiContext, IComponent& component)
	{
		if (imguiContext.CollapsingHeader("SphereCollision3D", true))
		{
			CSphereCollisionObject3D& sphereCollision = dynamic_cast<CSphereCollisionObject3D&>(component);

			// \todo Implement this drawer
		}
	}

	void CDefeaultInspectorsRegistry::DrawConvexHullCollision3DGUI(IImGUIContext& imguiContext, IComponent& component)
	{
		if (imguiContext.CollapsingHeader("ConvexHullCollision3D", true))
		{
			CConvexHullCollisionObject3D& convexHullCollision = dynamic_cast<CConvexHullCollisionObject3D&>(component);

			// \todo Implement this drawer
		}
	}

	void CDefeaultInspectorsRegistry::DrawTrigger3DGUI(IImGUIContext& imguiContext, IComponent& component)
	{
		if (imguiContext.CollapsingHeader("Trigger3D", true))
		{
			// \todo Implement this drawer
		}
	}


	void CDefeaultInspectorsRegistry::DrawAudioSourceGUI(IImGUIContext& imguiContext, IComponent& component)
	{
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

	void CDefeaultInspectorsRegistry::DrawAudioListenerGUI(IImGUIContext& imguiContext, IComponent& component)
	{
		if (imguiContext.CollapsingHeader("Audio Listener", true))
		{
		}
	}
}

#endif