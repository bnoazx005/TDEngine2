#include "../include/CVulkanGraphicsContext.h"
#include "../include/CVulkanUtils.h"
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
		return VK_FALSE;
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
		
		if (!pWindowSystem)
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

		mIsInitialized = true;
		
		return RC_OK;
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
	};
	

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


	static void PrepareDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
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

		VkApplicationInfo appInfo;
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = appName.c_str();
		appInfo.applicationVersion = VK_MAKE_VERSION(TDE2_MAJOR_VERSON, TDE2_MINOR_VERSION, TDE2_PATCH_VERSION);
		appInfo.pEngineName = "TDEngine2";
		appInfo.engineVersion = VK_MAKE_VERSION(TDE2_MAJOR_VERSON, TDE2_MINOR_VERSION, TDE2_PATCH_VERSION);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo;
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = static_cast<U32>(RequiredExtensions.size());
		createInfo.ppEnabledExtensionNames = RequiredExtensions.data();
		createInfo.enabledLayerCount = 0;

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


	static bool IsDeviceSuitable(VkPhysicalDevice device)
	{
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		// \todo Implement score based selection
		return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader;
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


	struct TQueuesCreateInfo
	{
		TDE2_STATIC_CONSTEXPR U32 InvalidIndex = std::numeric_limits<U32>::max();

		U32 mGraphicsQueueIndex = InvalidIndex;

		bool IsValid() const
		{
			return mGraphicsQueueIndex != InvalidIndex;
		}
	};


	static TQueuesCreateInfo GetQueuesCreateInfo(VkPhysicalDevice physDevice)
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
				break;
			}
		}

		return info;
	}


	static std::vector<VkDeviceQueueCreateInfo> PrepareQueuesCreateInfo(const TQueuesCreateInfo& info)
	{
		std::vector<VkDeviceQueueCreateInfo> queues;

		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = info.mGraphicsQueueIndex;
		queueCreateInfo.queueCount = 1;

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

		VkPhysicalDeviceFeatures deviceFeatures{};
		
		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = queuesInfos.data();
		createInfo.queueCreateInfoCount = static_cast<U32>(queuesInfos.size());
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = 0;

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


	E_RESULT_CODE CVulkanGraphicsContext::_onInitInternal()
	{
		VK_SAFE_CALL(volkInitialize());

		auto createInstanceResult = CreateInstance(mpWindowSystem->GetTitle());
		if (createInstanceResult.HasError())
		{
			return createInstanceResult.GetError();
		}

		mInstance = createInstanceResult.Get();

#if TDE2_DEBUG_MODE
		InitDebugMessageOutput(mInstance, mDebugMessenger);
#endif
		
		auto pickPhysicalDeviceResult = PickPhysicalDevice(mInstance);
		if (pickPhysicalDeviceResult.HasError())
		{
			return pickPhysicalDeviceResult.GetError();
		}

		mPhysicalDevice = pickPhysicalDeviceResult.Get();

		auto queuesInfo = GetQueuesCreateInfo(mPhysicalDevice);
		if (!queuesInfo.IsValid())
		{
			return RC_FAIL;
		}

		auto createLogicalDeviceResult = CreateLogicDevice(mPhysicalDevice, queuesInfo);
		if (createLogicalDeviceResult.HasError())
		{
			return createLogicalDeviceResult.GetError();
		}

		mDevice = createLogicalDeviceResult.Get();

		vkGetDeviceQueue(mDevice, queuesInfo.mGraphicsQueueIndex, 0, &mGraphicsQueue);

		return RC_OK;
	}

	E_RESULT_CODE CVulkanGraphicsContext::_onFreeInternal()
	{
#if TDE2_DEBUG_MODE
		DestroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);
#endif

		vkDestroyDevice(mDevice, nullptr);
		vkDestroyInstance(mInstance, nullptr);

		return RC_OK;
	}
	
	void CVulkanGraphicsContext::ClearBackBuffer(const TColor32F& color)
	{
	}

	void CVulkanGraphicsContext::ClearRenderTarget(IRenderTarget* pRenderTarget, const TColor32F& color)
	{
	}
	
	void CVulkanGraphicsContext::ClearRenderTarget(U8 slot, const TColor32F& color)
	{
	}

	void CVulkanGraphicsContext::ClearDepthBufferTarget(IDepthBufferTarget* pDepthBufferTarget, F32 value, U8 stencilValue)
	{
		TDE2_UNIMPLEMENTED();
	}

	void CVulkanGraphicsContext::ClearDepthBuffer(F32 value)
	{
	}
	
	void CVulkanGraphicsContext::ClearStencilBuffer(U8 value)
	{
	}

	void CVulkanGraphicsContext::Present()
	{
	}

	void CVulkanGraphicsContext::SetViewport(F32 x, F32 y, F32 width, F32 height, F32 minDepth, F32 maxDepth)
	{
	}
	
	void CVulkanGraphicsContext::SetScissorRect(const TRectU32& scissorRect)
	{
	}

	TMatrix4 CVulkanGraphicsContext::CalcPerspectiveMatrix(F32 fov, F32 aspect, F32 zn, F32 zf)
	{
		return PerspectiveProj(fov, aspect, zn, zf, -1.0f, 1.0f, -1.0f);
	}

	TMatrix4 CVulkanGraphicsContext::CalcOrthographicMatrix(F32 left, F32 top, F32 right, F32 bottom, F32 zn, F32 zf, bool isDepthless)
	{
		return OrthographicProj(left, top, right, bottom, zn, zf, -1.0f, 1.0f, -1.0f, isDepthless);
	}

	void CVulkanGraphicsContext::Draw(E_PRIMITIVE_TOPOLOGY_TYPE topology, U32 startVertex, U32 numOfVertices)
	{
	}

	void CVulkanGraphicsContext::DrawIndexed(E_PRIMITIVE_TOPOLOGY_TYPE topology, E_INDEX_FORMAT_TYPE indexFormatType, U32 baseVertex, U32 startIndex, U32 numOfIndices)
	{
	}

	void CVulkanGraphicsContext::DrawInstanced(E_PRIMITIVE_TOPOLOGY_TYPE topology, U32 startVertex, U32 verticesPerInstance, U32 startInstance, U32 numOfInstances)
	{
	}

	void CVulkanGraphicsContext::DrawIndexedInstanced(E_PRIMITIVE_TOPOLOGY_TYPE topology, E_INDEX_FORMAT_TYPE indexFormatType, U32 baseVertex, U32 startIndex,
												   U32 startInstance, U32 indicesPerInstance, U32 numOfInstances)
	{
	}

	void CVulkanGraphicsContext::DispatchCompute(U32 groupsCountX, U32 groupsCountY, U32 groupsCountZ)
	{
	}

	void CVulkanGraphicsContext::BindTextureSampler(U32 slot, TTextureSamplerId samplerId)
	{
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

	void CVulkanGraphicsContext::BindRenderTarget(U8 slot, IRenderTarget* pRenderTarget)
	{
	}

	void CVulkanGraphicsContext::BindDepthBufferTarget(IDepthBufferTarget* pDepthBufferTarget, bool disableRTWrite)
	{
	}

	void CVulkanGraphicsContext::SetDepthBufferEnabled(bool value)
	{
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
		return nullptr;
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

		SetViewport(0.0f, 0.0f, static_cast<F32>(pOnWindowResizedEvent->mWidth), static_cast<F32>(pOnWindowResizedEvent->mHeight), 0.0f, 1.0f);

		return RC_OK;
	}

	TEventListenerId CVulkanGraphicsContext::GetListenerId() const
	{
		return TEventListenerId(GetTypeId());
	}

	TVideoAdapterInfo CVulkanGraphicsContext::GetInfo() const
	{
		return {};
	}

	const TGraphicsContextInfo& CVulkanGraphicsContext::GetContextInfo() const
	{
		const static TGraphicsContextInfo infoData
		{
			{ { -1.0f, -1.0f, -1.0f }, { 1.0f, 1.0f, 1.0f } },
			true,
			E_GRAPHICS_CONTEXT_GAPI_TYPE::GCGT_VULKAN
		};

		return infoData;
	}


	TDE2_API IGraphicsContext* CreateVulkanGraphicsContext(TPtr<IWindowSystem> pWindowSystem, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IGraphicsContext, CVulkanGraphicsContext, result, pWindowSystem);
	}
}