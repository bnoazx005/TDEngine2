/*!
	\file Inspectors.h
	\date 13.12.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Config.h"
#include "../utils/Types.h"
#include <functional>


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

	class CDefaultInspectorsRegistry
	{
		public:
			TDE2_API static E_RESULT_CODE RegisterBuiltinInspectors(CLevelEditorWindow& editor);

			TDE2_API static void DrawTransformGUI(const TEditorContext& editorContext);
			TDE2_API static void DrawBoundsGUI(const TEditorContext& editorContext);

			/// Graphics components
			TDE2_API static void DrawQuadSpriteGUI(const TEditorContext& editorContext);
			TDE2_API static void DrawStaticMeshContainerGUI(const TEditorContext& editorContext);
			TDE2_API static void DrawSkinnedMeshContainerGUI(const TEditorContext& editorContext);
			TDE2_API static void DrawShadowReceiverGUI(const TEditorContext& editorContext);
			TDE2_API static void DrawShadowCasterGUI(const TEditorContext& editorContext);
			TDE2_API static void DrawDirectionalLightGUI(const TEditorContext& editorContext);
			TDE2_API static void DrawPointLightGUI(const TEditorContext& editorContext);
			TDE2_API static void DrawAnimationContainerGUI(const TEditorContext& editorContext);
			TDE2_API static void DrawSkyboxGUI(const TEditorContext& editorContext);
			TDE2_API static void DrawParticleEmitterGUI(const TEditorContext& editorContext);
			TDE2_API static void DrawCanvasGUI(const TEditorContext& editorContext);
			TDE2_API static void DrawLayoutElementGUI(const TEditorContext& editorContext);
			TDE2_API static void DrawUIElementMeshDataGUI(const TEditorContext& editorContext);
			TDE2_API static void DrawImageGUI(const TEditorContext& editorContext);
			TDE2_API static void DrawInputReceiverGUI(const TEditorContext& editorContext);
			TDE2_API static void DrawLabelGUI(const TEditorContext& editorContext);
			TDE2_API static void DrawMeshAnimatorGUI(const TEditorContext& editorContext);
			TDE2_API static void Draw9SliceImageGUI(const TEditorContext& editorContext);
			TDE2_API static void DrawGridGroupLayoutGUI(const TEditorContext& editorContext);
			TDE2_API static void DrawPerspectiveCameraGUI(const TEditorContext& editorContext);
			TDE2_API static void DrawOrthographicCameraGUI(const TEditorContext& editorContext);

			/// 2D Physics components
			TDE2_API static void DrawBoxCollision2DGUI(const TEditorContext& editorContext);
			TDE2_API static void DrawCircleCollision2DGUI(const TEditorContext& editorContext);
			TDE2_API static void DrawTrigger2DGUI(const TEditorContext& editorContext);

			/// Audio components
			TDE2_API static void DrawAudioListenerGUI(const TEditorContext& editorContext);
			TDE2_API static void DrawAudioSourceGUI(const TEditorContext& editorContext);
			
			/// Scenes components
			TDE2_API static void DrawLODStrategyGUI(const TEditorContext& editorContext);

			TDE2_API static void DrawInspectorHeader(const std::string& headerText, const TEditorContext& editorContext, const std::function<void(const TEditorContext&)>& action);
	};
}

#endif