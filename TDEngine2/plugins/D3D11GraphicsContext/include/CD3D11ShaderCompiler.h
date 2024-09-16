/*!
	\file CD3D11ShaderCompiler.h
	\date 22.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include <graphics/CBaseShaderCompiler.h>
#include <vector>
#include <unordered_map>


#if defined (TDE2_USE_WINPLATFORM)

namespace TDEngine2
{
	/*!
		struct TD3D11ShaderCompilerOutput

		\brief The structure contains shader compiler's output data for D3D11 GAPI
	*/

	typedef struct TD3D11ShaderCompilerOutput: public TShaderCompilerOutput
	{
		virtual ~TD3D11ShaderCompilerOutput() = default;
	} TD3D11ShaderCompilerOutput, *TD3D11ShaderCompilerOutputPtr;


	/*!
		\brief A factory function for creation objects of CD3D11ShaderCompiler's type
		
		\param[in, out] pFileSystem A pointer to IFileSystem implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CD3D11ShaderCompiler's implementation
	*/

	IShaderCompiler* CreateD3D11ShaderCompiler(IFileSystem* pFileSystem, E_RESULT_CODE& result);
}

#endif