/*!
	\file shadersCompiler.h
	\date 19.10.2023
	\author Ildar Kasimov

	\brief 
*/

#pragma once


#include <TDEngine2.h>
#include <vector>
#include <string>


namespace TDEngine2
{

#if TDE2_USE_NOEXCEPT
	#define TDE2_NOEXCEPT noexcept
#else 
	#define TDE2_NOEXCEPT 
#endif


	static struct TVersion
	{
		const uint32_t mMajor = 0;
		const uint32_t mMinor = 1;
	} ToolVersion;


	std::vector<std::string> GetDefaultShadersPaths() TDE2_NOEXCEPT;

	std::vector<std::string> BuildFilesList(TPtr<IFileSystem> pFileSystem, const std::vector<std::string>& directories) TDE2_NOEXCEPT;

	E_RESULT_CODE ProcessShaders(IEngineCore* pEngineCore, std::vector<std::string>&& files) TDE2_NOEXCEPT;

}