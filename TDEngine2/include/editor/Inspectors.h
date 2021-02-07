/*!
	\file Inspectors.h
	\date 13.12.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Config.h"
#include "../utils/Types.h"


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	class IImGUIContext;
	class IComponent;
	class CLevelEditorWindow;


	/*!
		\brief The static class provides implementation of built-in components that are allowed within in-game editor
	*/

	class CDefeaultInspectorsRegistry
	{
		public:
			TDE2_API static E_RESULT_CODE RegisterBuiltinInspectors(CLevelEditorWindow& editor);

			TDE2_API static void DrawTransformGUI(IImGUIContext& imguiContext, IComponent& component);
			TDE2_API static void DrawBoundsGUI(IImGUIContext& imguiContext, IComponent& component);

			/// Graphics components
			TDE2_API static void DrawQuadSpriteGUI(IImGUIContext& imguiContext, IComponent& component);
			TDE2_API static void DrawStaticMeshContainerGUI(IImGUIContext& imguiContext, IComponent& component);
			TDE2_API static void DrawShadowReceiverGUI(IImGUIContext& imguiContext, IComponent& component);
			TDE2_API static void DrawShadowCasterGUI(IImGUIContext& imguiContext, IComponent& component);
			TDE2_API static void DrawDirectionalLightGUI(IImGUIContext& imguiContext, IComponent& component);
			TDE2_API static void DrawAnimationContainerGUI(IImGUIContext& imguiContext, IComponent& component);
			TDE2_API static void DrawSkyboxGUI(IImGUIContext& imguiContext, IComponent& component);

			/// 2D Physics components
			TDE2_API static void DrawBoxCollision2DGUI(IImGUIContext& imguiContext, IComponent& component);
			TDE2_API static void DrawCircleCollision2DGUI(IImGUIContext& imguiContext, IComponent& component);
			TDE2_API static void DrawTrigger2DGUI(IImGUIContext& imguiContext, IComponent& component);

			/// 3D Physics components
			TDE2_API static void DrawBoxCollision3DGUI(IImGUIContext& imguiContext, IComponent& component);
			TDE2_API static void DrawSphereCollision3DGUI(IImGUIContext& imguiContext, IComponent& component);
			TDE2_API static void DrawConvexHullCollision3DGUI(IImGUIContext& imguiContext, IComponent& component);
			TDE2_API static void DrawTrigger3DGUI(IImGUIContext& imguiContext, IComponent& component);

			/// Audio components
			TDE2_API static void DrawAudioListenerGUI(IImGUIContext& imguiContext, IComponent& component);
			TDE2_API static void DrawAudioSourceGUI(IImGUIContext& imguiContext, IComponent& component);
	};
}

#endif