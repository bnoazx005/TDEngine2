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
	class IEditorActionsHistory;
	class IWorld;
	
	enum class TEntityId : U32;


	typedef struct TEditorContext
	{
		IImGUIContext& mImGUIContext;
		IComponent& mComponent;
		IEditorActionsHistory& mActionsHistory;
		IWorld& mWorld;
		TEntityId mCurrEntityId;
	} TEditorContext;


	/*!
		\brief The static class provides implementation of built-in components that are allowed within in-game editor
	*/

	class CDefeaultInspectorsRegistry
	{
		public:
			TDE2_API static E_RESULT_CODE RegisterBuiltinInspectors(CLevelEditorWindow& editor);

			TDE2_API static void DrawTransformGUI(const TEditorContext& editorContext);
			TDE2_API static void DrawBoundsGUI(const TEditorContext& editorContext);

			/// Graphics components
			TDE2_API static void DrawQuadSpriteGUI(const TEditorContext& editorContext);
			TDE2_API static void DrawStaticMeshContainerGUI(const TEditorContext& editorContext);
			TDE2_API static void DrawShadowReceiverGUI(const TEditorContext& editorContext);
			TDE2_API static void DrawShadowCasterGUI(const TEditorContext& editorContext);
			TDE2_API static void DrawDirectionalLightGUI(const TEditorContext& editorContext);
			TDE2_API static void DrawAnimationContainerGUI(const TEditorContext& editorContext);
			TDE2_API static void DrawSkyboxGUI(const TEditorContext& editorContext);

			/// 2D Physics components
			TDE2_API static void DrawBoxCollision2DGUI(const TEditorContext& editorContext);
			TDE2_API static void DrawCircleCollision2DGUI(const TEditorContext& editorContext);
			TDE2_API static void DrawTrigger2DGUI(const TEditorContext& editorContext);

			/// 3D Physics components
			TDE2_API static void DrawBoxCollision3DGUI(const TEditorContext& editorContext);
			TDE2_API static void DrawSphereCollision3DGUI(const TEditorContext& editorContext);
			TDE2_API static void DrawConvexHullCollision3DGUI(const TEditorContext& editorContext);
			TDE2_API static void DrawTrigger3DGUI(const TEditorContext& editorContext);

			/// Audio components
			TDE2_API static void DrawAudioListenerGUI(const TEditorContext& editorContext);
			TDE2_API static void DrawAudioSourceGUI(const TEditorContext& editorContext);
	};
}

#endif