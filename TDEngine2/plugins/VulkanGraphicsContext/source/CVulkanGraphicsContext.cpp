#include "../include/CVulkanGraphicsContext.h"
#include "../include/CVulkanUtils.h"
#include "../include/IWindowSurfaceFactory.h"
#include "../include/CVulkanGraphicsObjectManager.h"
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



	CVulkanGraphicsContext::CVulkanGraphicsContext(TPtr<IWindowSurfaceFactory> pWindowSurfaceFactory):
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


	static const std::vector<const C8*> RequiredDeviceExtensions
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
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


	static bool CheckDeviceRequiredExtensions(VkPhysicalDevice device)
	{
		U32 extensionsCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionsCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, availableExtensions.data());

		USIZE supportedExtensionsCount = 0;

		for (auto&& currExtension : RequiredDeviceExtensions)
		{
			auto it = std::find_if(availableExtensions.cbegin(), availableExtensions.cend(), [&currExtension](auto&& ext) { return strcmp(ext.extensionName, currExtension) == 0; });
			if (it != availableExtensions.cend())
			{
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

		createInfo.enabledExtensionCount = static_cast<U32>(RequiredDeviceExtensions.size());
		createInfo.ppEnabledExtensionNames = RequiredDeviceExtensions.data();

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
		
		mSurface = mpWindowSurfaceFactory->GetSurface(mInstance);
		if (VK_NULL_HANDLE == mSurface)
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

		mQueuesInfo = GetQueuesCreateInfo(mPhysicalDevice, mSurface);
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

		E_RESULT_CODE result = _createSwapChain();
		if (RC_OK != result)
		{
			return result;
		}

		result = _prepareCommandBuffers();
		if (RC_OK != result)
		{
			return result;
		}

		return RC_OK;
	}

	E_RESULT_CODE CVulkanGraphicsContext::_onFreeInternal()
	{
#if TDE2_DEBUG_MODE
		DestroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);
#endif
		
		vkDestroyCommandPool(mDevice, mMainCommandPool, nullptr);
		
		for (auto& currImageView : mSwapChainImageViews) 
		{
			vkDestroyImageView(mDevice, currImageView, nullptr);
		}

		for (USIZE i = 0; i < mNumOfCommandsBuffers; i++)
		{
			vkDestroyFence(mDevice, mCommandBuffersFences[i], nullptr);
			vkDestroySemaphore(mDevice, mImageReadySemaphore[i], nullptr);
			vkDestroySemaphore(mDevice, mRenderFinishedSemaphore[i], nullptr);
		}

		vkDestroySwapchainKHR(mDevice, mSwapChain, nullptr);
		vkDestroyDevice(mDevice, nullptr);
		vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
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

	void CVulkanGraphicsContext::BeginFrame()
	{
		VK_SAFE_VOID_CALL(vkWaitForFences(mDevice, 1, &mCommandBuffersFences[mCurrFrameIndex], VK_TRUE, UINT64_MAX));
		VK_SAFE_VOID_CALL(vkResetFences(mDevice, 1, &mCommandBuffersFences[mCurrFrameIndex]));

		VK_SAFE_VOID_CALL(vkAcquireNextImageKHR(mDevice, mSwapChain, UINT64_MAX, mImageReadySemaphore[mCurrFrameIndex], VK_NULL_HANDLE, &mCurrUsedImageIndex));

		VK_SAFE_VOID_CALL(vkResetCommandBuffer(mCommandBuffers[mCurrFrameIndex], 0));

	}

	void CVulkanGraphicsContext::Present()
	{
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { mImageReadySemaphore[mCurrFrameIndex] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &mCommandBuffers[mCurrFrameIndex];

		VkSemaphore signalSemaphores[] = { mRenderFinishedSemaphore[mCurrFrameIndex] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		VK_SAFE_VOID_CALL(vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, mCommandBuffersFences[mCurrFrameIndex]));

		// actual present
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { mSwapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;

		presentInfo.pImageIndices = &mCurrUsedImageIndex;

		vkQueuePresentKHR(mPresentQueue, &presentInfo);

		mCurrFrameIndex = (mCurrFrameIndex + 1) % mNumOfCommandsBuffers;
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

		SetViewport(0.0f, 0.0f, static_cast<F32>(pOnWindowResizedEvent->mWidth), static_cast<F32>(pOnWindowResizedEvent->mHeight), 0.0f, 1.0f);

		return RC_OK;
	}

#if TDE2_DEBUG_MODE

	void CVulkanGraphicsContext::BeginSectionMarker(const std::string& id)
	{
	}

	void CVulkanGraphicsContext::EndSectionMarker()
	{
	}

#endif

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

	VkDevice CVulkanGraphicsContext::GetDevice() const
	{
		return mDevice;
	}

	E_RESULT_CODE CVulkanGraphicsContext::_createSwapChain()
	{
		E_RESULT_CODE result = RC_OK;

		const U32 flags = mpWindowSystem->GetFlags();

		auto swapChainSupportInfo = GetSwapChainSupportInfo(mPhysicalDevice, mSurface);
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
			mSwapChainExtents.width = windowRect.width;
			mSwapChainExtents.height = windowRect.height;
		}

		U32 imagesCount = swapChainSupportInfo.mCapabilities.minImageCount + 1;
		imagesCount = std::min(imagesCount, swapChainSupportInfo.mCapabilities.maxImageCount);

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = mSurface;
		createInfo.minImageCount = imagesCount;
		createInfo.imageFormat = mSwapChainFormat.format;
		createInfo.imageColorSpace = mSwapChainFormat.colorSpace;
		createInfo.imageExtent = mSwapChainExtents;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		auto queuesInfo = GetQueuesCreateInfo(mPhysicalDevice, mSurface);

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

		VK_SAFE_CALL(vkCreateSwapchainKHR(mDevice, &createInfo, nullptr, &mSwapChain));

		VK_SAFE_CALL(vkGetSwapchainImagesKHR(mDevice, mSwapChain, &imagesCount, nullptr));
		
		mSwapChainImages.resize(static_cast<USIZE>(imagesCount));
		VK_SAFE_CALL(vkGetSwapchainImagesKHR(mDevice, mSwapChain, &imagesCount, mSwapChainImages.data()));

		mSwapChainImageViews.resize(mSwapChainImages.size());

		for (USIZE i = 0; i < mSwapChainImageViews.size(); i++)
		{
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = mSwapChainImages[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = mSwapChainFormat.format;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			VK_SAFE_CALL(vkCreateImageView(mDevice, &createInfo, nullptr, &mSwapChainImageViews[i]));
		}

		return result;
	}

	E_RESULT_CODE CVulkanGraphicsContext::_prepareCommandBuffers()
	{
		E_RESULT_CODE result = RC_OK;

		VkCommandPoolCreateInfo poolInfo {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = mQueuesInfo.mGraphicsQueueIndex;

		VK_SAFE_CALL(vkCreateCommandPool(mDevice, &poolInfo, nullptr, &mMainCommandPool));

		// \note Create a few command buffers within main command pool
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = mMainCommandPool;
		allocInfo.commandBufferCount = mNumOfCommandsBuffers;

		VK_SAFE_CALL(vkAllocateCommandBuffers(mDevice, &allocInfo, mCommandBuffers.data()));

		// \note Create fences one for each command buffer
		VkFenceCreateInfo fenceCreateInfo = {};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		VkSemaphoreCreateInfo semaphoreCreateInfo = {};
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		for (USIZE i = 0; i < mNumOfCommandsBuffers; i++)
		{
			VK_SAFE_CALL(vkCreateFence(mDevice, &fenceCreateInfo, nullptr, &mCommandBuffersFences[i]));

			VK_SAFE_CALL(vkCreateSemaphore(mDevice, &semaphoreCreateInfo, nullptr, &mImageReadySemaphore[i]));
			VK_SAFE_CALL(vkCreateSemaphore(mDevice, &semaphoreCreateInfo, nullptr, &mRenderFinishedSemaphore[i]));
		}

		return result;
	}


	TDE2_API IGraphicsContext* CreateVulkanGraphicsContext(TPtr<IWindowSystem> pWindowSystem, TPtr<IWindowSurfaceFactory> pWindowSurfaceFactory, E_RESULT_CODE& result)
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