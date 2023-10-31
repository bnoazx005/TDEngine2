#include "../../include/win32/CWin32WindowSurfaceFactory.h"
#include <core/IWindowSystem.h>
#define VK_NO_PROTOTYPES
#include <volk.h>
#include <vulkan/vulkan.h>


#if defined(TDE2_USE_WINPLATFORM)

namespace TDEngine2
{
	CWin32WindowSurfaceFactory::CWin32WindowSurfaceFactory():
		CBaseObject()
	{
	}
	
	E_RESULT_CODE CWin32WindowSurfaceFactory::Init(IWindowSystem* pWindowSystem)
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

	VkSurfaceKHR CWin32WindowSurfaceFactory::GetSurface(VkInstance instance)
	{
		VkSurfaceKHR surface = VK_NULL_HANDLE;

		VkWin32SurfaceCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		createInfo.hwnd = mpWindowSystem->GetInternalData().mWindowHandler;
		createInfo.hinstance = mpWindowSystem->GetInternalData().mWindowInstanceHandler;

		VkResult result = vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface);
		if (VK_SUCCESS != result)
		{
			return VK_NULL_HANDLE;
		}

		return surface;
	}


	TDE2_API IWindowSurfaceFactory* CreateWin32WindowSurfaceFactory(IWindowSystem* pWindowSystem, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IWindowSurfaceFactory, CWin32WindowSurfaceFactory, result, pWindowSystem);
	}
}

#endif