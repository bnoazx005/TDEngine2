/*!
	\file CUnixWindowSurfaceFactory.h
	\date 06.02.2025
	\authors Kasimov Ildar
*/
#pragma once


#include "../IWindowSurfaceFactory.h"
#include <utils/Utils.h>
#include <core/CBaseObject.h>


#if defined(TDE2_USE_UNIXPLATFORM)


namespace TDEngine2
{
	class CUnixWindowSurfaceFactory: public CBaseObject, public IWindowSurfaceFactory
	{
		public:
			friend TDE2_API IWindowSurfaceFactory* CreateUnixWindowSurfaceFactory(IWindowSystem*, E_RESULT_CODE&);
		public:
			TDE2_API E_RESULT_CODE Init(IWindowSystem* pWindowSystem) override;

			TDE2_API VkSurfaceKHR GetSurface(VkInstance instance) override;

		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CUnixWindowSurfaceFactory)

		protected:
			IWindowSystem* mpWindowSystem;
	};


	TDE2_API IWindowSurfaceFactory* CreateUnixWindowSurfaceFactory(IWindowSystem* pWindowSystem, E_RESULT_CODE& result);
}

#endif