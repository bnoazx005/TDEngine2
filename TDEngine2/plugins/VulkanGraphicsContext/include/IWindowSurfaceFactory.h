/*!
	\file IWindowSurfaceFactory.h
	\date 31.10.2023
	\authors Kasimov Ildar
*/
#pragma once


#include <utils/Types.h>
#include <core/IBaseObject.h>
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan_core.h>


namespace TDEngine2
{
	class IWindowSystem;


	/*!
		interface IWindowSurfaceFactory

		\brief The interface provides functionality of a factory for window surfaces creation.
	*/

	class IWindowSurfaceFactory: public virtual IBaseObject
	{
		public:
			TDE2_API virtual E_RESULT_CODE Init(IWindowSystem* pWindowSystem) = 0;

			TDE2_API virtual VkSurfaceKHR GetSurface(VkInstance instance) = 0;

		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IWindowSurfaceFactory)
	};


	TDE2_DECLARE_SCOPED_PTR_INLINED(IWindowSurfaceFactory);
}