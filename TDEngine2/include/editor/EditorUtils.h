/*!
	\file EditorUtils.h
	\date 13.04.2022
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Types.h"
#include "../utils/Utils.h"
#include <functional>


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	class IDebugUtility;
	class IWorld;
	class IPrefabsRegistry;
	class IScene;
	class IImGUIContext;
	class CEntityRef;


	TDE2_DECLARE_SCOPED_PTR(IWorld);
	TDE2_DECLARE_SCOPED_PTR(IPrefabsRegistry);
	TDE2_DECLARE_SCOPED_PTR(IImGUIContext);


	/*!
		\brief The method draws editor's grid at center of the world 
	*/

	TDE2_API void DrawEditorGrid(IDebugUtility* pDebugUtility, I32 rows, I32 cols, F32 cellSize = 1.0f);


	class CEntitiesCommands
	{
		public:
			TDE2_API static E_RESULT_CODE CopyEntitiesHierarchy(TPtr<IPrefabsRegistry> pPrefabsRegistry, TPtr<IWorld> pWorld, TEntityId entityId);
			TDE2_API static TResult<TEntityId> PasteEntitiesHierarchy(TPtr<IPrefabsRegistry> pPrefabsRegistry, TPtr<IWorld> pWorld, IScene* pCurrScene, TEntityId parentEntityId);
	};


	class CImGUIExtensions
	{
		public:
			TDE2_API static E_RESULT_CODE EntityRefField(TPtr<IImGUIContext> pImGUIContext, TPtr<IWorld> pWorld, const std::string& text, TEntityId& entityRef, const std::function<void()>& onValueChanged = {});
			TDE2_API static E_RESULT_CODE EntityRefField(TPtr<IImGUIContext> pImGUIContext, TPtr<IWorld> pWorld, const std::string& text, CEntityRef& entityRef, const std::function<void()>& onValueChanged = {});
	};
}

#endif