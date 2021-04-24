#include "./../include/CUtilityListener.h"
#include <TDEngine2.h>
#include <iostream>
#include <memory>

using namespace TDEngine2;

#if defined (TDE2_USE_WIN32PLATFORM)
	#pragma comment(lib, "TDEngine2.lib")
#endif


I32 main(I32 argc, const C8** argv)
{
	E_RESULT_CODE result = RC_OK;

	auto parseOptionsResult = ParseOptions(argc, argv);
	if (parseOptionsResult.HasError())
	{
		return parseOptionsResult.GetError();
	}

	IEngineCoreBuilder* pEngineCoreBuilder = CreateConfigFileEngineCoreBuilder(CreateEngineCore, "settings.cfg", result);

	if (result != RC_OK)
	{
		return -1;
	}

	IEngineCore* pEngineCore = pEngineCoreBuilder->GetEngineCore();

	pEngineCoreBuilder->Free();
	
	std::unique_ptr<CUtilityListener> pUtilityListener = std::make_unique<CUtilityListener>(parseOptionsResult.Get());

	pEngineCore->RegisterListener(pUtilityListener.get());

	pEngineCore->Run();

	pEngineCore->Free();

	return 0;
}