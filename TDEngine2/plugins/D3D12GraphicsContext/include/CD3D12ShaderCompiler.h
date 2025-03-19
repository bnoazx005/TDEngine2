/*!
	\file CD3D12ShaderCompiler.h
	\date 19.03.2025
	\authors Kasimov Ildar
*/

#pragma once


#include <graphics/CBaseShaderCompiler.h>
#include <vector>


#if defined(TDE2_USE_WINPLATFORM) /// Used only on Windows platform


namespace TDEngine2
{
	class IDLLManager;


	/*!
		\brief A factory function for creation objects of CD3D12ShaderCompiler's type
		
		\param[in, out] pFileSystem A pointer to IFileSystem implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CD3D12ShaderCompiler's implementation
	*/

	IShaderCompiler* CreateD3D12ShaderCompiler(IFileSystem* pFileSystem, IDLLManager* pDLLManager, E_RESULT_CODE& result);
}


#endif