#include "../include/CVulkanGraphicsContext.h"
#include "../include/CVulkanUtils.h"
#include "../include/IWindowSurfaceFactory.h"
#include "../include/CVulkanGraphicsObjectManager.h"
#include "../include/CVulkanResources.h"
#include "../include/CVulkanShaderCompiler.h"
#include <core/IEventManager.h>
#include <core/IWindowSystem.h>
#include <utils/CFileLogger.h>
#include <editor/CPerfProfiler.h>
#define DEFER_IMPLEMENTATION
#include "deferOperation.hpp"
#include "stringUtils.hpp"
#include <string>
#include <cstring>
#include <algorithm>


namespace TDEngine2
{

	static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugMessage(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
															VkDebugUtilsMessageTypeFlagsEXT messageType,
															const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
															void* pUserData)
	{
		switch (messageSeverity)
		{
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
				LOG_ERROR(Wrench::StringUtils::Format("[CVulkanGraphicsContext] {0}\n", pCallbackData->pMessage));
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
				LOG_WARNING(Wrench::StringUtils::Format("[CVulkanGraphicsContext] {0}\n", pCallbackData->pMessage));
				break;
			default:
				LOG_MESSAGE(Wrench::StringUtils::Format("[CVulkanGraphicsContext] {0}\n", pCallbackData->pMessage));
				break;
		}
				
		return VK_FALSE;
	}


	static std::vector<VkExtensionProperties> GetAvailableExtensions()
	{
		U32 extensionsCount = 0;
		if (VK_SUCCESS != vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, nullptr))
		{
			return {};
		}

		std::vector<VkExtensionProperties> availableExtensions(extensionsCount);
		if (VK_SUCCESS != vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, availableExtensions.data()))
		{
			return {};
		}

		LOG_MESSAGE("[VulkanGraphicsContext] List of available extensions:");

		for (auto&& currExtension : availableExtensions)
		{
			LOG_MESSAGE(Wrench::StringUtils::Format("[VulkanGraphicsContext] {0}", currExtension.extensionName));
		}

		return availableExtensions;
	}

#if TDE2_DEBUG_MODE
	static const std::vector<const C8*> ValidationLayers
	{
		"VK_LAYER_KHRONOS_validation",
	};
#endif


	static const std::vector<const C8*> RequiredExtensions
	{
		VK_KHR_SURFACE_EXTENSION_NAME,
#if defined(TDE2_USE_WINPLATFORM)
			VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#elif defined(TDE2_USE_UNIXPLATFORM)
#endif
#if TDE2_DEBUG_MODE
			VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
		VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
	};


	static const std::vector<const C8*> RequiredDeviceExtensions
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
		VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME,
		VK_EXT_ROBUSTNESS_2_EXTENSION_NAME
	};

	static std::vector<const C8*> SupportedDeviceExtensions{}; // RequiredDeviceExtensions minus unsupported extensions

	bool IS_ROBUSTNESS2_EXTENSION_SUPPORTED = false;


	static bool CheckUpValidationLayers()
	{
#if TDE2_DEBUG_MODE
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (auto&& currLayerName : ValidationLayers)
		{
			auto it = std::find_if(availableLayers.cbegin(), availableLayers.cend(), [&currLayerName](auto&& layer) { return strcmp(layer.layerName, currLayerName) == 0; });
			if (it == availableLayers.cend())
			{
				return false;
			}
		}

		return true;
#else
		return true;
#endif
	}


#if TDE2_DEBUG_MODE

	static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func)
		{
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}

		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}


	static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func)
		{
			func(instance, debugMessenger, pAllocator);
		}
	}


	static void PrepareDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
	{
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = VulkanDebugMessage;
	}


	static void InitDebugMessageOutput(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger)
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		PrepareDebugMessengerCreateInfo(createInfo);

		VK_SAFE_VOID_CALL(CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger));
	}
#endif


	static TResult<VkInstance> CreateInstance(const std::string& appName)
	{
#if TDE2_DEBUG_MODE
		if (!CheckUpValidationLayers())
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}
#endif

		std::vector<VkExtensionProperties> availableExtensions = GetAvailableExtensions();
		if (availableExtensions.empty())
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		USIZE requiredExtensionsFound = 0;

		for (auto&& currRequiredExtensionName : RequiredExtensions)
		{
			auto it = std::find_if(availableExtensions.cbegin(), availableExtensions.cend(), [&currRequiredExtensionName](const VkExtensionProperties& ext)
				{
					return strcmp(ext.extensionName, currRequiredExtensionName) == 0;
				});

			if (it != availableExtensions.cend())
			{
				++requiredExtensionsFound;
			}
		}

		if (requiredExtensionsFound < RequiredExtensions.size())
		{
			LOG_ERROR("[VulkanGraphicsContext] Not all of required extensions are available in the system");
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		VkInstance instance;

		VkApplicationInfo appInfo{};
		appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName   = appName.c_str();
		appInfo.applicationVersion = VK_MAKE_API_VERSION(0, TDE2_MAJOR_VERSION, TDE2_MINOR_VERSION, TDE2_PATCH_VERSION);
		appInfo.pEngineName        = "TDEngine2";
		appInfo.engineVersion      = VK_MAKE_API_VERSION(0, TDE2_MAJOR_VERSION, TDE2_MINOR_VERSION, TDE2_PATCH_VERSION);
		appInfo.apiVersion         = VK_API_VERSION_1_3;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo        = &appInfo;
		createInfo.enabledExtensionCount   = static_cast<U32>(RequiredExtensions.size());
		createInfo.ppEnabledExtensionNames = RequiredExtensions.data();
		createInfo.enabledLayerCount       = 0;

#if TDE2_DEBUG_MODE
		createInfo.ppEnabledLayerNames = ValidationLayers.data();
		createInfo.enabledLayerCount = static_cast<U32>(ValidationLayers.size());

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		PrepareDebugMessengerCreateInfo(debugCreateInfo);

		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
#endif

		const VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
		if (VK_SUCCESS != result)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(CVulkanMappings::GetErrorCode(result));
		}

		volkLoadInstance(instance);

		return Wrench::TOkValue<VkInstance>(instance);
	}


	static bool CheckDeviceRequiredExtensions(VkPhysicalDevice device)
	{
		U32 extensionsCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionsCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, availableExtensions.data());

		USIZE supportedExtensionsCount = 0;

		SupportedDeviceExtensions.clear();

		for (auto&& currExtension : RequiredDeviceExtensions)
		{
			auto&& it = std::find_if(availableExtensions.cbegin(), availableExtensions.cend(), [&currExtension](auto&& ext) { return strcmp(ext.extensionName, currExtension) == 0; });
			if (it != availableExtensions.cend())
			{
				SupportedDeviceExtensions.emplace_back(currExtension);
				++supportedExtensionsCount;
			}
		}

		return supportedExtensionsCount >= RequiredDeviceExtensions.size();
	}


	static bool IsDeviceSuitable(VkPhysicalDevice device)
	{
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);

		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		// \todo Implement score based selection
		return CheckDeviceRequiredExtensions(device) && deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader;
	}


	static TResult<VkPhysicalDevice> PickPhysicalDevice(VkInstance instance)
	{
		U32 devicesCount = 0;
		vkEnumeratePhysicalDevices(instance, &devicesCount, nullptr);

		if (!devicesCount)
		{
			LOG_ERROR("[VulkanGraphicsContext] There are no supported physical devices in the system");
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		std::vector<VkPhysicalDevice> devices(devicesCount);
		vkEnumeratePhysicalDevices(instance, &devicesCount, devices.data());

		VkPhysicalDevice selectedDevice = VK_NULL_HANDLE;

		for (const auto& device : devices) {
			if (IsDeviceSuitable(device)) {
				selectedDevice = device;
				break;
			}
		}

		if (VK_NULL_HANDLE == selectedDevice)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		return Wrench::TOkValue<VkPhysicalDevice>(selectedDevice);
	}


	static TQueuesCreateInfo GetQueuesCreateInfo(VkPhysicalDevice physDevice, VkSurfaceKHR surface)
	{
		TQueuesCreateInfo info;

		U32 queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physDevice, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physDevice, &queueFamilyCount, queueFamilies.data());

		for (USIZE i = 0; i < queueFamilies.size(); i++)
		{
			if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				info.mGraphicsQueueIndex = static_cast<U32>(i);
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(physDevice, static_cast<U32>(i), surface, &presentSupport);
			if (presentSupport)
			{
				info.mPresentQueueIndex = static_cast<U32>(i);
			}

			if (info.IsValid())
			{
				break;
			}
		}

		return info;
	}


	static std::vector<VkDeviceQueueCreateInfo> PrepareQueuesCreateInfo(const TQueuesCreateInfo& info)
	{
		std::vector<VkDeviceQueueCreateInfo> queues;

		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = info.mGraphicsQueueIndex;
		queueCreateInfo.queueCount       = 1;

		queues.emplace_back(queueCreateInfo);

		return queues;
	}


	static TResult<VkDevice> CreateLogicDevice(VkPhysicalDevice physicalDevice, const TQueuesCreateInfo& queueCreateInfo)
	{
		auto&& queuesInfos = PrepareQueuesCreateInfo(queueCreateInfo);
		std::vector<F32> queuesPriorities(queuesInfos.size(), 1.0f);

		for (USIZE i = 0; i < queuesInfos.size(); i++)
		{
			queuesInfos[i].pQueuePriorities = &queuesPriorities[i];
		}

		VkPhysicalDeviceVulkan13Features device13Features{};
		device13Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
		device13Features.dynamicRendering = VK_TRUE;
		device13Features.synchronization2 = VK_TRUE;

		VkPhysicalDeviceRobustness2FeaturesEXT robustnessFeatures{};
		robustnessFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT;
		robustnessFeatures.nullDescriptor = true;

		IS_ROBUSTNESS2_EXTENSION_SUPPORTED = std::find_if(SupportedDeviceExtensions.cbegin(), SupportedDeviceExtensions.cend(), [](auto&& ext) { return strcmp(ext, VK_EXT_ROBUSTNESS_2_EXTENSION_NAME) == 0; }) != SupportedDeviceExtensions.cend();
		if (IS_ROBUSTNESS2_EXTENSION_SUPPORTED)
		{
			device13Features.pNext = &robustnessFeatures;
		}

		VkPhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.depthBiasClamp = VK_TRUE;
		deviceFeatures.depthClamp     = VK_TRUE;
		deviceFeatures.geometryShader = VK_TRUE;

		VkDeviceCreateInfo createInfo{};
		createInfo.sType                = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pNext                = &device13Features;
		createInfo.pQueueCreateInfos    = queuesInfos.data();
		createInfo.queueCreateInfoCount = static_cast<U32>(queuesInfos.size());
		createInfo.pEnabledFeatures     = &deviceFeatures;

		createInfo.enabledExtensionCount   = static_cast<U32>(SupportedDeviceExtensions.size());
		createInfo.ppEnabledExtensionNames = SupportedDeviceExtensions.data();

#if TDE2_DEBUG_MODE
		createInfo.ppEnabledLayerNames = ValidationLayers.data();
		createInfo.enabledLayerCount = static_cast<U32>(ValidationLayers.size());
#else
		createInfo.enabledLayerCount = 0;
#endif

		VkDevice device = VK_NULL_HANDLE;

		VkResult result = vkCreateDevice(physicalDevice, &createInfo, nullptr, &device);
		if (VK_SUCCESS != result)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(CVulkanMappings::GetErrorCode(result));
		}

		volkLoadDevice(device);

		return Wrench::TOkValue<VkDevice>(device);
	}


	struct TSwapChainSupportInfo
	{
		VkSurfaceCapabilitiesKHR mCapabilities;
		std::vector<VkSurfaceFormatKHR> mFormats;
		std::vector<VkPresentModeKHR> mPresentModes;
	};


	static TSwapChainSupportInfo GetSwapChainSupportInfo(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
	{
		TSwapChainSupportInfo info;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &info.mCapabilities);

		uint32_t formatsCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatsCount, nullptr);

		if (formatsCount)
		{
			info.mFormats.resize(formatsCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatsCount, info.mFormats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

		if (presentModeCount)
		{
			info.mPresentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, info.mPresentModes.data());
		}

		return info;
	}


	static TResult<VmaAllocator> InitMainAllocator(VkPhysicalDevice physicalDevice, VkDevice device, VkInstance instance)
	{
		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.physicalDevice = physicalDevice;
		allocatorInfo.device = device;
		allocatorInfo.instance = instance;

		// \note volk 2 vma function pointers
		VmaVulkanFunctions vma_vulkan_func{};
		vma_vulkan_func.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
		vma_vulkan_func.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
		vma_vulkan_func.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
		vma_vulkan_func.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
		vma_vulkan_func.vkAllocateMemory = vkAllocateMemory;
		vma_vulkan_func.vkFreeMemory = vkFreeMemory;
		vma_vulkan_func.vkMapMemory = vkMapMemory;
		vma_vulkan_func.vkUnmapMemory = vkUnmapMemory;
		vma_vulkan_func.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
		vma_vulkan_func.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
		vma_vulkan_func.vkBindBufferMemory = vkBindBufferMemory;
		vma_vulkan_func.vkBindImageMemory = vkBindImageMemory;
		vma_vulkan_func.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
		vma_vulkan_func.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
		vma_vulkan_func.vkCreateBuffer = vkCreateBuffer;
		vma_vulkan_func.vkDestroyBuffer = vkDestroyBuffer;
		vma_vulkan_func.vkCreateImage = vkCreateImage;
		vma_vulkan_func.vkDestroyImage = vkDestroyImage;
		vma_vulkan_func.vkCmdCopyBuffer = vkCmdCopyBuffer;

		allocatorInfo.pVulkanFunctions = &vma_vulkan_func;

		VmaAllocator allocator = VK_NULL_HANDLE;

		VkResult result = vmaCreateAllocator(&allocatorInfo, &allocator);
		if (VK_SUCCESS != result)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(CVulkanMappings::GetErrorCode(result));
		}

		return Wrench::TOkValue<VmaAllocator>(allocator);
	}


	CVulkanDeviceContext* CreateVulkanDeviceContext(TPtr<IWindowSystem>, TPtr<IWindowSurfaceFactory>, E_RESULT_CODE&);
	CVulkanSwapchain* CreateSwapchain(CVulkanDeviceContext*, TPtr<IWindowSystem>, E_RESULT_CODE&); 
	CVulkanCommandBuffer* CreateCommandBuffer(CVulkanDeviceContext*, E_RESULT_CODE&);


	/*!
		\brief CVulkanCommandBuffer's declaration
	*/

	class CVulkanCommandBuffer : public CBaseObject
	{
		public:
			friend CVulkanCommandBuffer* CreateCommandBuffer(CVulkanDeviceContext*, E_RESULT_CODE&);
		public:
			E_RESULT_CODE Init(CVulkanDeviceContext* pDeviceContext);

			E_RESULT_CODE Begin(VkCommandBufferUsageFlags flags = 0);
			E_RESULT_CODE End();

			E_RESULT_CODE Reset(VkCommandBufferResetFlags flags = 0);

			const VkCommandBuffer GetHandle() const { return mCommandBuffer; }
			const VkFence GetFence() const { return mFence; }
		private:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CVulkanCommandBuffer)

			E_RESULT_CODE _onFreeInternal() override;
		private:
			CVulkanDeviceContext* mpDeviceContext = nullptr;

			VkCommandPool         mCommandPool = VK_NULL_HANDLE;
			VkCommandBuffer       mCommandBuffer = VK_NULL_HANDLE;

			VkFence               mFence = VK_NULL_HANDLE;
	};


	/*!
		\brief CVulkanDeviceContext's definition
	*/

	class CVulkanDeviceContext : public CBaseObject
	{
		public:
			friend CVulkanDeviceContext* CreateVulkanDeviceContext(TPtr<IWindowSystem>, TPtr<IWindowSurfaceFactory>, E_RESULT_CODE&);
		public:
			E_RESULT_CODE Init(TPtr<IWindowSystem> pWindowSystem, TPtr<IWindowSurfaceFactory> pWindowSurfaceFactory);

			TPtr<CVulkanSwapchain> CreateSwapchain();

			VkFence CreateFence(bool signaled = true);
			VkSemaphore CreateSemaphore(VkSemaphoreCreateFlags flags = 0x0);

			E_RESULT_CODE SubmitCommands(TPtr<CVulkanCommandBuffer> pCommandBuffer, VkSemaphore waitSemaphore = VK_NULL_HANDLE, VkSemaphore signalSemaphore = VK_NULL_HANDLE, VkFence fence = VK_NULL_HANDLE);
			void WaitForIdle();

			const VkPhysicalDevice GetPhysicalDevice() const { return mPhysicalDevice; }
			const VkDevice GetDevice() const { return mDevice; }
			const VkSurfaceKHR GetSwapchainSurface() const { return mSwapChainSurface; }
			const VkInstance GetInstance() const { return mInstance; }

			const VkQueue GetGraphicsQueue() const { return mGraphicsQueue; }
			const TQueuesCreateInfo& GetQueuesInfo() const { return mQueuesInfo; }

			const VmaAllocator GetMemoryAllocator() const { return mMainAllocator; }
		private:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CVulkanDeviceContext)

			E_RESULT_CODE _onFreeInternal() override;
		private:
			TPtr<IWindowSystem>         mpWindowSystem = nullptr;
			TPtr<IWindowSurfaceFactory> mpWindowSurfaceFactory = nullptr;

			VkInstance                  mInstance = VK_NULL_HANDLE;
			VkPhysicalDevice            mPhysicalDevice = VK_NULL_HANDLE;
			VkDevice                    mDevice = VK_NULL_HANDLE;

			VkSurfaceKHR                mSwapChainSurface = VK_NULL_HANDLE;

			// queues
			VkQueue                     mGraphicsQueue = VK_NULL_HANDLE;
			VkQueue                     mPresentQueue = VK_NULL_HANDLE;

			TQueuesCreateInfo           mQueuesInfo{};

#if TDE2_DEBUG_MODE
			VkDebugUtilsMessengerEXT    mDebugMessenger = VK_NULL_HANDLE;
#endif

			VmaAllocator                mMainAllocator = VK_NULL_HANDLE;
	};


	CVulkanDeviceContext::CVulkanDeviceContext() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CVulkanDeviceContext::Init(TPtr<IWindowSystem> pWindowSystem, TPtr<IWindowSurfaceFactory> pWindowSurfaceFactory)
	{
		mpWindowSystem = pWindowSystem;
		mpWindowSurfaceFactory = pWindowSurfaceFactory;

		auto createInstanceResult = CreateInstance(mpWindowSystem->GetTitle());
		if (createInstanceResult.HasError())
		{
			return createInstanceResult.GetError();
		}

		mInstance = createInstanceResult.Get();

#if TDE2_DEBUG_MODE
		InitDebugMessageOutput(mInstance, mDebugMessenger);
#endif

		mSwapChainSurface = mpWindowSurfaceFactory->GetSurface(mInstance);
		if (VK_NULL_HANDLE == mSwapChainSurface)
		{
			LOG_ERROR("[VulkanGraphicsContext] Failed on creating window surface");
			return RC_FAIL;
		}

		auto pickPhysicalDeviceResult = PickPhysicalDevice(mInstance);
		if (pickPhysicalDeviceResult.HasError())
		{
			return pickPhysicalDeviceResult.GetError();
		}

		mPhysicalDevice = pickPhysicalDeviceResult.Get();

		mQueuesInfo = GetQueuesCreateInfo(mPhysicalDevice, mSwapChainSurface);
		if (!mQueuesInfo.IsValid())
		{
			return RC_FAIL;
		}

		auto createLogicalDeviceResult = CreateLogicDevice(mPhysicalDevice, mQueuesInfo);
		if (createLogicalDeviceResult.HasError())
		{
			return createLogicalDeviceResult.GetError();
		}

		mDevice = createLogicalDeviceResult.Get();

		vkGetDeviceQueue(mDevice, mQueuesInfo.mGraphicsQueueIndex, 0, &mGraphicsQueue);
		vkGetDeviceQueue(mDevice, mQueuesInfo.mPresentQueueIndex, 0, &mPresentQueue);

		auto allocatorCreateResult = InitMainAllocator(mPhysicalDevice, mDevice, mInstance);
		if (allocatorCreateResult.HasError())
		{
			return allocatorCreateResult.GetError();
		}

		mMainAllocator = allocatorCreateResult.Get();

		mIsInitialized = true;

		return RC_OK;
	}

	TPtr<CVulkanSwapchain> CVulkanDeviceContext::CreateSwapchain()
	{
		E_RESULT_CODE result = RC_OK;
		return TPtr<CVulkanSwapchain>(::TDEngine2::CreateSwapchain(this, mpWindowSystem, result));
	}

	VkFence CVulkanDeviceContext::CreateFence(bool signaled)
	{
		VkFenceCreateInfo fenceCreateInfo{};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : static_cast<VkFenceCreateFlags>(0);
		
		VkFence fence = VK_NULL_HANDLE;

		VkResult result = vkCreateFence(mDevice, &fenceCreateInfo, nullptr, &fence);
		if (VkResult::VK_SUCCESS != result)
		{
			TDE2_ASSERT(false);
			return VK_NULL_HANDLE;
		}

		return fence;
	}

	VkSemaphore CVulkanDeviceContext::CreateSemaphore(VkSemaphoreCreateFlags flags)
	{
		VkSemaphoreCreateInfo semaphoreCreateInfo{};
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreCreateInfo.flags = flags;

		VkSemaphore semaphore = VK_NULL_HANDLE;

		VkResult result = vkCreateSemaphore(mDevice, &semaphoreCreateInfo, nullptr, &semaphore);
		if (VkResult::VK_SUCCESS != result)
		{
			TDE2_ASSERT(false);
			return VK_NULL_HANDLE;
		}

		return semaphore;
	}


	E_RESULT_CODE CVulkanDeviceContext::SubmitCommands(TPtr<CVulkanCommandBuffer> pCommandBuffer, VkSemaphore waitSemaphore, VkSemaphore signalSemaphore, VkFence fence)
	{
		VkCommandBufferSubmitInfo cmdBufferSubmitInfo{};
		cmdBufferSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
		cmdBufferSubmitInfo.commandBuffer = pCommandBuffer->GetHandle();
		cmdBufferSubmitInfo.deviceMask = 0;

		// semaphores
		VkSemaphoreSubmitInfo waitSemaphoreSubmitInfo{};
		waitSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
		waitSemaphoreSubmitInfo.semaphore = waitSemaphore;
		waitSemaphoreSubmitInfo.value = 1;
		waitSemaphoreSubmitInfo.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR;
		waitSemaphoreSubmitInfo.deviceIndex = 0;

		VkSemaphoreSubmitInfo signalSemaphoreSubmitInfo{};
		signalSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
		signalSemaphoreSubmitInfo.semaphore = signalSemaphore;
		signalSemaphoreSubmitInfo.value = 1;
		signalSemaphoreSubmitInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
		signalSemaphoreSubmitInfo.deviceIndex = 0;

		VkSubmitInfo2 submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
		submitInfo.waitSemaphoreInfoCount = static_cast<uint32_t>(waitSemaphore == VK_NULL_HANDLE ? 0 : 1);
		submitInfo.pWaitSemaphoreInfos = &waitSemaphoreSubmitInfo;
		submitInfo.commandBufferInfoCount = 1;
		submitInfo.pCommandBufferInfos = &cmdBufferSubmitInfo;
		submitInfo.signalSemaphoreInfoCount = static_cast<uint32_t>(signalSemaphore == VK_NULL_HANDLE ? 0 : 1);
		submitInfo.pSignalSemaphoreInfos = &signalSemaphoreSubmitInfo;

		VK_SAFE_CALL(vkQueueSubmit2(mGraphicsQueue, 1, &submitInfo, fence));

		return RC_OK;
	}

	void CVulkanDeviceContext::WaitForIdle()
	{
		VK_SAFE_VOID_CALL(vkDeviceWaitIdle(mDevice));
	}

	E_RESULT_CODE CVulkanDeviceContext::_onFreeInternal()
	{
#if TDE2_DEBUG_MODE
		DestroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);
#endif

		vmaDestroyAllocator(mMainAllocator);

		//vkDestroyFence(mDevice, mTransferCommandFence, nullptr);

		vkDestroyDevice(mDevice, nullptr);
		vkDestroySurfaceKHR(mInstance, mSwapChainSurface, nullptr);
		vkDestroyInstance(mInstance, nullptr);

		return RC_OK;
	}


	CVulkanDeviceContext* CreateVulkanDeviceContext(TPtr<IWindowSystem> pWindowSystem, TPtr<IWindowSurfaceFactory> pWindowSurfaceFactory, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(CVulkanDeviceContext, CVulkanDeviceContext, result, pWindowSystem, pWindowSurfaceFactory);
	}


	TDE2_DEFINE_SCOPED_PTR(CVulkanDeviceContext)


	/*!
		\brief CVulkanSwapchain's definition
	*/

	class CVulkanSwapchain: public CBaseObject
	{
		public:
			friend CVulkanSwapchain* CreateSwapchain(CVulkanDeviceContext*, TPtr<IWindowSystem> pWindowSystem, E_RESULT_CODE&);
		public:
			E_RESULT_CODE Init(CVulkanDeviceContext* pDeviceContext, TPtr<IWindowSystem> pWindowSystem);

			E_RESULT_CODE AcquireNextImage(VkSemaphore semaphore);
			E_RESULT_CODE Present(VkSemaphore waitSemaphore = VK_NULL_HANDLE);

			void InvalidateState() { mIsValid = false; }
			E_RESULT_CODE TryProcessInvalidateState();

			uint32_t GetImageIndex() const { return mCurrImageIndex; }
			VkImage GetCurrImage() const { return mSwapChainImages[mCurrImageIndex]; }
			VkImageView GetCurrImageView() const { return mSwapChainImageViews[mCurrImageIndex]; }

			E_FORMAT_TYPE GetBackBuffersFormat() const { return mBackBufferFormat; }

			const VkSwapchainKHR GetHandle() const { return mSwapChain; }

			bool IsValid() const { return mIsValid; }
		private:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CVulkanSwapchain)

			E_RESULT_CODE _onInitInternal();
			E_RESULT_CODE _onFreeInternal() override;
		private:
			CVulkanDeviceContext*    mpDeviceContext = nullptr;
			TPtr<IWindowSystem>      mpWindowSystem = nullptr;

			VkSwapchainKHR           mSwapChain = VK_NULL_HANDLE;
			VkSurfaceFormatKHR       mSwapChainFormat{};
			VkExtent2D               mSwapChainExtents{};
			std::vector<VkImage>     mSwapChainImages {};
			std::vector<VkImageView> mSwapChainImageViews {};

			U32                      mCurrImageIndex = 0;

			E_FORMAT_TYPE            mBackBufferFormat = E_FORMAT_TYPE::FT_UNKNOWN;

			bool                     mIsValid = true;
	};


	CVulkanSwapchain::CVulkanSwapchain() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CVulkanSwapchain::Init(CVulkanDeviceContext* pDeviceContext, TPtr<IWindowSystem> pWindowSystem)
	{
		mpDeviceContext = pDeviceContext;
		mpWindowSystem  = pWindowSystem;

		return _onInitInternal();
	}

	E_RESULT_CODE CVulkanSwapchain::AcquireNextImage(VkSemaphore semaphore)
	{
		const VkResult result = vkAcquireNextImageKHR(mpDeviceContext->GetDevice(), mSwapChain, UINT64_MAX, semaphore, nullptr, &mCurrImageIndex);
		if (VK_ERROR_OUT_OF_DATE_KHR == result)
		{
			mIsValid = false;
			return RC_FAIL;
		}

		return RC_OK;
	}

	E_RESULT_CODE CVulkanSwapchain::Present(VkSemaphore waitSemaphore)
	{
		VkPresentInfoKHR presentInfo{};
		
		presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores    = &waitSemaphore;
		presentInfo.swapchainCount     = 1;
		presentInfo.pSwapchains        = &mSwapChain;
		presentInfo.pImageIndices      = &mCurrImageIndex;

		const VkResult result = vkQueuePresentKHR(mpDeviceContext->GetGraphicsQueue(), &presentInfo);
		if (VK_ERROR_OUT_OF_DATE_KHR == result)
		{
			mIsValid = false;
		}

		return RC_OK;
	}

	E_RESULT_CODE CVulkanSwapchain::TryProcessInvalidateState()
	{
		if (mIsValid)
		{
			return RC_OK;
		}

		mpDeviceContext->WaitForIdle();

		E_RESULT_CODE result = RC_OK;

		result = result | _onFreeInternal();
		result = result | _onInitInternal();

		return result;
	}

	E_RESULT_CODE CVulkanSwapchain::_onInitInternal()
	{
		const U32 flags = mpWindowSystem->GetFlags();

		auto swapChainSupportInfo = GetSwapChainSupportInfo(mpDeviceContext->GetPhysicalDevice(), mpDeviceContext->GetSwapchainSurface());
		if (swapChainSupportInfo.mFormats.empty() || swapChainSupportInfo.mPresentModes.empty())
		{
			return RC_FAIL;
		}

		const bool needsHardwareGammaCorrection = flags & P_HARDWARE_GAMMA_CORRECTION;

		mSwapChainFormat = swapChainSupportInfo.mFormats.front();
		for (auto&& currFormatInfo : swapChainSupportInfo.mFormats)
		{
			if (currFormatInfo.format == (needsHardwareGammaCorrection ? VK_FORMAT_B8G8R8A8_SRGB : VK_FORMAT_B8G8R8A8_UNORM)
				&& currFormatInfo.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				mSwapChainFormat = currFormatInfo;
				break;
			}
		}

		mBackBufferFormat = E_FORMAT_TYPE::FT_UBYTE4_BGRA_UNORM; // \todo

		const bool needsVSyncEnabled = flags & P_VSYNC;

		VkPresentModeKHR presentMode = swapChainSupportInfo.mPresentModes.front();
		for (auto&& currPresentModeInfo : swapChainSupportInfo.mPresentModes)
		{
			if (currPresentModeInfo == (needsVSyncEnabled ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR))
			{
				presentMode = currPresentModeInfo;
				break;
			}
		}

		mSwapChainExtents = swapChainSupportInfo.mCapabilities.currentExtent;
		if (mSwapChainExtents.width == std::numeric_limits<U32>::max())
		{
			auto&& windowRect = mpWindowSystem->GetClientRect();

			mSwapChainExtents.width  = windowRect.width;
			mSwapChainExtents.height = windowRect.height;
		}

		U32 imagesCount = swapChainSupportInfo.mCapabilities.minImageCount + 1;
		imagesCount = std::min(imagesCount, swapChainSupportInfo.mCapabilities.maxImageCount);

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface          = mpDeviceContext->GetSwapchainSurface();
		createInfo.minImageCount    = imagesCount;
		createInfo.imageFormat      = mSwapChainFormat.format;
		createInfo.imageColorSpace  = mSwapChainFormat.colorSpace;
		createInfo.imageExtent      = mSwapChainExtents;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		auto queuesInfo = GetQueuesCreateInfo(mpDeviceContext->GetPhysicalDevice(), mpDeviceContext->GetSwapchainSurface());

		std::array<U32, 2> queuesIndices
		{
			queuesInfo.mGraphicsQueueIndex,
				queuesInfo.mPresentQueueIndex,
		};

		if (queuesInfo.mGraphicsQueueIndex != queuesInfo.mPresentQueueIndex)
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queuesIndices.data();
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		createInfo.preTransform = swapChainSupportInfo.mCapabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		VK_SAFE_CALL(vkCreateSwapchainKHR(mpDeviceContext->GetDevice(), &createInfo, nullptr, &mSwapChain));
		VK_SAFE_CALL(vkGetSwapchainImagesKHR(mpDeviceContext->GetDevice(), mSwapChain, &imagesCount, nullptr));

		mSwapChainImages.resize(static_cast<USIZE>(imagesCount));
		VK_SAFE_CALL(vkGetSwapchainImagesKHR(mpDeviceContext->GetDevice(), mSwapChain, &imagesCount, mSwapChainImages.data()));

		mSwapChainImageViews.resize(mSwapChainImages.size());

		for (USIZE i = 0; i < mSwapChainImageViews.size(); i++)
		{
			VkImageViewCreateInfo createInfo{};
			createInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image                           = mSwapChainImages[i];
			createInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format                          = mSwapChainFormat.format;
			createInfo.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel   = 0;
			createInfo.subresourceRange.levelCount     = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount     = 1;

			VK_SAFE_CALL(vkCreateImageView(mpDeviceContext->GetDevice(), &createInfo, nullptr, &mSwapChainImageViews[i]));
		}

		mIsInitialized = true;
		mIsValid = true;

		return RC_OK;
	}

	E_RESULT_CODE CVulkanSwapchain::_onFreeInternal()
	{
		for (VkImageView& currImageView : mSwapChainImageViews)
		{
			vkDestroyImageView(mpDeviceContext->GetDevice(), currImageView, nullptr);
		}

		vkDestroySwapchainKHR(mpDeviceContext->GetDevice(), mSwapChain, nullptr);

		return RC_OK;
	}


	CVulkanSwapchain* CreateSwapchain(CVulkanDeviceContext* pDeviceContext, TPtr<IWindowSystem> pWindowSystem, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(CVulkanSwapchain, CVulkanSwapchain, result, pDeviceContext, pWindowSystem);
	}


	TDE2_DEFINE_SCOPED_PTR(CVulkanSwapchain)


	/*!
		\brief CVulkanCommandBuffer's definition
	*/

	CVulkanCommandBuffer::CVulkanCommandBuffer() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CVulkanCommandBuffer::Init(CVulkanDeviceContext* pDeviceContext)
	{
		mpDeviceContext = pDeviceContext;
		
		VkCommandPoolCreateInfo commandPoolCreateInfo{};
		commandPoolCreateInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		commandPoolCreateInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		commandPoolCreateInfo.queueFamilyIndex = pDeviceContext->GetQueuesInfo().mGraphicsQueueIndex;

		VK_SAFE_CALL(vkCreateCommandPool(pDeviceContext->GetDevice(), &commandPoolCreateInfo, nullptr, &mCommandPool));

		VkCommandBufferAllocateInfo commandBufferCreateInfo{};
		commandBufferCreateInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferCreateInfo.commandPool        = mCommandPool;
		commandBufferCreateInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commandBufferCreateInfo.commandBufferCount = 1;

		VK_SAFE_CALL(vkAllocateCommandBuffers(pDeviceContext->GetDevice(), &commandBufferCreateInfo, &mCommandBuffer));
		
		mFence = pDeviceContext->CreateFence();

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CVulkanCommandBuffer::Begin(VkCommandBufferUsageFlags flags)
	{
		VkCommandBufferBeginInfo commandBufferBeginInfo{};		
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		commandBufferBeginInfo.flags = flags;

		VK_SAFE_CALL(vkBeginCommandBuffer(mCommandBuffer, &commandBufferBeginInfo));

		return RC_OK;
	}

	E_RESULT_CODE CVulkanCommandBuffer::End()
	{
		VK_SAFE_CALL(vkEndCommandBuffer(mCommandBuffer));
		return RC_OK;
	}

	E_RESULT_CODE CVulkanCommandBuffer::Reset(VkCommandBufferResetFlags flags)
	{
		VK_SAFE_CALL(vkResetCommandBuffer(mCommandBuffer, flags));
		return RC_OK;
	}

	E_RESULT_CODE CVulkanCommandBuffer::_onFreeInternal()
	{
		vkDestroyCommandPool(mpDeviceContext->GetDevice(), mCommandPool, nullptr);
		vkDestroyFence(mpDeviceContext->GetDevice(), mFence, nullptr);

		return RC_OK;
	}


	CVulkanCommandBuffer* CreateCommandBuffer(CVulkanDeviceContext* pDeviceContext, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(CVulkanCommandBuffer, CVulkanCommandBuffer, result, pDeviceContext);
	}


	TDE2_DEFINE_SCOPED_PTR(CVulkanCommandBuffer);


	/*!
		\brief CVulkanGraphicsContext's definition
	*/


	CVulkanGraphicsContext::CVulkanGraphicsContext(TPtr<IWindowSurfaceFactory> pWindowSurfaceFactory) :
		CBaseObject(), mpWindowSurfaceFactory(pWindowSurfaceFactory)
	{
	}

	CVulkanGraphicsContext::CVulkanGraphicsContext() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CVulkanGraphicsContext::Init(TPtr<IWindowSystem> pWindowSystem)
	{
		TDE2_PROFILER_SCOPE("CVulkanGraphicsContext::Init");

		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pWindowSystem || !mpWindowSurfaceFactory)
		{
			return RC_INVALID_ARGS;
		}

		mpWindowSystem = pWindowSystem;
		mpEventManager = pWindowSystem->GetEventManager();

		if (!mpEventManager)
		{
			return RC_FAIL;
		}

		mpEventManager->Subscribe(TOnWindowResized::GetTypeId(), this);

		E_RESULT_CODE result = _onInitInternal();
		if (RC_OK != result)
		{
			return result;
		}

		mpGraphicsObjectManager = TPtr<IGraphicsObjectManager>(CreateVulkanGraphicsObjectManager(this, result));
		if (result != RC_OK)
		{
			return result;
		}

		mpGraphicsObjectManagerImpl = dynamic_cast<CVulkanGraphicsObjectManager*>(mpGraphicsObjectManager.Get());

		mDescriptorWrites.reserve(
			TDescriptorsBindingsTable::MAX_CBV_COUNT +
			TDescriptorsBindingsTable::MAX_SRV_COUNT +
			TDescriptorsBindingsTable::MAX_SAMPLERS_COUNT +
			TDescriptorsBindingsTable::MAX_UAV_COUNT);

		mDescriptorBufferInfos.reserve(
			TDescriptorsBindingsTable::MAX_CBV_COUNT +
			TDescriptorsBindingsTable::MAX_SRV_COUNT +
			TDescriptorsBindingsTable::MAX_UAV_COUNT);

		mDescriptorImageInfos.reserve(
			TDescriptorsBindingsTable::MAX_SRV_COUNT +
			TDescriptorsBindingsTable::MAX_SAMPLERS_COUNT +
			TDescriptorsBindingsTable::MAX_UAV_COUNT);

		if (!IS_ROBUSTNESS2_EXTENSION_SUPPORTED)
		{
			mEmptyStructuredBufferHandle = mpGraphicsObjectManager->CreateBuffer(
				{
					E_BUFFER_USAGE_TYPE::DYNAMIC,
					E_BUFFER_TYPE::STRUCTURED,
					32,
					nullptr,
					32,
					false,
					32,
					E_STRUCTURED_BUFFER_TYPE::DEFAULT
				}).GetOrDefault(TBufferHandleId::Invalid);

			TInitTextureImplParams emptyTextureInitParams;

			emptyTextureInitParams.mWidth = 1;
			emptyTextureInitParams.mHeight = 1;
			emptyTextureInitParams.mFormat = FT_FLOAT1;
			emptyTextureInitParams.mNumOfMipLevels = 1;
			emptyTextureInitParams.mNumOfSamples = 1;
			emptyTextureInitParams.mSamplingQuality = 0;
			emptyTextureInitParams.mType = E_TEXTURE_IMPL_TYPE::TEXTURE_2D;
			emptyTextureInitParams.mUsageType = E_TEXTURE_IMPL_USAGE_TYPE::STATIC;
			emptyTextureInitParams.mBindFlags = E_BIND_GRAPHICS_TYPE::BIND_SHADER_RESOURCE | E_BIND_GRAPHICS_TYPE::BIND_RENDER_TARGET;
			emptyTextureInitParams.mName = "DEFAULT";

			mEmptyTextureHandle = mpGraphicsObjectManager->CreateTexture(emptyTextureInitParams).GetOrDefault(TTextureHandleId::Invalid);
		}

		mIsInitialized = true;

		return RC_OK;
	}



	E_RESULT_CODE CVulkanGraphicsContext::_onInitInternal()
	{
		VK_SAFE_CALL(volkInitialize());

		E_RESULT_CODE result = RC_OK;
		
		mpVulkanDeviceContext = TPtr<CVulkanDeviceContext>(CreateVulkanDeviceContext(mpWindowSystem, mpWindowSurfaceFactory, result));
		if (RC_OK != result)
		{
			return result;
		}

		mpSwapchain = mpVulkanDeviceContext->CreateSwapchain();
		TDE2_ASSERT(mpSwapchain);

		result = _prepareFrameData();
		if (RC_OK != result)
		{
			return result;
		}

		mpImmediateCommandBuffer = TPtr<CVulkanCommandBuffer>(CreateCommandBuffer(mpVulkanDeviceContext.Get(), result));
		if (RC_OK != result)
		{
			return result;
		}

		mDescriptorsBindingsTable.Reset();

		return RC_OK;
	}


	static void CleanupGarbage(CVulkanGraphicsContext::TGarbageCollection& garbageArray, VkDevice device, VmaAllocator allocator)
	{
		// destroy objects that were marked for deletion
		for (auto&& currGarbageEntity : garbageArray)
		{
			switch (currGarbageEntity.mType)
			{
			case CVulkanGraphicsContext::TGarbageEntity::E_TYPE::BUFFER:
				vmaDestroyBuffer(allocator, currGarbageEntity.mData.mBufferHandle, currGarbageEntity.mAllocation);
				break;
			case CVulkanGraphicsContext::TGarbageEntity::E_TYPE::IMAGE:
				vmaDestroyImage(allocator, currGarbageEntity.mData.mImageHandle, currGarbageEntity.mAllocation);
				break;
			case CVulkanGraphicsContext::TGarbageEntity::E_TYPE::IMAGE_VIEW:
				vkDestroyImageView(device, currGarbageEntity.mData.mImageViewHandle, nullptr);
				break;
			}
		}

		garbageArray.clear();
	}


	E_RESULT_CODE CVulkanGraphicsContext::_onFreeInternal()
	{
		mpVulkanDeviceContext->WaitForIdle();

		for (USIZE i = 0; i < FRAMES_COUNT; i++)
		{
			vkDestroySemaphore(mpVulkanDeviceContext->GetDevice(), mImageReadySemaphores[i], nullptr);
			vkDestroySemaphore(mpVulkanDeviceContext->GetDevice(), mRenderFinishedSemaphores[i], nullptr);
		}

		for (auto&& currCachedPipelineEntry : mCachedPipelinesLibrary)
		{
			vkDestroyPipeline(mpVulkanDeviceContext->GetDevice(), currCachedPipelineEntry.second, nullptr);
		}

		mCachedPipelinesLibrary.clear();

		for (TPtr<CVulkanCommandBuffer>& pCommandBuffer : mpCommandBuffers)
		{
			pCommandBuffer = nullptr;
		}

		mpGraphicsObjectManager     = nullptr;
		mpGraphicsObjectManagerImpl = nullptr;

		// \note Clean up is invoked only after all resources are freed in mpGraphicsObjectManager
		{
			std::lock_guard<std::mutex> lock(mGarbageCollectorMutex);

			for (auto&& currGarbageArray : mAwaitingDeletionObjects)
			{
				CleanupGarbage(currGarbageArray, mpVulkanDeviceContext->GetDevice(), mpVulkanDeviceContext->GetMemoryAllocator());
			}
		}

		mpImmediateCommandBuffer    = nullptr;
		mpSwapchain                 = nullptr;
		mpVulkanDeviceContext       = nullptr;

		return RC_OK;
	}

	E_RESULT_CODE CVulkanGraphicsContext::DestroyObjectDeffered(VkBuffer bufferHandle, VmaAllocation allocation)
	{
		std::lock_guard<std::mutex> lock(mGarbageCollectorMutex);

		TGarbageEntity garbageEntity;
		garbageEntity.mData.mBufferHandle = bufferHandle;
		garbageEntity.mType               = TGarbageEntity::E_TYPE::BUFFER;
		garbageEntity.mAllocation         = allocation;

		mAwaitingDeletionObjects[mCurrFrameIndex].emplace_back(garbageEntity);

		return RC_OK;
	}

	E_RESULT_CODE CVulkanGraphicsContext::DestroyObjectDeffered(VkImage imageHandle, VmaAllocation allocation)
	{
		std::lock_guard<std::mutex> lock(mGarbageCollectorMutex);

		TGarbageEntity garbageEntity;
		garbageEntity.mData.mImageHandle = imageHandle;
		garbageEntity.mType              = TGarbageEntity::E_TYPE::IMAGE;
		garbageEntity.mAllocation        = allocation;

		mAwaitingDeletionObjects[mCurrFrameIndex].emplace_back(garbageEntity);

		return RC_OK;
	}

	E_RESULT_CODE CVulkanGraphicsContext::DestroyObjectDeffered(VkImageView imageViewHandle)
	{
		std::lock_guard<std::mutex> lock(mGarbageCollectorMutex);

		TGarbageEntity garbageEntity;
		garbageEntity.mData.mImageViewHandle = imageViewHandle;
		garbageEntity.mType                  = TGarbageEntity::E_TYPE::IMAGE_VIEW;

		mAwaitingDeletionObjects[mCurrFrameIndex].emplace_back(garbageEntity);

		return RC_OK;
	}

	E_RESULT_CODE CVulkanGraphicsContext::ExecuteImmediate(const std::function<void(VkCommandBuffer)>& copyCommand)
	{
		if (!copyCommand)
		{
			return RC_INVALID_ARGS;
		}

		VkFence cmdBufferFence = mpImmediateCommandBuffer->GetFence();

		vkResetFences(mpVulkanDeviceContext->GetDevice(), 1, &cmdBufferFence);
		mpImmediateCommandBuffer->Reset();

		mpImmediateCommandBuffer->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		if (copyCommand)
		{
			copyCommand(mpImmediateCommandBuffer->GetHandle());
		}

		mpImmediateCommandBuffer->End();

		mpVulkanDeviceContext->SubmitCommands(mpImmediateCommandBuffer, VK_NULL_HANDLE, VK_NULL_HANDLE, cmdBufferFence);
		VK_SAFE_VOID_CALL(vkWaitForFences(mpVulkanDeviceContext->GetDevice(), 1, &cmdBufferFence, true, UINT64_MAX));

		return RC_OK;
	}

	void CVulkanGraphicsContext::BeginFrame()
	{
		if (!mpSwapchain->IsValid())
		{
			E_RESULT_CODE result = mpSwapchain->TryProcessInvalidateState();
			TDE2_ASSERT(RC_OK == result);
		}

		const TPtr<CVulkanCommandBuffer>& pCurrCommandBuffer = mpCommandBuffers[mCurrFrameIndex];
		VkFence commandBufferFence = pCurrCommandBuffer->GetFence();

		VK_SAFE_VOID_CALL(vkWaitForFences(mpVulkanDeviceContext->GetDevice(), 1, &commandBufferFence, VK_TRUE, UINT64_MAX));
		VK_SAFE_VOID_CALL(vkResetFences(mpVulkanDeviceContext->GetDevice(), 1, &commandBufferFence));
		mpVulkanDeviceContext->WaitForIdle(); // \todo Remove this later when sync issue will be fixed

		mpSwapchain->AcquireNextImage(mImageReadySemaphores[mCurrFrameIndex]);

		{
			std::lock_guard<std::mutex> lock(mGarbageCollectorMutex);
			CleanupGarbage(mAwaitingDeletionObjects[mCurrFrameIndex], mpVulkanDeviceContext->GetDevice(), mpVulkanDeviceContext->GetMemoryAllocator());
		}

		pCurrCommandBuffer->Reset();
		pCurrCommandBuffer->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		mDescriptorsBindingsTable.Reset();
	}

	void CVulkanGraphicsContext::Present()
	{
		// \note Add final barrier for current swapchain's image
		VkImageMemoryBarrier2 barrier = {};
		barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
		barrier.image                           = mpSwapchain->GetCurrImage();
		barrier.oldLayout                       = VK_IMAGE_LAYOUT_UNDEFINED;
		barrier.newLayout                       = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		barrier.srcStageMask                    = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		barrier.srcAccessMask                   = VK_ACCESS_2_MEMORY_WRITE_BIT;
		barrier.dstStageMask                    = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
		barrier.dstAccessMask                   = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
		barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel   = 0;
		barrier.subresourceRange.levelCount     = VK_REMAINING_MIP_LEVELS;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount     = VK_REMAINING_ARRAY_LAYERS;
		barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;

		VkDependencyInfo dependencyInfo {};
		dependencyInfo.sType                   = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		dependencyInfo.imageMemoryBarrierCount = 1;
		dependencyInfo.pImageMemoryBarriers    = &barrier;
		vkCmdPipelineBarrier2(_getCurrCommandBufferHandle(), &dependencyInfo);

		const TPtr<CVulkanCommandBuffer>& pCurrCommandBuffer = mpCommandBuffers[mCurrFrameIndex];
		pCurrCommandBuffer->End();

		E_RESULT_CODE result = mpVulkanDeviceContext->SubmitCommands(pCurrCommandBuffer, mImageReadySemaphores[mCurrFrameIndex], mRenderFinishedSemaphores[mCurrFrameIndex], pCurrCommandBuffer->GetFence());
		TDE2_ASSERT(RC_OK == result);

		result = mpSwapchain->Present(mRenderFinishedSemaphores[mCurrFrameIndex]);
		TDE2_ASSERT(RC_OK == result);

		if (!mpSwapchain->IsValid()) // \note Assume that window's sizes are changed 
		{
			return;
		}

		mCurrFrameIndex = (mCurrFrameIndex + 1) & (FRAMES_COUNT - 1);
	}

	void CVulkanGraphicsContext::SetViewport(F32 x, F32 y, F32 width, F32 height, F32 minDepth, F32 maxDepth)
	{
		VkViewport viewport{};
		viewport.x        = x;
		viewport.y        = y + height;
		viewport.width    = width;
		viewport.height   = y - height;
		viewport.minDepth = minDepth;
		viewport.maxDepth = maxDepth;

		vkCmdSetViewport(_getCurrCommandBufferHandle(), 0, 1, &viewport);
	}

	void CVulkanGraphicsContext::SetScissorRect(const TRectU32& scissorRect)
	{
		TDE2_ASSERT(mIsRenderPassActive); // \note To set up scissor rect outside of some render pass is meaningless
		
		VkRect2D internalScissorRect{};
		internalScissorRect.offset = { static_cast<I32>(scissorRect.x), static_cast<I32>(scissorRect.y) };
		internalScissorRect.extent = { scissorRect.width, scissorRect.height };

		vkCmdSetScissor(_getCurrCommandBufferHandle(), 0, 1, &internalScissorRect);
	}

	TMatrix4 CVulkanGraphicsContext::CalcPerspectiveMatrix(F32 fov, F32 aspect, F32 zn, F32 zf)
	{
		return PerspectiveProj(fov, aspect, zn, zf, 0.0f, 1.0f, -1.0f);
	}

	TMatrix4 CVulkanGraphicsContext::CalcOrthographicMatrix(F32 left, F32 top, F32 right, F32 bottom, F32 zn, F32 zf, bool isDepthless)
	{
		return OrthographicProj(left, top, right, bottom, zn, zf, 0.0f, 1.0f, -1.0f, isDepthless);
	}

	E_RESULT_CODE CVulkanGraphicsContext::SetVertexBuffer(U32 slot, TBufferHandleId vertexBufferHandle, U32 offset, U32 strideSize)
	{
		auto pBuffer = mpGraphicsObjectManagerImpl->GetVulkanBufferPtr(vertexBufferHandle);
		if (!pBuffer)
		{
			return RC_FAIL;
		}

		TDE2_ASSERT(E_BUFFER_TYPE::VERTEX == pBuffer->GetParams().mBufferType);

		const VkBuffer vertexBuffers[] = { pBuffer->GetVulkanHandle() };
		const VkDeviceSize offsets[] = { static_cast<USIZE>(offset) };

		vkCmdBindVertexBuffers(mpCommandBuffers[mCurrFrameIndex]->GetHandle(), slot, 1, vertexBuffers, offsets);

		return RC_OK;
	}

	E_RESULT_CODE CVulkanGraphicsContext::SetIndexBuffer(TBufferHandleId indexBufferHandle, U32 offset)
	{
		auto pBuffer = mpGraphicsObjectManagerImpl->GetVulkanBufferPtr(indexBufferHandle);
		if (!pBuffer)
		{
			return RC_FAIL;
		}

		TDE2_ASSERT(E_BUFFER_TYPE::INDEX == pBuffer->GetParams().mBufferType);

		vkCmdBindIndexBuffer(mpCommandBuffers[mCurrFrameIndex]->GetHandle(), pBuffer->GetVulkanHandle(),
			static_cast<VkDeviceSize>(offset), CVulkanMappings::GetIndexFormat(pBuffer->GetParams().mIndexFormat));

		return RC_OK;
	}

	E_RESULT_CODE CVulkanGraphicsContext::SetConstantBuffer(U32 slot, TBufferHandleId constantsBufferHandle)
	{
		auto pBuffer = mpGraphicsObjectManagerImpl->GetVulkanBufferPtr(constantsBufferHandle);
		if (!pBuffer)
		{
			return RC_FAIL;
		}

		TDE2_ASSERT(E_BUFFER_TYPE::CONSTANT == pBuffer->GetParams().mBufferType);
		TDE2_ASSERT(slot < TDescriptorsBindingsTable::MAX_CBV_COUNT);

		mDescriptorsBindingsTable.mConstantBuffers[slot] = constantsBufferHandle;

		return RC_OK;
	}

	E_RESULT_CODE CVulkanGraphicsContext::SetStructuredBuffer(U32 slot, TBufferHandleId bufferHandle, bool isWriteEnabled)
	{
		auto pBuffer = mpGraphicsObjectManagerImpl->GetVulkanBufferPtr(bufferHandle);
		if (!pBuffer)
		{
			return RC_OK;
		}

		TDE2_ASSERT(E_BUFFER_TYPE::STRUCTURED == pBuffer->GetParams().mBufferType);

		if (isWriteEnabled)
		{
			TDE2_ASSERT(slot < mDescriptorsBindingsTable.mUAVBuffers.size());

			mDescriptorsBindingsTable.mUAVBuffers[slot].mType = TDescriptorsBindingsTable::E_DESCRIPTOR_TYPE::BUFFER;
			mDescriptorsBindingsTable.mUAVBuffers[slot].mValue.mBuffer = bufferHandle;
		}
		else
		{
			TDE2_ASSERT(slot < mDescriptorsBindingsTable.mSRVBuffers.size());

			mDescriptorsBindingsTable.mSRVBuffers[slot].mType = TDescriptorsBindingsTable::E_DESCRIPTOR_TYPE::BUFFER;
			mDescriptorsBindingsTable.mSRVBuffers[slot].mValue.mBuffer = bufferHandle;
		}

		return RC_OK;
	}

	E_RESULT_CODE CVulkanGraphicsContext::SetTexture(U32 slot, TTextureHandleId textureHandle, bool isWriteEnabled)
	{
		auto pTexture = mpGraphicsObjectManagerImpl->GetVulkanTexturePtr(textureHandle);
		if (!pTexture)
		{
			return RC_FAIL;
		}

		if (isWriteEnabled)
		{
			TDE2_ASSERT(slot < mDescriptorsBindingsTable.mUAVBuffers.size());

			mDescriptorsBindingsTable.mUAVBuffers[slot].mType = TDescriptorsBindingsTable::E_DESCRIPTOR_TYPE::TEXTURE;
			mDescriptorsBindingsTable.mUAVBuffers[slot].mValue.mTexture = textureHandle;
		}
		else
		{
			TDE2_ASSERT(slot < mDescriptorsBindingsTable.mSRVBuffers.size());

			mDescriptorsBindingsTable.mSRVBuffers[slot].mType = TDescriptorsBindingsTable::E_DESCRIPTOR_TYPE::TEXTURE;
			mDescriptorsBindingsTable.mSRVBuffers[slot].mValue.mTexture = textureHandle;
		}

		return RC_OK;
	}

	E_RESULT_CODE CVulkanGraphicsContext::SetSampler(U32 slot, TTextureSamplerId samplerHandle)
	{
		mDescriptorsBindingsTable.mSamplers[slot] = samplerHandle;
		return RC_OK;
	}

	E_RESULT_CODE CVulkanGraphicsContext::UpdateTexture2D(TTextureHandleId textureHandle, U32 mipLevel, const TRectI32& regionRect, const void* pData, USIZE dataSize)
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		auto pTexture = mpGraphicsObjectManagerImpl->GetVulkanTexturePtr(textureHandle);
		if (!pTexture)
		{
			return RC_FAIL;
		}

		const USIZE textureSize = static_cast<USIZE>(regionRect.width * regionRect.height * CFormatUtils::GetFormatSize(pTexture->GetParams().mFormat));

		TPtr<IBuffer> pStagingBuffer = TPtr<IBuffer>(CreateVulkanBuffer(this, { E_BUFFER_USAGE_TYPE::DYNAMIC, E_BUFFER_TYPE::GENERIC, textureSize, nullptr }, result));
		if (RC_OK != result || !pStagingBuffer)
		{
			return result;
		}

		TPtr<CVulkanBuffer> pVulkanStagingBuffer = DynamicPtrCast<CVulkanBuffer>(pStagingBuffer);

		result = pStagingBuffer->Map(E_BUFFER_MAP_TYPE::BMT_WRITE);
		if (RC_OK != result)
		{
			return result;
		}

		result = pStagingBuffer->Write(pData, textureSize);
		if (RC_OK != result)
		{
			return result;
		}

		pStagingBuffer->Unmap();

		auto pStagingBufferImpl = DynamicPtrCast<CVulkanBuffer>(pStagingBuffer);

		result = ExecuteImmediate([=](VkCommandBuffer cmdBuffer)
		{
			VkImageSubresourceRange range;
			range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			range.baseMipLevel = 0;
			range.levelCount = 1;
			range.baseArrayLayer = 0;
			range.layerCount = 1;

			VkImageMemoryBarrier imageBarrierToTransfer = {};
			imageBarrierToTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

			imageBarrierToTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageBarrierToTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imageBarrierToTransfer.image = pTexture->GetTextureHandle();
			imageBarrierToTransfer.subresourceRange = range;

			imageBarrierToTransfer.srcAccessMask = 0;
			imageBarrierToTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			//barrier the image into the transferreceive layout
			vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrierToTransfer);

			VkBufferImageCopy copyRegion = {};
			copyRegion.bufferOffset = 0;
			copyRegion.bufferRowLength = 0;
			copyRegion.bufferImageHeight = 0;

			auto&& textureParams = pTexture->GetParams();

			copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copyRegion.imageSubresource.mipLevel = 0;
			copyRegion.imageSubresource.baseArrayLayer = 0;
			copyRegion.imageSubresource.layerCount = 1;
			copyRegion.imageExtent = { static_cast<U32>(regionRect.width), static_cast<U32>(regionRect.height), 1 };
			copyRegion.imageOffset = { regionRect.x, regionRect.y, 0 };

			//copy the buffer into the image
			vkCmdCopyBufferToImage(cmdBuffer, pVulkanStagingBuffer->GetVulkanHandle(), pTexture->GetTextureHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

			VkImageMemoryBarrier imageBarrierToReadable = imageBarrierToTransfer;

			imageBarrierToReadable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imageBarrierToReadable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			imageBarrierToReadable.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			imageBarrierToReadable.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			//barrier the image into the shader readable layout
			vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrierToReadable);
		});

		return result;
	}

	E_RESULT_CODE CVulkanGraphicsContext::UpdateTexture2DArray(TTextureHandleId textureHandle, U32 index, const TRectI32& regionRect, const void* pData, USIZE dataSize)
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		auto pTexture = mpGraphicsObjectManagerImpl->GetVulkanTexturePtr(textureHandle);
		if (!pTexture)
		{
			return RC_FAIL;
		}

		const USIZE textureSize = static_cast<USIZE>(regionRect.width * regionRect.height * CFormatUtils::GetFormatSize(pTexture->GetParams().mFormat));

		TPtr<IBuffer> pStagingBuffer = TPtr<IBuffer>(CreateVulkanBuffer(this, { E_BUFFER_USAGE_TYPE::DYNAMIC, E_BUFFER_TYPE::GENERIC, textureSize, nullptr }, result));
		if (RC_OK != result || !pStagingBuffer)
		{
			return result;
		}

		TPtr<CVulkanBuffer> pVulkanStagingBuffer = DynamicPtrCast<CVulkanBuffer>(pStagingBuffer);

		result = pStagingBuffer->Map(E_BUFFER_MAP_TYPE::BMT_WRITE);
		if (RC_OK != result)
		{
			return result;
		}

		result = pStagingBuffer->Write(pData, textureSize);
		if (RC_OK != result)
		{
			return result;
		}

		pStagingBuffer->Unmap();

		auto pStagingBufferImpl = DynamicPtrCast<CVulkanBuffer>(pStagingBuffer);

		result = ExecuteImmediate([=](VkCommandBuffer cmdBuffer)
			{
				VkImageSubresourceRange range;
				range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				range.baseMipLevel = 0;
				range.levelCount = 1;
				range.baseArrayLayer = index;
				range.layerCount = 1;

				VkImageMemoryBarrier imageBarrierToTransfer = {};
				imageBarrierToTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

				imageBarrierToTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				imageBarrierToTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				imageBarrierToTransfer.image = pTexture->GetTextureHandle();
				imageBarrierToTransfer.subresourceRange = range;

				imageBarrierToTransfer.srcAccessMask = 0;
				imageBarrierToTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

				//barrier the image into the transferreceive layout
				vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrierToTransfer);

				VkBufferImageCopy copyRegion = {};
				copyRegion.bufferOffset = 0;
				copyRegion.bufferRowLength = 0;
				copyRegion.bufferImageHeight = 0;

				auto&& textureParams = pTexture->GetParams();

				copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				copyRegion.imageSubresource.mipLevel = 0;
				copyRegion.imageSubresource.baseArrayLayer = index;
				copyRegion.imageSubresource.layerCount = 1;
				copyRegion.imageExtent = { static_cast<U32>(regionRect.width), static_cast<U32>(regionRect.height), 1 };
				copyRegion.imageOffset = { regionRect.x, regionRect.y, 0 };

				//copy the buffer into the image
				vkCmdCopyBufferToImage(cmdBuffer, pVulkanStagingBuffer->GetVulkanHandle(), pTexture->GetTextureHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

				VkImageMemoryBarrier imageBarrierToReadable = imageBarrierToTransfer;

				imageBarrierToReadable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				imageBarrierToReadable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

				imageBarrierToReadable.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				imageBarrierToReadable.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				//barrier the image into the shader readable layout
				vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrierToReadable);
			});

		return result;
	}

	E_RESULT_CODE CVulkanGraphicsContext::UpdateCubemapTexture(TTextureHandleId textureHandle, E_CUBEMAP_FACE face, const TRectI32& regionRect, const void* pData, USIZE dataSize)
	{
		return UpdateTexture2DArray(textureHandle, static_cast<U32>(face), regionRect, pData, dataSize);
	}

	E_RESULT_CODE CVulkanGraphicsContext::UpdateTexture3D(TTextureHandleId textureHandle, U32 depthFrom, U32 depthTo, const TRectI32& regionRect, const void* pData, USIZE dataSize)
	{
		TDE2_UNIMPLEMENTED();
		return RC_NOT_IMPLEMENTED_YET;
	}

	E_RESULT_CODE CVulkanGraphicsContext::CopyResource(TTextureHandleId sourceHandle, TTextureHandleId destHandle)
	{
		if (sourceHandle == destHandle)
		{
			return RC_INVALID_ARGS;
		}

		TPtr<CVulkanTextureImpl> pSourceTexture = mpGraphicsObjectManagerImpl->GetVulkanTexturePtr(sourceHandle);
		if (!pSourceTexture)
		{
			return RC_INVALID_ARGS;
		}

		TPtr<CVulkanTextureImpl> pDestTexture = mpGraphicsObjectManagerImpl->GetVulkanTexturePtr(destHandle);
		if (!pDestTexture)
		{
			return RC_INVALID_ARGS;
		}

		const E_RESOURCE_LAYOUT currSourceLayout = pSourceTexture->GetLayout();
		const E_RESOURCE_LAYOUT currDestLayout   = pDestTexture->GetLayout();

		E_RESULT_CODE result = RC_OK;
		
		result = result | pSourceTexture->Transition(E_RESOURCE_LAYOUT::COPY_SRC);
		result = result | pDestTexture->Transition(E_RESOURCE_LAYOUT::COPY_DEST);

		FlushBarriers();

		const TInitTextureParams& sourceTexParams = pSourceTexture->GetParams();
		const TInitTextureParams& destTexParams   = pDestTexture->GetParams();

		VkImageCopy regionsInfo{};
		regionsInfo.extent.width  = sourceTexParams.mWidth;
		regionsInfo.extent.height = sourceTexParams.mHeight;
		regionsInfo.extent.depth  = sourceTexParams.mDepth;

		regionsInfo.srcSubresource.aspectMask = (E_FORMAT_TYPE::FT_D32 == sourceTexParams.mFormat) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		regionsInfo.srcSubresource.layerCount = 1;
		
		regionsInfo.dstSubresource.aspectMask = (E_FORMAT_TYPE::FT_D32 == destTexParams.mFormat) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		regionsInfo.dstSubresource.layerCount = 1;

		vkCmdCopyImage(_getCurrCommandBufferHandle(), pSourceTexture->GetTextureHandle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, pDestTexture->GetTextureHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &regionsInfo);

		if (E_RESOURCE_LAYOUT::UNDEFINED != currSourceLayout)
		{
			result = result | pSourceTexture->Transition(currSourceLayout);
		}

		if (E_RESOURCE_LAYOUT::UNDEFINED != currDestLayout)
		{
			result = result | pDestTexture->Transition(currDestLayout);
		}

		return result;
	}

	E_RESULT_CODE CVulkanGraphicsContext::CopyResource(TBufferHandleId sourceHandle, TTextureHandleId destHandle)
	{
		TPtr<CVulkanBuffer> pSourceBuffer = mpGraphicsObjectManagerImpl->GetVulkanBufferPtr(sourceHandle);
		if (!pSourceBuffer)
		{
			return RC_INVALID_ARGS;
		}

		TPtr<CVulkanTextureImpl> pDestTexture = mpGraphicsObjectManagerImpl->GetVulkanTexturePtr(destHandle);
		if (!pDestTexture)
		{
			return RC_INVALID_ARGS;
		}

		const TInitTextureParams& destTextureParams = pDestTexture->GetParams();

		VkBufferImageCopy regionsInfo{};
		regionsInfo.imageSubresource.aspectMask = CVulkanMappings::IsDepthTextureFormat(destTextureParams.mFormat) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		regionsInfo.imageSubresource.layerCount = VK_REMAINING_ARRAY_LAYERS;
		regionsInfo.imageExtent.width           = destTextureParams.mWidth;
		regionsInfo.imageExtent.height          = destTextureParams.mHeight;
		regionsInfo.imageExtent.depth           = destTextureParams.mDepth;

		const E_RESOURCE_LAYOUT currDestLayout = pDestTexture->GetLayout();

		E_RESULT_CODE result = pDestTexture->Transition(E_RESOURCE_LAYOUT::COPY_DEST);
		FlushBarriers();

		vkCmdCopyBufferToImage(_getCurrCommandBufferHandle(), pSourceBuffer->GetVulkanHandle(), pDestTexture->GetTextureHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &regionsInfo);
		
		if (E_RESOURCE_LAYOUT::UNDEFINED != currDestLayout)
		{
			result = result | pDestTexture->Transition(currDestLayout);
		}
		
		return result;
	}

	E_RESULT_CODE CVulkanGraphicsContext::CopyResource(TTextureHandleId sourceHandle, TBufferHandleId destHandle)
	{
		TPtr<CVulkanTextureImpl> pSourceTexture = mpGraphicsObjectManagerImpl->GetVulkanTexturePtr(sourceHandle);
		if (!pSourceTexture)
		{
			return RC_INVALID_ARGS;
		}

		TPtr<CVulkanBuffer> pDestBuffer = mpGraphicsObjectManagerImpl->GetVulkanBufferPtr(destHandle);
		if (!pDestBuffer)
		{
			return RC_INVALID_ARGS;
		}

		const TInitTextureParams& sourceTextureParams = pSourceTexture->GetParams();

		VkBufferImageCopy regionsInfo{};
		regionsInfo.imageSubresource.aspectMask = CVulkanMappings::IsDepthTextureFormat(sourceTextureParams.mFormat) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		regionsInfo.imageSubresource.layerCount = VK_REMAINING_ARRAY_LAYERS;
		regionsInfo.imageExtent.width           = sourceTextureParams.mWidth;
		regionsInfo.imageExtent.height          = sourceTextureParams.mHeight;
		regionsInfo.imageExtent.depth           = sourceTextureParams.mDepth;

		const E_RESOURCE_LAYOUT currSourceLayout = pSourceTexture->GetLayout();

		E_RESULT_CODE result = pSourceTexture->Transition(E_RESOURCE_LAYOUT::COPY_SRC);
		FlushBarriers();

		vkCmdCopyImageToBuffer(_getCurrCommandBufferHandle(), pSourceTexture->GetTextureHandle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, pDestBuffer->GetVulkanHandle(), 1, &regionsInfo);

		if (E_RESOURCE_LAYOUT::UNDEFINED != currSourceLayout)
		{
			result = result | pSourceTexture->Transition(currSourceLayout);
		}

		return RC_OK;
	}

	E_RESULT_CODE CVulkanGraphicsContext::CopyResource(TBufferHandleId sourceHandle, TBufferHandleId destHandle)
	{
		if (sourceHandle == destHandle) 
		{
			return RC_INVALID_ARGS;
		}

		TPtr<CVulkanBuffer> pSourceBuffer = mpGraphicsObjectManagerImpl->GetVulkanBufferPtr(sourceHandle);
		if (!pSourceBuffer)
		{
			return RC_INVALID_ARGS;
		}

		TPtr<CVulkanBuffer> pDestBuffer = mpGraphicsObjectManagerImpl->GetVulkanBufferPtr(destHandle);
		if (!pDestBuffer)
		{
			return RC_INVALID_ARGS;
		}

		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0;
		copyRegion.dstOffset = 0;
		copyRegion.size      = pSourceBuffer->GetSize();

		vkCmdCopyBuffer(_getCurrCommandBufferHandle(), pSourceBuffer->GetVulkanHandle(), pDestBuffer->GetVulkanHandle(), 1, &copyRegion);
		
		return RC_OK;
	}

	void CVulkanGraphicsContext::MemoryAccessBarrier(const std::variant<TBufferHandleId, TTextureHandleId> resourceHandle)
	{
#if 0
		struct TVisitor
		{
			CVulkanGraphicsContext* mpGraphicsContext = nullptr;

			void operator()(TBufferHandleId bufferHandle)
			{
				VkMemoryBarrier2 memoryBarrier{};
				memoryBarrier.sType         = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2;
				memoryBarrier.srcStageMask  = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
				memoryBarrier.dstStageMask  = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
				memoryBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
				memoryBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT;

				mpGraphicsContext->mMemoryBarriers[mpGraphicsContext->mCurrFrameIndex].push_back(memoryBarrier);
			}

			void operator()(TTextureHandleId textureHandle)
			{
				VkMemoryBarrier2 memoryBarrier{};
				memoryBarrier.sType         = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2;
				memoryBarrier.srcStageMask  = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
				memoryBarrier.dstStageMask  = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
				memoryBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
				memoryBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT;

				mpGraphicsContext->mMemoryBarriers[mpGraphicsContext->mCurrFrameIndex].push_back(memoryBarrier);
			}
		};

		std::visit(TVisitor{ this }, resourceHandle);
#endif

		VkMemoryBarrier2 memoryBarrier{};
		memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2;
		memoryBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		memoryBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		memoryBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
		memoryBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT;

		mMemoryBarriers[mCurrFrameIndex].push_back(memoryBarrier);
	}


	static VkPipelineStageFlagBits2 GetStageMaskBitsFromLayout(E_RESOURCE_LAYOUT layout)
	{
		VkPipelineStageFlags2 result = ((E_RESOURCE_LAYOUT::SHADER_RESOURCE & layout) == E_RESOURCE_LAYOUT::SHADER_RESOURCE) ? VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT : 0x0;

		result |= ((E_RESOURCE_LAYOUT::UAV_RESOURCE & layout) == E_RESOURCE_LAYOUT::UAV_RESOURCE) ? VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT : 0x0;
		result |= ((E_RESOURCE_LAYOUT::CONSTANT_BUFFER & layout) == E_RESOURCE_LAYOUT::CONSTANT_BUFFER) ? VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT : 0x0;
		result |= ((E_RESOURCE_LAYOUT::COPY_SRC & layout) == E_RESOURCE_LAYOUT::COPY_SRC) ? VK_PIPELINE_STAGE_2_TRANSFER_BIT : 0x0;
		result |= ((E_RESOURCE_LAYOUT::COPY_DEST & layout) == E_RESOURCE_LAYOUT::COPY_DEST) ? VK_PIPELINE_STAGE_2_TRANSFER_BIT : 0x0;
		result |= ((E_RESOURCE_LAYOUT::RENDER_TARGET & layout) == E_RESOURCE_LAYOUT::RENDER_TARGET) ? VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT : 0x0;
		result |= ((E_RESOURCE_LAYOUT::DEPTH_STENCIL & layout) == E_RESOURCE_LAYOUT::DEPTH_STENCIL) ? (VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT) : 0x0;
		result |= ((E_RESOURCE_LAYOUT::DEPTH_STENCIL_READONLY & layout) == E_RESOURCE_LAYOUT::DEPTH_STENCIL_READONLY) ? (VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT) : 0x0;
		result |= ((E_RESOURCE_LAYOUT::INDIRECT_ARGS_BUFFER & layout) == E_RESOURCE_LAYOUT::INDIRECT_ARGS_BUFFER) ? VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT : 0x0;

		return result;
	}


	static VkAccessFlags2 GetAccessMaskFromLayout(E_RESOURCE_LAYOUT layout)
	{
		VkAccessFlags2 flags = ((E_RESOURCE_LAYOUT::SHADER_RESOURCE & layout) == E_RESOURCE_LAYOUT::SHADER_RESOURCE) ? VK_ACCESS_2_SHADER_READ_BIT : 0x0;

		flags |= ((E_RESOURCE_LAYOUT::UAV_RESOURCE & layout) == E_RESOURCE_LAYOUT::UAV_RESOURCE) ? (VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT) : 0x0;
		flags |= ((E_RESOURCE_LAYOUT::CONSTANT_BUFFER & layout) == E_RESOURCE_LAYOUT::CONSTANT_BUFFER) ? VK_ACCESS_2_UNIFORM_READ_BIT : 0x0;
		flags |= ((E_RESOURCE_LAYOUT::COPY_SRC & layout) == E_RESOURCE_LAYOUT::COPY_SRC) ? VK_ACCESS_2_TRANSFER_READ_BIT : 0x0;
		flags |= ((E_RESOURCE_LAYOUT::COPY_DEST & layout) == E_RESOURCE_LAYOUT::COPY_DEST) ? VK_ACCESS_2_TRANSFER_WRITE_BIT : 0x0;
		flags |= ((E_RESOURCE_LAYOUT::RENDER_TARGET & layout) == E_RESOURCE_LAYOUT::RENDER_TARGET) ? (VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT) : 0x0;
		flags |= ((E_RESOURCE_LAYOUT::DEPTH_STENCIL & layout) == E_RESOURCE_LAYOUT::DEPTH_STENCIL) ? (VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT) : 0x0;
		flags |= ((E_RESOURCE_LAYOUT::DEPTH_STENCIL_READONLY & layout) == E_RESOURCE_LAYOUT::DEPTH_STENCIL_READONLY) ? VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT : 0x0;
		flags |= ((E_RESOURCE_LAYOUT::DEPTH_STENCIL_READONLY & layout) == E_RESOURCE_LAYOUT::DEPTH_STENCIL_READONLY) ? VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT : 0x0;
		flags |= ((E_RESOURCE_LAYOUT::INDIRECT_ARGS_BUFFER & layout) == E_RESOURCE_LAYOUT::INDIRECT_ARGS_BUFFER) ? VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT : 0x0;

		return flags;
	}


	void CVulkanGraphicsContext::TransitionBarrier(const TBufferTransitionBarrierInfo& barrierInfo)
	{
		TPtr<CVulkanBuffer> pBuffer = mpGraphicsObjectManagerImpl->GetVulkanBufferPtr(barrierInfo.mHandle);
		if (!pBuffer)
		{
			return;
		}

		VkBufferMemoryBarrier2 bufferMemoryBarrier{};
		bufferMemoryBarrier.sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
		bufferMemoryBarrier.buffer              = pBuffer->GetVulkanHandle();
		bufferMemoryBarrier.srcStageMask        = GetStageMaskBitsFromLayout(barrierInfo.mCurrLayout);
		bufferMemoryBarrier.dstStageMask        = GetStageMaskBitsFromLayout(barrierInfo.mNewLayout);
		bufferMemoryBarrier.srcAccessMask       = GetAccessMaskFromLayout(barrierInfo.mCurrLayout);
		bufferMemoryBarrier.dstAccessMask       = GetAccessMaskFromLayout(barrierInfo.mNewLayout);
		bufferMemoryBarrier.offset              = 0;
		bufferMemoryBarrier.size                = pBuffer->GetSize();
		bufferMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		bufferMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		mBufferBarriers[mCurrFrameIndex].emplace_back(bufferMemoryBarrier);
	}

	void CVulkanGraphicsContext::TransitionBarrier(const TTextureTransitionBarrierInfo& barrierInfo)
	{
		TPtr<CVulkanTextureImpl> pTextureImpl = mpGraphicsObjectManagerImpl->GetVulkanTexturePtr(barrierInfo.mHandle);
		if (!pTextureImpl)
		{
			return;
		}

		VkImageMemoryBarrier2 imageMemoryBarrier{};
		imageMemoryBarrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
		imageMemoryBarrier.image                           = pTextureImpl->GetTextureHandle();
		imageMemoryBarrier.srcStageMask                    = GetStageMaskBitsFromLayout(barrierInfo.mCurrLayout);
		imageMemoryBarrier.dstStageMask                    = GetStageMaskBitsFromLayout(barrierInfo.mNewLayout);
		imageMemoryBarrier.srcAccessMask                   = GetAccessMaskFromLayout(barrierInfo.mCurrLayout);
		imageMemoryBarrier.dstAccessMask                   = GetAccessMaskFromLayout(barrierInfo.mNewLayout);
		imageMemoryBarrier.oldLayout                       = CVulkanMappings::GetResourceLayout(barrierInfo.mCurrLayout);
		imageMemoryBarrier.newLayout                       = CVulkanMappings::GetResourceLayout(barrierInfo.mNewLayout);
		imageMemoryBarrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.subresourceRange.baseMipLevel   = 0; // \todo
		imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
		imageMemoryBarrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
		imageMemoryBarrier.subresourceRange.layerCount     = VK_REMAINING_ARRAY_LAYERS;
		imageMemoryBarrier.subresourceRange.levelCount     = VK_REMAINING_MIP_LEVELS;

		if (E_FORMAT_TYPE::FT_D32 == pTextureImpl->GetParams().mFormat)
		{
			imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		}

		mTextureBarriers[mCurrFrameIndex].emplace_back(imageMemoryBarrier);
	}

	E_RESULT_CODE CVulkanGraphicsContext::CopyCount(TBufferHandleId sourceHandle, TBufferHandleId destHandle, U32 offset)
	{
		return RC_NOT_IMPLEMENTED_YET;
	}

	E_RESULT_CODE CVulkanGraphicsContext::GenerateMipMaps(TTextureHandleId textureHandle)
	{
		TPtr<CVulkanTextureImpl> pTextureImpl = mpGraphicsObjectManagerImpl->GetVulkanTexturePtr(textureHandle);
		if (!pTextureImpl)
		{
			return RC_INVALID_ARGS;
		}

		const TInitTextureParams& textureParams = pTextureImpl->GetParams();
		if (textureParams.mNumOfMipLevels <= 1)
		{
			return RC_OK;
		}

		BeginSectionMarker("GenerateMipMaps");
		defer([this] { EndSectionMarker(); });

		const U32 maxMipsCount = std::min(textureParams.mNumOfMipLevels, static_cast<U32>(floor(log2(std::max(textureParams.mWidth, textureParams.mHeight))) + 1));
		const E_RESOURCE_LAYOUT originalLayout = pTextureImpl->GetLayout();

		I32 mipWidth  = textureParams.mWidth;
		I32 mipHeight = textureParams.mHeight;

		const VkImage currImageHandle = pTextureImpl->GetTextureHandle();

		TTextureTransitionBarrierInfo barrierInfo{};
		barrierInfo.mCurrLayout = E_RESOURCE_LAYOUT::UNDEFINED;
		barrierInfo.mNewLayout  = E_RESOURCE_LAYOUT::COPY_SRC;
		barrierInfo.mHandle     = textureHandle;
		barrierInfo.mMipLevel   = 0;

		TransitionBarrier(barrierInfo);
		FlushBarriers();

		for (I32 i = 1; i < static_cast<I32>(maxMipsCount); ++i)
		{
			VkImageBlit imageBlit{};

			// Source
			imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageBlit.srcSubresource.layerCount = 1;
			imageBlit.srcSubresource.mipLevel   = i - 1;
			imageBlit.srcOffsets[1].x           = static_cast<I32>(textureParams.mWidth >> (i - 1));
			imageBlit.srcOffsets[1].y           = static_cast<I32>(textureParams.mHeight >> (i - 1));
			imageBlit.srcOffsets[1].z           = 1;

			// Destination
			imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageBlit.dstSubresource.layerCount = 1;
			imageBlit.dstSubresource.mipLevel   = i;
			imageBlit.dstOffsets[1].x           = static_cast<I32>(textureParams.mWidth >> i);
			imageBlit.dstOffsets[1].y           = static_cast<I32>(textureParams.mHeight >> i);
			imageBlit.dstOffsets[1].z           = 1;

			barrierInfo.mMipLevel = static_cast<U32>(i);
			barrierInfo.mCurrLayout = E_RESOURCE_LAYOUT::UNDEFINED;
			barrierInfo.mNewLayout = E_RESOURCE_LAYOUT::COPY_DEST;

			TransitionBarrier(barrierInfo);
			FlushBarriers();

			vkCmdBlitImage(
				_getCurrCommandBufferHandle(),
				currImageHandle,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				currImageHandle,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&imageBlit,
				VK_FILTER_LINEAR);

			barrierInfo.mCurrLayout = E_RESOURCE_LAYOUT::COPY_DEST;
			barrierInfo.mNewLayout = E_RESOURCE_LAYOUT::COPY_SRC;

			TransitionBarrier(barrierInfo);
			FlushBarriers();

			mipWidth  = (mipWidth > 1) ? mipWidth >> 1 : 1;
			mipHeight = (mipHeight > 1) ? mipHeight >> 1 : 1;
		}

		barrierInfo.mCurrLayout = E_RESOURCE_LAYOUT::COPY_SRC;
		barrierInfo.mNewLayout  = originalLayout;
		barrierInfo.mMipLevel   = std::nullopt;

		TransitionBarrier(barrierInfo);
		FlushBarriers();

		return RC_OK;
	}

	void CVulkanGraphicsContext::Draw(E_PRIMITIVE_TOPOLOGY_TYPE topology, U32 startVertex, U32 numOfVertices)
	{
		_flushPipelineDescriptorsSet();

		vkCmdSetPrimitiveTopology(_getCurrCommandBufferHandle(), CVulkanMappings::GetPrimitiveTopology(topology));
		vkCmdDraw(_getCurrCommandBufferHandle(), numOfVertices, 1, startVertex, 0);
	}

	void CVulkanGraphicsContext::DrawIndexed(E_PRIMITIVE_TOPOLOGY_TYPE topology, E_INDEX_FORMAT_TYPE indexFormatType, U32 baseVertex, U32 startIndex, U32 numOfIndices)
	{
		_flushPipelineDescriptorsSet();

		vkCmdSetPrimitiveTopology(_getCurrCommandBufferHandle(), CVulkanMappings::GetPrimitiveTopology(topology));
		vkCmdDrawIndexed(_getCurrCommandBufferHandle(), numOfIndices, 1, startIndex, baseVertex, 0);
	}

	void CVulkanGraphicsContext::DrawInstanced(E_PRIMITIVE_TOPOLOGY_TYPE topology, U32 startVertex, U32 verticesPerInstance, U32 startInstance, U32 numOfInstances)
	{
		_flushPipelineDescriptorsSet();

		vkCmdSetPrimitiveTopology(_getCurrCommandBufferHandle(), CVulkanMappings::GetPrimitiveTopology(topology));
		vkCmdDraw(_getCurrCommandBufferHandle(), verticesPerInstance, numOfInstances, startVertex, startInstance);
	}

	void CVulkanGraphicsContext::DrawIndexedInstanced(E_PRIMITIVE_TOPOLOGY_TYPE topology, E_INDEX_FORMAT_TYPE indexFormatType, U32 baseVertex, U32 startIndex,
		U32 startInstance, U32 indicesPerInstance, U32 numOfInstances)
	{
		_flushPipelineDescriptorsSet();

		vkCmdSetPrimitiveTopology(_getCurrCommandBufferHandle(), CVulkanMappings::GetPrimitiveTopology(topology));
		vkCmdDrawIndexed(_getCurrCommandBufferHandle(), indicesPerInstance, numOfInstances, startIndex, baseVertex, startInstance);
	}

	void CVulkanGraphicsContext::DrawIndirectInstanced(E_PRIMITIVE_TOPOLOGY_TYPE topology, TBufferHandleId argsBufferHandle, U32 alignedOffset)
	{
		_flushPipelineDescriptorsSet();

		vkCmdSetPrimitiveTopology(_getCurrCommandBufferHandle(), CVulkanMappings::GetPrimitiveTopology(topology));
		vkCmdDrawIndexedIndirect(_getCurrCommandBufferHandle(), mpGraphicsObjectManagerImpl->GetVulkanBufferPtr(argsBufferHandle)->GetVulkanHandle(), static_cast<VkDeviceSize>(alignedOffset), 1, 0);
	}

	void CVulkanGraphicsContext::DrawIndirectIndexedInstanced(E_PRIMITIVE_TOPOLOGY_TYPE topology, E_INDEX_FORMAT_TYPE indexFormatType, TBufferHandleId argsBufferHandle, U32 alignedOffset)
	{
		_flushPipelineDescriptorsSet();

		vkCmdSetPrimitiveTopology(_getCurrCommandBufferHandle(), CVulkanMappings::GetPrimitiveTopology(topology));
		TDE2_UNIMPLEMENTED();
	}

	void CVulkanGraphicsContext::DispatchCompute(U32 groupsCountX, U32 groupsCountY, U32 groupsCountZ)
	{
		_flushPipelineDescriptorsSet();
		vkCmdDispatch(_getCurrCommandBufferHandle(), groupsCountX, groupsCountY, groupsCountZ);
	}

	void CVulkanGraphicsContext::DispatchIndirectCompute(TBufferHandleId argsBufferHandle, U32 alignedOffset)
	{
		_flushPipelineDescriptorsSet();
	}

	E_RESULT_CODE CVulkanGraphicsContext::BindPipelineState(CVulkanBasePipeline* pPipeline)
	{
		const bool isGraphicsPipeline = E_PIPELINE_TYPE::GRAPHICS == pPipeline->GetType();

		if (isGraphicsPipeline)
		{
			TDE2_ASSERT(mIsRenderPassActive);
		}

		mpActivePipelineStates[mCurrFrameIndex] = pPipeline;

		const U64 pipelineHash = isGraphicsPipeline ? (static_cast<U64>(ComputeStateDescHash(mCurrRenderPassInfo)) << 32) | pPipeline->GetHash() : pPipeline->GetHash();

		VkPipeline currPipelineHandle = VK_NULL_HANDLE;

		if (isGraphicsPipeline)
		{
			auto&& it = mCachedPipelinesLibrary.find(pipelineHash);
			if (it == mCachedPipelinesLibrary.cend())
			{
				currPipelineHandle = dynamic_cast<CVulkanGraphicsPipeline*>(pPipeline)->GetPipelineForRenderPass(mCurrRenderPassInfo);
				mCachedPipelinesLibrary[pipelineHash] = currPipelineHandle;
			}
			else
			{
				currPipelineHandle = it->second;
			}
		}
		else
		{
			currPipelineHandle = pPipeline->GetBasePipelineHandle();
		}
		
		vkCmdBindPipeline(_getCurrCommandBufferHandle(), isGraphicsPipeline ? VK_PIPELINE_BIND_POINT_GRAPHICS : VK_PIPELINE_BIND_POINT_COMPUTE, currPipelineHandle);

		return RC_OK;
	}

	void CVulkanGraphicsContext::BindBlendState(TBlendStateId blendStateId)
	{
	}

	void CVulkanGraphicsContext::BindDepthStencilState(TDepthStencilStateId depthStencilStateId, U8 stencilRef)
	{
	}

	void CVulkanGraphicsContext::BindRasterizerState(TRasterizerStateId rasterizerStateId)
	{
	}

	E_RESULT_CODE CVulkanGraphicsContext::BeginRenderPass(const TFramebufferInfo& framebufferInfo)
	{
		mCurrRenderPassInfo = TRenderPassInfo(mpGraphicsObjectManager.Get(), framebufferInfo);

		std::array<VkRenderingAttachmentInfo, RENDER_TARGETS_MAX_COUNT> colorAttachmentInfos;
		VkRenderingAttachmentInfo depthStencilAttachmentInfo {};

		VkExtent2D viewportSizes{};
		
		U32 layersCount = 1;

		for (USIZE i = 0; i < framebufferInfo.mAttachments.size(); ++i)
		{
			const TFramebufferInfo::TAttachment& currAttachment = framebufferInfo.mAttachments[i];

			TPtr<CVulkanTextureImpl> pRenderTargetTexture = mpGraphicsObjectManagerImpl->GetVulkanTexturePtr(currAttachment.mTargetHandle);
			if (!pRenderTargetTexture) // \note The special corner case when there is no render target attached except the back buffer
			{
				mCurrRenderPassInfo.mRenderTargetFormats[0] = mpSwapchain->GetBackBuffersFormat();

				const TRectU32& windowRect = mpWindowSystem->GetClientRect();
				viewportSizes.width  = windowRect.width;
				viewportSizes.height = windowRect.height;

				const bool hasClearValue = currAttachment.mClearValue.has_value();
				VkClearValue clearValue{};

				if (hasClearValue)
				{
					const auto& targetClearValueVariant = currAttachment.mClearValue.value();
					memcpy(clearValue.color.float32, &std::get<TColor32F>(targetClearValueVariant), sizeof(TColor32F));
				}

				VkRenderingAttachmentInfo currVkAttachmentInfo{};
				currVkAttachmentInfo.sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
				currVkAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				currVkAttachmentInfo.imageView   = mpSwapchain->GetCurrImageView();
				currVkAttachmentInfo.resolveMode = VK_RESOLVE_MODE_NONE; // \note For now don't use MSAA at all
				currVkAttachmentInfo.loadOp      = hasClearValue ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
				currVkAttachmentInfo.storeOp     = VK_ATTACHMENT_STORE_OP_STORE;
				currVkAttachmentInfo.clearValue  = clearValue;

				colorAttachmentInfos[i] = currVkAttachmentInfo;

				// \note Add barrier for current swapchain's image
				VkImageMemoryBarrier2 barrier{};
				barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
				barrier.image                           = mpSwapchain->GetCurrImage();
				barrier.oldLayout                       = VK_IMAGE_LAYOUT_UNDEFINED;
				barrier.newLayout                       = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				barrier.srcStageMask                    = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
				barrier.srcAccessMask                   = VK_ACCESS_2_MEMORY_WRITE_BIT;
				barrier.dstStageMask                    = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
				barrier.dstAccessMask                   = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
				barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
				barrier.subresourceRange.baseMipLevel   = 0;
				barrier.subresourceRange.levelCount     = VK_REMAINING_MIP_LEVELS;
				barrier.subresourceRange.baseArrayLayer = 0;
				barrier.subresourceRange.layerCount     = VK_REMAINING_ARRAY_LAYERS;
				barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
				barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;

				VkDependencyInfo dependencyInfo{};
				dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
				dependencyInfo.imageMemoryBarrierCount = 1;
				dependencyInfo.pImageMemoryBarriers = &barrier;
				vkCmdPipelineBarrier2(_getCurrCommandBufferHandle(), &dependencyInfo);

				continue;
			}

			if (E_BIND_GRAPHICS_TYPE::BIND_RENDER_TARGET != (pRenderTargetTexture->GetParams().mBindFlags & E_BIND_GRAPHICS_TYPE::BIND_RENDER_TARGET))
			{
				TDE2_ASSERT_MSG(false, "[CVulkanGraphicsContext] Try to use the texture that is not a render target as a color attachment");
				return RC_FAIL;
			}

			viewportSizes.width  = std::max<U32>(viewportSizes.width, pRenderTargetTexture->GetParams().mWidth);
			viewportSizes.height = std::max<U32>(viewportSizes.height, pRenderTargetTexture->GetParams().mHeight);

			const bool hasClearValue = currAttachment.mClearValue.has_value();
			VkClearValue clearValue{};

			if (hasClearValue)
			{
				const auto& targetClearValueVariant = currAttachment.mClearValue.value();
				memcpy(clearValue.color.float32, &std::get<TColor32F>(targetClearValueVariant), sizeof(TColor32F));
			}

			VkRenderingAttachmentInfo currVkAttachmentInfo{};
			currVkAttachmentInfo.sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			currVkAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			currVkAttachmentInfo.imageView   = pRenderTargetTexture->GetTextureViewHandle();
			currVkAttachmentInfo.resolveMode = VK_RESOLVE_MODE_NONE; // \note For now don't use MSAA at all
			currVkAttachmentInfo.loadOp      = hasClearValue ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
			currVkAttachmentInfo.storeOp     = VK_ATTACHMENT_STORE_OP_STORE;
			currVkAttachmentInfo.clearValue  = clearValue;

			colorAttachmentInfos[i] = currVkAttachmentInfo;

			pRenderTargetTexture->Transition(E_RESOURCE_LAYOUT::RENDER_TARGET);
		}

		if (framebufferInfo.mDepthStencilAttachment)
		{
			const auto& depthStencilAttachment = framebufferInfo.mDepthStencilAttachment.value();

			TPtr<CVulkanTextureImpl> pDepthBufferTexture = mpGraphicsObjectManagerImpl->GetVulkanTexturePtr(depthStencilAttachment.mTargetHandle);

			if (E_BIND_GRAPHICS_TYPE::BIND_DEPTH_BUFFER != (pDepthBufferTexture->GetParams().mBindFlags & E_BIND_GRAPHICS_TYPE::BIND_DEPTH_BUFFER))
			{
				TDE2_ASSERT_MSG(false, "[CVulkanGraphicsContext] Try to bind the texture that is not a depth buffer as a depth-stencil attachment");
				return RC_FAIL;
			}

			if (framebufferInfo.mAttachments.empty())
			{
				viewportSizes.width = std::max<U32>(viewportSizes.width, pDepthBufferTexture->GetParams().mWidth);
				viewportSizes.height = std::max<U32>(viewportSizes.height, pDepthBufferTexture->GetParams().mHeight);
			}

			const bool hasDepthClearValue   = depthStencilAttachment.mDepthClearValue.has_value();
			const bool hasStencilClearValue = depthStencilAttachment.mStencilClearValue.has_value();
			
			VkClearValue clearValue;
			clearValue.depthStencil.depth   = 1.0f;
			clearValue.depthStencil.stencil = 0xff;

			if (hasDepthClearValue)
			{
				const auto& targetClearValueVariant = depthStencilAttachment.mDepthClearValue.value();
				clearValue.depthStencil.depth = std::get<F32>(targetClearValueVariant);
			}

			if (hasStencilClearValue)
			{
				const auto& targetClearValueVariant = depthStencilAttachment.mStencilClearValue.value();
				clearValue.depthStencil.stencil = std::get<U8>(targetClearValueVariant);
			}

			if (pDepthBufferTexture)
			{
				depthStencilAttachmentInfo.sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
				depthStencilAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL; // VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
				depthStencilAttachmentInfo.imageView   = pDepthBufferTexture->GetTextureViewHandle();
				depthStencilAttachmentInfo.resolveMode = VK_RESOLVE_MODE_NONE; // \note For now don't use MSAA at all
				depthStencilAttachmentInfo.loadOp      = (hasDepthClearValue || hasStencilClearValue) ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
				depthStencilAttachmentInfo.storeOp     = VK_ATTACHMENT_STORE_OP_STORE;
				depthStencilAttachmentInfo.clearValue  = clearValue;

				pDepthBufferTexture->Transition(E_RESOURCE_LAYOUT::DEPTH_STENCIL);

				layersCount = pDepthBufferTexture->GetParams().mArraySize;
			}
		}

		VkRenderingInfo renderPassInfo{};
		renderPassInfo.sType                = VK_STRUCTURE_TYPE_RENDERING_INFO;
		renderPassInfo.colorAttachmentCount = static_cast<U32>(framebufferInfo.mAttachments.size());
		renderPassInfo.layerCount           = layersCount;
		renderPassInfo.pColorAttachments    = colorAttachmentInfos.data();
		renderPassInfo.renderArea           = { {}, viewportSizes };
		renderPassInfo.pDepthAttachment     = framebufferInfo.mDepthStencilAttachment.has_value() ? &depthStencilAttachmentInfo : nullptr;
		renderPassInfo.pStencilAttachment   = VK_NULL_HANDLE;
		//renderPassInfo.pStencilAttachment   = framebufferInfo.mDepthStencilAttachment.has_value() ? &depthStencilAttachmentInfo : nullptr; // \todo Add support of stencil buffer

		FlushBarriers();

		vkCmdBeginRendering(_getCurrCommandBufferHandle(), &renderPassInfo);
		mIsRenderPassActive = true;

		SetScissorRect({ 0, 0, viewportSizes.width, viewportSizes.height }); // \note Set up default scissor rect for the whole screen

		return RC_OK;
	}

	E_RESULT_CODE CVulkanGraphicsContext::EndRenderPass()
	{
		vkCmdEndRendering(_getCurrCommandBufferHandle());
		mIsRenderPassActive = false;

		return RC_OK;
	}

	void CVulkanGraphicsContext::FlushBarriers()
	{
		auto& memoryBarriers  = mMemoryBarriers[mCurrFrameIndex];
		auto& textureBarriers = mTextureBarriers[mCurrFrameIndex];
		auto& bufferBarriers  = mBufferBarriers[mCurrFrameIndex];

		if (!memoryBarriers.empty() || !textureBarriers.empty() || !bufferBarriers.empty())
		{
			VkDependencyInfo dependencyInfo{};
			dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
			dependencyInfo.memoryBarrierCount       = static_cast<U32>(memoryBarriers.size());
			dependencyInfo.pMemoryBarriers          = memoryBarriers.data();
			dependencyInfo.imageMemoryBarrierCount  = static_cast<U32>(textureBarriers.size());
			dependencyInfo.pImageMemoryBarriers     = textureBarriers.data();
			dependencyInfo.bufferMemoryBarrierCount = static_cast<U32>(bufferBarriers.size());
			dependencyInfo.pBufferMemoryBarriers    = bufferBarriers.data();

			vkCmdPipelineBarrier2(_getCurrCommandBufferHandle(), &dependencyInfo);

			memoryBarriers.clear();
			textureBarriers.clear();
			bufferBarriers.clear();
		}
	}

	const TGraphicsCtxInternalData& CVulkanGraphicsContext::GetInternalData() const
	{
		return mInternalDataObject;
	}

	E_ENGINE_SUBSYSTEM_TYPE CVulkanGraphicsContext::GetType() const
	{
		return EST_GRAPHICS_CONTEXT;
	}

	IGraphicsObjectManager* CVulkanGraphicsContext::GetGraphicsObjectManager() const
	{
		return mpGraphicsObjectManager.Get();
	}

	TPtr<IWindowSystem> CVulkanGraphicsContext::GetWindowSystem() const
	{
		return mpWindowSystem;
	}

	std::vector<U8> CVulkanGraphicsContext::GetBackBufferData() const
	{
		TDE2_UNIMPLEMENTED();
		return {};
	}

	F32 CVulkanGraphicsContext::GetPositiveZAxisDirection() const
	{
		return 1.0f;
	}

	E_RESULT_CODE CVulkanGraphicsContext::OnEvent(const TBaseEvent* pEvent)
	{
		if (pEvent->GetEventType() != TOnWindowResized::GetTypeId())
		{
			return RC_OK;
		}

		const TOnWindowResized* pOnWindowResizedEvent = dynamic_cast<const TOnWindowResized*>(pEvent);

		mpSwapchain->InvalidateState();
		mpSwapchain->TryProcessInvalidateState();

		return RC_OK;
	}

#if TDE2_DEBUG_MODE

	void CVulkanGraphicsContext::BeginSectionMarker(const std::string& id)
	{
		VkDebugUtilsLabelEXT markerInfo = {};
		markerInfo.sType      = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
		markerInfo.pLabelName = id.c_str();

		vkCmdBeginDebugUtilsLabelEXT(_getCurrCommandBufferHandle(), &markerInfo);
	}

	void CVulkanGraphicsContext::EndSectionMarker()
	{
		vkCmdEndDebugUtilsLabelEXT(_getCurrCommandBufferHandle());
	}

#endif

	TEventListenerId CVulkanGraphicsContext::GetListenerId() const
	{
		return TEventListenerId(GetTypeId());
	}

	TVideoAdapterInfo CVulkanGraphicsContext::GetInfo() const
	{
		/*VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(mPhysicalDevice, &properties);*/

		TVideoAdapterInfo outputInfo{};
		outputInfo.mAvailableVideoMemory = 0;


		return outputInfo;
	}

	const TGraphicsContextInfo& CVulkanGraphicsContext::GetContextInfo() const
	{
		const static TGraphicsContextInfo infoData
		{
			{ { -1.0f, -1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f } },
			false,
			E_GRAPHICS_CONTEXT_GAPI_TYPE::GCGT_VULKAN
		};

		return infoData;
	}

	VkDevice CVulkanGraphicsContext::GetDevice()
	{
		return mpVulkanDeviceContext->GetDevice();
	}

	VkPhysicalDevice CVulkanGraphicsContext::GetPhysicalDevice()
	{
		return mpVulkanDeviceContext->GetPhysicalDevice();
	}

	VkInstance CVulkanGraphicsContext::GetInstance()
	{
		return mpVulkanDeviceContext->GetInstance();
	}

	VmaAllocator CVulkanGraphicsContext::GetAllocator()
	{
		return mpVulkanDeviceContext->GetMemoryAllocator();
	}

	E_RESULT_CODE CVulkanGraphicsContext::_prepareFrameData()
	{
		E_RESULT_CODE result = RC_OK;

		for (USIZE i = 0; i < FRAMES_COUNT; ++i)
		{
			E_RESULT_CODE localResult = RC_OK;
			
			mpCommandBuffers[i] = TPtr<CVulkanCommandBuffer>(CreateCommandBuffer(mpVulkanDeviceContext.Get(), localResult));
			result = result | localResult;
		}

		if (RC_OK != result)
		{
			return result;
		}

		for (USIZE i = 0; i < FRAMES_COUNT; i++)
		{
			mImageReadySemaphores[i] = mpVulkanDeviceContext->CreateSemaphore();
			mRenderFinishedSemaphores[i] = mpVulkanDeviceContext->CreateSemaphore();
		}

		return RC_OK;
	}

	VkCommandBuffer CVulkanGraphicsContext::_getCurrCommandBufferHandle() const
	{
		return mpCommandBuffers[mCurrFrameIndex]->GetHandle();
	}

	void CVulkanGraphicsContext::_flushPipelineDescriptorsSet()
	{
		if (!mpActivePipelineStates[mCurrFrameIndex])
		{
			TDE2_ASSERT(false);
			return;
		}

		const auto pipelineType = E_PIPELINE_TYPE::GRAPHICS == mpActivePipelineStates[mCurrFrameIndex]->GetType() ? 
			VK_PIPELINE_BIND_POINT_GRAPHICS : VK_PIPELINE_BIND_POINT_COMPUTE;

		if (pipelineType == VK_PIPELINE_BIND_POINT_GRAPHICS)
		{
			TDE2_ASSERT(mIsRenderPassActive);
		}

		mDescriptorWrites.clear();
		mDescriptorBufferInfos.clear();
		mDescriptorImageInfos.clear();

		const auto& currPipelineActiveSlots = mpActivePipelineStates[mCurrFrameIndex]->GetLayoutInfo();

		for (U32 i = 0; i < currPipelineActiveSlots.mCBVActiveSlots.size(); ++i)
		{
			const TBufferHandleId currBufferHandle = mDescriptorsBindingsTable.mConstantBuffers[currPipelineActiveSlots.mCBVActiveSlots[i]];
			if (TBufferHandleId::Invalid == currBufferHandle)
			{
				continue;
			}

			auto pBuffer = mpGraphicsObjectManagerImpl->GetVulkanBufferPtr(currBufferHandle);
			if (!pBuffer)
			{
				continue;
			}

			VkDescriptorBufferInfo& currBufferInfo = mDescriptorBufferInfos.emplace_back(VkDescriptorBufferInfo { pBuffer->GetVulkanHandle(), 0, VK_WHOLE_SIZE });

			VkWriteDescriptorSet currWriteDescriptorSet{};

			currWriteDescriptorSet.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			currWriteDescriptorSet.dstSet          = 0;
			currWriteDescriptorSet.descriptorCount = 1;
			currWriteDescriptorSet.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			currWriteDescriptorSet.dstBinding      = currPipelineActiveSlots.mCBVActiveSlots[i];
			currWriteDescriptorSet.pBufferInfo     = &currBufferInfo;
			
			mDescriptorWrites.emplace_back(currWriteDescriptorSet);
		}

		for (U32 i = 0; i < currPipelineActiveSlots.mSRVActiveSlots.size(); ++i)
		{
			const U32 currBinding = currPipelineActiveSlots.mSRVActiveSlots[i].mSlot;
			const auto& currResourceEntity = mDescriptorsBindingsTable.mSRVBuffers[currBinding];

			const bool isNullDescriptor = TDescriptorsBindingsTable::E_DESCRIPTOR_TYPE::UNKNOWN == currResourceEntity.mType;

			if (TDescriptorsBindingsTable::E_DESCRIPTOR_TYPE::UNKNOWN == currResourceEntity.mType)
			{
				/*mDescriptorsBindingsTable.mSRVBuffers[currBinding].mType =
					TVulkanPipelineLayoutInfo::TBindingInfo::E_TYPE::BUFFER == currPipelineActiveSlots.mSRVActiveSlots[i].mType ? TDescriptorsBindingsTable::E_DESCRIPTOR_TYPE::BUFFER : TDescriptorsBindingsTable::E_DESCRIPTOR_TYPE::TEXTURE;*/

				VkWriteDescriptorSet& currWriteDescriptorSet = mDescriptorWrites.emplace_back();

				switch (currPipelineActiveSlots.mSRVActiveSlots[i].mType)
				{
					case TVulkanPipelineLayoutInfo::TBindingInfo::E_TYPE::BUFFER:
					{
						VkDescriptorBufferInfo& currBufferInfo = mDescriptorBufferInfos.emplace_back();
						currBufferInfo.buffer = IS_ROBUSTNESS2_EXTENSION_SUPPORTED ? VK_NULL_HANDLE : mpGraphicsObjectManagerImpl->GetVulkanBufferPtr(mEmptyStructuredBufferHandle)->GetVulkanHandle();
						currBufferInfo.offset = 0;
						currBufferInfo.range  = VK_WHOLE_SIZE;

						currWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
						currWriteDescriptorSet.pBufferInfo = &currBufferInfo;
					}
						break;
					case TVulkanPipelineLayoutInfo::TBindingInfo::E_TYPE::TEXTURE:
					{
						VkDescriptorImageInfo& currImageInfo = mDescriptorImageInfos.emplace_back();
						currImageInfo.imageView   = IS_ROBUSTNESS2_EXTENSION_SUPPORTED ? VK_NULL_HANDLE : mpGraphicsObjectManagerImpl->GetVulkanTexturePtr(mEmptyTextureHandle)->GetTextureViewHandle();
						currImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
						currImageInfo.sampler     = VK_NULL_HANDLE;

						currWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
						currWriteDescriptorSet.pImageInfo = &currImageInfo;
					}
						break;
				}

				currWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				currWriteDescriptorSet.dstSet = 0;
				currWriteDescriptorSet.descriptorCount = 1;
				currWriteDescriptorSet.dstBinding = GetBindingOffsetByResourceType(E_SHADER_RESOURCE_TYPE::SRT_TEXTURE2D) + currBinding;

				continue;
			}

			VkWriteDescriptorSet& currWriteDescriptorSet = mDescriptorWrites.emplace_back();

			switch (mDescriptorsBindingsTable.mSRVBuffers[currBinding].mType)
			{
				case TDescriptorsBindingsTable::E_DESCRIPTOR_TYPE::BUFFER:
					{
						auto pBuffer = mpGraphicsObjectManagerImpl->GetVulkanBufferPtr(currResourceEntity.mValue.mBuffer);
						if (!isNullDescriptor && !pBuffer)
						{
							mDescriptorWrites.erase(mDescriptorWrites.cend() - 1);
							continue;
						}

						VkDescriptorBufferInfo& currBufferInfo = mDescriptorBufferInfos.emplace_back();
						currBufferInfo.buffer = isNullDescriptor ? VK_NULL_HANDLE : pBuffer->GetVulkanHandle();
						currBufferInfo.offset = 0;
						currBufferInfo.range  = VK_WHOLE_SIZE;

						currWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
						currWriteDescriptorSet.pBufferInfo    = &currBufferInfo;
					}
					break;

				case TDescriptorsBindingsTable::E_DESCRIPTOR_TYPE::TEXTURE:
					{
						auto pTexture = mpGraphicsObjectManagerImpl->GetVulkanTexturePtr(currResourceEntity.mValue.mTexture);
						if (!isNullDescriptor && !pTexture)
						{
							mDescriptorWrites.erase(mDescriptorWrites.cend() - 1);
							continue;
						}

						VkDescriptorImageInfo& currImageInfo = mDescriptorImageInfos.emplace_back();
						currImageInfo.imageView   = isNullDescriptor ? VK_NULL_HANDLE : pTexture->GetTextureViewHandle();
						currImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
						currImageInfo.sampler     = VK_NULL_HANDLE;

						currWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
						currWriteDescriptorSet.pImageInfo = &currImageInfo;
					}
					break;
			}

			currWriteDescriptorSet.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			currWriteDescriptorSet.dstSet          = 0;
			currWriteDescriptorSet.descriptorCount = 1;
			currWriteDescriptorSet.dstBinding      = GetBindingOffsetByResourceType(E_SHADER_RESOURCE_TYPE::SRT_TEXTURE2D) + currBinding;
		}

		for (U32 i = 0; i < currPipelineActiveSlots.mSamplersActiveSlots.size(); ++i)
		{
			const TTextureSamplerId currSamplerHandle = mDescriptorsBindingsTable.mSamplers[currPipelineActiveSlots.mSamplersActiveSlots[i]];
			if (TTextureSamplerId::Invalid == currSamplerHandle)
			{
				continue;
			}

			VkWriteDescriptorSet& currWriteDescriptorSet = mDescriptorWrites.emplace_back();

			VkDescriptorImageInfo& currImageInfo = mDescriptorImageInfos.emplace_back();
			currImageInfo.sampler     = mpGraphicsObjectManagerImpl->GetTextureSampler(currSamplerHandle).Get();
			currImageInfo.imageView   = VK_NULL_HANDLE;
			currImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			currWriteDescriptorSet.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			currWriteDescriptorSet.dstSet          = 0;
			currWriteDescriptorSet.descriptorCount = 1;
			currWriteDescriptorSet.descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLER;
			currWriteDescriptorSet.dstBinding      = GetBindingOffsetByResourceType(E_SHADER_RESOURCE_TYPE::SRT_SAMPLER_STATE) + currPipelineActiveSlots.mSamplersActiveSlots[i];
			currWriteDescriptorSet.pImageInfo      = &currImageInfo;
		}

		for (U32 i = 0; i < currPipelineActiveSlots.mUAVActiveSlots.size(); ++i)
		{
			const U32 currBinding = currPipelineActiveSlots.mUAVActiveSlots[i].mSlot;
			const auto& currResourceEntity = mDescriptorsBindingsTable.mUAVBuffers[currBinding];

			if (TDescriptorsBindingsTable::E_DESCRIPTOR_TYPE::UNKNOWN == currResourceEntity.mType)
			{
				continue;
			}

			VkWriteDescriptorSet& currWriteDescriptorSet = mDescriptorWrites.emplace_back();

			switch (mDescriptorsBindingsTable.mUAVBuffers[currBinding].mType)
			{
				case TDescriptorsBindingsTable::E_DESCRIPTOR_TYPE::BUFFER:
				{
					auto pBuffer = mpGraphicsObjectManagerImpl->GetVulkanBufferPtr(currResourceEntity.mValue.mBuffer);
					if (!pBuffer)
					{
						mDescriptorWrites.erase(mDescriptorWrites.cend() - 1);
						continue;
					}

					VkDescriptorBufferInfo& currBufferInfo = mDescriptorBufferInfos.emplace_back();
					currBufferInfo.buffer = pBuffer->GetVulkanHandle();
					currBufferInfo.offset = 0;
					currBufferInfo.range = VK_WHOLE_SIZE;

					currWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
					currWriteDescriptorSet.pBufferInfo = &currBufferInfo;
				}
				break;

				case TDescriptorsBindingsTable::E_DESCRIPTOR_TYPE::TEXTURE:
				{
					auto pTexture = mpGraphicsObjectManagerImpl->GetVulkanTexturePtr(currResourceEntity.mValue.mTexture);
					if (!pTexture)
					{
						mDescriptorWrites.erase(mDescriptorWrites.cend() - 1);
						continue;
					}

					VkDescriptorImageInfo& currImageInfo = mDescriptorImageInfos.emplace_back();
					currImageInfo.imageView = pTexture->GetTextureViewHandle();
					currImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
					currImageInfo.sampler = VK_NULL_HANDLE;

					currWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
					currWriteDescriptorSet.pImageInfo = &currImageInfo;
				}
				break;
			}

			currWriteDescriptorSet.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			currWriteDescriptorSet.dstSet          = 0;
			currWriteDescriptorSet.descriptorCount = 1;
			currWriteDescriptorSet.dstBinding      = GetBindingOffsetByResourceType(E_SHADER_RESOURCE_TYPE::SRT_RW_IMAGE2D) + currBinding;
		}

		TDE2_ASSERT(mpActivePipelineStates[mCurrFrameIndex]);
		if (!mpActivePipelineStates[mCurrFrameIndex])
		{
			return;
		}
				
		vkCmdPushDescriptorSetKHR(_getCurrCommandBufferHandle(), 
			pipelineType,
			mpActivePipelineStates[mCurrFrameIndex]->GetPipelineLayout(),
			0, 
			static_cast<U32>(mDescriptorWrites.size()), 
			mDescriptorWrites.data());

		FlushBarriers();

		//mDescriptorsBindingsTable.Reset();
	}


	IGraphicsContext* CreateVulkanGraphicsContext(TPtr<IWindowSystem> pWindowSystem, TPtr<IWindowSurfaceFactory> pWindowSurfaceFactory, E_RESULT_CODE& result)
	{
		CVulkanGraphicsContext* pGraphicsContext = new (std::nothrow) CVulkanGraphicsContext(pWindowSurfaceFactory);

		if (!pGraphicsContext)
		{
			result = RC_OUT_OF_MEMORY;
			return nullptr;
		}

		result = pGraphicsContext->Init(pWindowSystem);

		if (result != RC_OK)
		{
			delete pGraphicsContext;
			pGraphicsContext = nullptr;
		}

		return dynamic_cast<IGraphicsContext*>(pGraphicsContext);
	}
}