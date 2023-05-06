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

			TDE2_API static void DrawInspectorHeader(const std::string& headerText, const TEditorContext& editorContext, const std::function<void(const TEditorContext&)>& action);
	};
}

#endif