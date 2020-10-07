/*!
	\file COGLUtils.h
	\date 18.01.2019
	\authors Kasimov Ildar
*/

#pragma once


#include <utils/Types.h>
#include <utils/Utils.h>
#include <utils/CFileLogger.h>
#include "COGLMappings.h"


namespace TDEngine2
{
	/*!
		\brief The macro is used for safe invokation of OpenGL's commands
	*/

	#define GL_SAFE_CALL(FunctionCall)																							\
			{																													\
				FunctionCall;																									\
				GLenum internalErrorCode = glGetError();																		\
				E_RESULT_CODE errCode = COGLMappings::GetErrorCode(internalErrorCode);											\
				if (errCode != RC_OK)																							\
				{																												\
					LOG_ERROR(Wrench::StringUtils::Format("GL error code: {0}", COGLMappings::ErrorCodeToString(internalErrorCode)));	\
					TDE2_ASSERT(false);																							\
					return errCode;																								\
				}																												\
			}


	/*!
		\brief The macro is used for safe invokation of OpenGL's commands
		within a functions that don't return any value
	*/

	#define GL_SAFE_VOID_CALL(FunctionCall)																						\
			{																													\
				FunctionCall;																									\
				GLenum internalErrorCode = glGetError();																		\
				E_RESULT_CODE errCode = COGLMappings::GetErrorCode(internalErrorCode);											\
				if (errCode != RC_OK)																							\
				{																												\
					LOG_ERROR(Wrench::StringUtils::Format("GL error code: {0}", COGLMappings::ErrorCodeToString(internalErrorCode)));	\
					TDE2_ASSERT(false);																							\
				}																												\
			}
}
