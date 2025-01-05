/*!
	\file CVulkanUtils.h
	\date 29.10.2023
	\authors Kasimov Ildar
*/

#pragma once


#include <utils/Types.h>
#include <utils/Utils.h>
#include <utils/CFileLogger.h>
#include "CVulkanMappings.h"


namespace TDEngine2
{
	/*!
		\brief The macro is used for debugging Vulkan calls when the owning function returns E_RESULT_CODE value
	*/

	#define VK_SAFE_CALL(...)																													\
			do {																																\
				const VkResult internalErrorCode = __VA_ARGS__; TDE2_ASSERT(VK_SUCCESS == internalErrorCode);									\
				if (const E_RESULT_CODE errorCode = CVulkanMappings::GetErrorCode(internalErrorCode);	RC_OK != errorCode)						\
				{																																\
					return errorCode;																											\
				}																																\
			} while(false)

#define VK_SAFE_TRESULT_CALL(...)																												\
			do {																																\
				const VkResult internalErrorCode = __VA_ARGS__; TDE2_ASSERT(VK_SUCCESS == internalErrorCode);									\
				if (const E_RESULT_CODE errorCode = CVulkanMappings::GetErrorCode(internalErrorCode);	RC_OK != errorCode)						\
				{																																\
					TDE2_ASSERT(false);																											\
					return Wrench::TErrValue<E_RESULT_CODE>(errorCode);																			\
				}																																\
			} while(false)


	/*!
		\brief The macro is used for debugging Vulkan calls when the owning function returns nothing
	*/

	#define VK_SAFE_VOID_CALL(...)																												\
			do {																																\
				const VkResult internalErrorCode = __VA_ARGS__; TDE2_ASSERT(VK_SUCCESS == internalErrorCode);									\
				const E_RESULT_CODE errorCode = CVulkanMappings::GetErrorCode(internalErrorCode);												\
				if (errorCode != RC_OK)																											\
				{																																\
					TDE2_ASSERT(false);																											\
				}																																\
			} while(false)
}
