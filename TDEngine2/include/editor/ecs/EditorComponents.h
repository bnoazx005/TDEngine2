/*!
	\file EditorComponents.h
	\date 09.03.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "./../../utils/Types.h"
#include "./../../ecs/CBaseComponent.h"
#include <vector>


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	TDE2_DECLARE_FLAG_COMPONENT(SelectedEntityComponent)
}

#endif