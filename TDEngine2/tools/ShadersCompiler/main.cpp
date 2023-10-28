/*!
	\author Ildar Kasimov
	\date 19.10.2023

	\brief The utility is intended for precompilation of shaders into binary cache blobs that speeds up their loading in runtime
*/

#include <iostream>

#define TDE2_USE_NOEXCEPT 1
#include "shadersCompiler.h"


using namespace TDEngine2;

#if defined (TDE2_USE_WINPLATFORM)
	#pragma comment(lib, "TDEngine2.lib")
#endif


TDEngine2::E_RESULT_CODE ParseOptions(int argc, const char** argv)
{
	TDEngine2::CProgramOptions::TParseArgsParams optionsParams;
	optionsParams.mArgsCount = argc;
	optionsParams.mpArgsValues = argv;
	optionsParams.mpProgramUsageStr = "tde2_shaders_compiler [options]";

	TDEngine2::CProgramOptions::Get()->AddArgument({ '\0', "graphics", "A string which determines target GAPI's plugin (./D3D11GraphicsContext|./GLGraphicsContext)", TDEngine2::CProgramOptions::TArgumentParams::E_VALUE_TYPE::STRING});

	return TDEngine2::CProgramOptions::Get()->ParseArgs(optionsParams);
}


int main(int argc, const char** argv)
{
#if TDE2_EDITORS_ENABLED

	E_RESULT_CODE result = ParseOptions(argc, argv);

	CScopedPtr<IEngineCoreBuilder> pEngineCoreBuilder { CreateConfigFileEngineCoreBuilder({ CreateEngineCore, "tde2_shaders_compiler.project" }, result) };
	if (result != RC_OK)
	{
		return -1;
	}

	CScopedPtr<IEngineCore> pEngineCore { pEngineCoreBuilder->GetEngineCore() };

	auto pResourceManager = pEngineCore->GetSubsystem<IResourceManager>();
	auto pManifest = pResourceManager->GetResourcesRuntimeManifest();

	result = ProcessShaders(pEngineCore.Get(), BuildFilesList(pEngineCore->GetSubsystem<IFileSystem>(), GetDefaultShadersPaths()));
	if (RC_OK != result)
	{
		return -1;
	}

	return 0;

#else

	return -1;

#endif
}