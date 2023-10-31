/*!
	\file CWin32WindowSurfaceFactory.h
	\date 31.10.2023
	\authors Kasimov Ildar
*/
#pragma once


#include "../IWindowSurfaceFactory.h"
#include <utils/Utils.h>
#include <core/CBaseObject.h>


#if defined(TDE2_USE_WINPLATFORM)


namespace TDEngine2
{
	class CWin32WindowSurfaceFactory: public CBaseObject, public IWindowSurfaceFactory
	{
		public:
			friend TDE2_API IWindowSurfaceFactory* CreateWin32WindowSurfaceFactory(IWindowSystem*, E_RESULT_CODE&);
		public:
			/*!
				\brief The method initializes an object's state

				\param[in] pWindowSystem A pointer to implementation of IWindowSystem

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IWindowSystem* pWindowSystem) override;

			TDE2_API VkSurfaceKHR GetSurface(VkInstance instance) override;

		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CWin32WindowSurfaceFactory)

		protected:
			IWindowSystem* mpWindowSystem;
	};


	/*!
		\brief A factory function for creation objects of CWin32WindowSurfaceFactory's type

		\return A pointer to CWin32WindowSurfaceFactory's implementation
	*/

	TDE2_API IWindowSurfaceFactory* CreateWin32WindowSurfaceFactory(IWindowSystem* pWindowSystem, E_RESULT_CODE& result);
}

#endif