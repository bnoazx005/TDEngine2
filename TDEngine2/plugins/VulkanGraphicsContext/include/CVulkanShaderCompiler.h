/*!
	\file CVulkanShaderCompiler.h
	\date 03.11.2023
	\authors Kasimov Ildar
*/

#pragma once


#include <graphics/CBaseShaderCompiler.h>
#include <vector>


namespace TDEngine2
{
	class IDLLManager;


	/*!
		\brief A factory function for creation objects of CVulkanShaderCompiler's type
		
		\param[in, out] pFileSystem A pointer to IFileSystem implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CVulkanShaderCompiler's implementation
	*/

	IShaderCompiler* CreateVulkanShaderCompiler(IFileSystem* pFileSystem, IDLLManager* pDLLManager, E_RESULT_CODE& result);
}