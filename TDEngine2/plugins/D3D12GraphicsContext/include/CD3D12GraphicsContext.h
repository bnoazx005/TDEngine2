/*!
	\file CD3D12GraphicsContext.h
	\date 21.09.2024
	\authors Kasimov Ildar
*/
#pragma once


#include <core/IGraphicsContext.h>
#include <core/Event.h>
#include <core/CBaseObject.h>


#if defined(TDE2_USE_WINPLATFORM) /// Used only on Windows platform

namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CD3D12GraphicsContext's type

		\return A pointer to CD3D12GraphicsContext's implementation
	*/

	IGraphicsContext* CreateD3D12GraphicsContext(TPtr<IWindowSystem> pWindowSystem, E_RESULT_CODE& result);
}

#endif