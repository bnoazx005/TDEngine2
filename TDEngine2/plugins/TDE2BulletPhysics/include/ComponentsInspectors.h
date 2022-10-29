/*!
	\file ComponentsInspector.h
	\date 29.10.2022
	\authors Kasimov Ildar
*/

#pragma once


#include <editor/IEditorsManager.h>


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	TDE2_API E_RESULT_CODE RegisterComponentsInspector(TPtr<IEditorsManager> pEditorsManager);
}

#endif