/*!
	\file EditorUtils.h
	\date 13.04.2022
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Types.h"
#include "../utils/Utils.h"


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	class IDebugUtility;
	class IWorld;
	class IPrefabsRegistry;


	TDE2_DECLARE_SCOPED_PTR(IWorld);
	TDE2_DECLARE_SCOPED_PTR(IPrefabsRegistry);


	/*!
		\brief The method draws editor's grid at center of the world 
	*/

	TDE2_API void DrawEditorGrid(IDebugUtility* pDebugUtility, I32 rows, I32 cols, F32 cellSize = 1.0f);


	class CEntitiesCommands
	{
		public:
			TDE2_API static E_RESULT_CODE CopyEntitiesHierarchy(TPtr<IPrefabsRegistry> pPrefabsRegistry, TPtr<IWorld> pWorld, TEntityId entityId);
			TDE2_API static E_RESULT_CODE PasteEntitiesHierarchy(TPtr<IPrefabsRegistry> pPrefabsRegistry, TPtr<IWorld> pWorld, TEntityId parentEntityId);
	};
}

#endif