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


	/*!
		\brief The method draws editor's grid at center of the world 
	*/

	TDE2_API void DrawEditorGrid(IDebugUtility* pDebugUtility, I32 rows, I32 cols, F32 cellSize = 1.0f);


	class CEntitiesCommands
	{
		public:
			TDE2_API static E_RESULT_CODE CopyEntitiesHierarchy(IWorld* pWorld, TEntityId entityId);
			TDE2_API static E_RESULT_CODE PasteEntitiesHierarchy(IWorld* pWorld, TEntityId parentEntityId);
	};
}

#endif