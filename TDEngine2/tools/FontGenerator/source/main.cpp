#include "./../include/CUtilityListener.h"
#include <TDEngine2.h>
#include <iostream>
#include <memory>

using namespace TDEngine2;

#if defined (TDE2_USE_WIN32PLATFORM)
	#pragma comment(lib, "TDEngine2.lib")
#endif


I32 main(I32 argc, C8** argv)
{
	E_RESULT_CODE result = RC_OK;

	IEngineCoreBuilder* pEngineCoreBuilder = CreateConfigFileEngineCoreBuilder(CreateEngineCore, "settings.cfg", result);

	if (result != RC_OK)
	{
		return -1;
	}

	IEngineCore* pEngineCore = pEngineCoreBuilder->GetEngineCore();

	pEngineCoreBuilder->Free();
	
	std::unique_ptr<CUtilityListener> pUtilityListener = std::make_unique<CUtilityListener>();

	pEngineCore->RegisterListener(pUtilityListener.get());

	// \note load a plugin that enables YAML format's support
	auto pPluginManager = dynamic_cast<IPluginManager*>(pEngineCore->GetSubsystem(EST_PLUGIN_MANAGER));
	
	if (pPluginManager->LoadPlugin("YAMLFormatSupport") != RC_OK)
	{
		return -1;
	}

	pEngineCore->Run();

	pEngineCore->Free();

	return 0;
}