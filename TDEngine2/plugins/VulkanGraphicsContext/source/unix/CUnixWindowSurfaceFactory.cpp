#include "../../include/unix/CUnixWindowSurfaceFactory.h"
#include <core/IWindowSystem.h>
#define VK_NO_PROTOTYPES
#include <volk.h>
#include <vulkan/vulkan.h>


#if defined(TDE2_USE_UNIXPLATFORM)

namespace TDEngine2
{
	CUnixWindowSurfaceFactory::CUnixWindowSurfaceFactory():
		CBaseObject()
	{
	}
	
	E_RESULT_CODE CUnixWindowSurfaceFactory::Init(IWindowSystem* pWindowSystem)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		if (!pWindowSystem)
		{
			return RC_INVALID_ARGS;
		}
		
		mpWindowSystem = pWindowSystem;

		mIsInitialized = true;

		return RC_OK;
	}

	VkSurfaceKHR CUnixWindowSurfaceFactory::GetSurface(VkInstance instance)
	{
		VkSurfaceKHR surface = VK_NULL_HANDLE;

		VkXlibSurfaceCreateInfoKHR createInfo{};
		createInfo.sType  = VK_STRUCTURE_TYPE_Unix_SURFACE_CREATE_INFO_KHR;
		createInfo.window = mpWindowSystem->GetInternalData().mWindowHandler;
		createInfo.dpy    = mpWindowSystem->GetInternalData().mpDisplayHandler;

		VkResult result = vkCreateXlibSurfaceKHR(instance, &createInfo, nullptr, &surface);
		if (VK_SUCCESS != result)
		{
			return VK_NULL_HANDLE;
		}

		return surface;
	}


	TDE2_API IWindowSurfaceFactory* CreateUnixWindowSurfaceFactory(IWindowSystem* pWindowSystem, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IWindowSurfaceFactory, CUnixWindowSurfaceFactory, result, pWindowSystem);
	}
}

#endif