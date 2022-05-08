#include "./../include/CUtilityListener.h"
#include <TDEngine2.h>
#include <iostream>
#include <memory>

using namespace TDEngine2;

#if defined (TDE2_USE_WINPLATFORM)
	#pragma comment(lib, "TDEngine2.lib")
#endif


I32 main(I32 argc, C8** argv)
{
	E_RESULT_CODE result = RC_OK;

	TPtr<IEngineCoreBuilder> pEngineCoreBuilder = TPtr<IEngineCoreBuilder>(CreateConfigFileEngineCoreBuilder({ CreateEngineCore, "TDE2AnimationEd.project" }, result));

	if (result != RC_OK)
	{
		return -1;
	}

	TPtr<IEngineCore> pEngineCore = TPtr<IEngineCore>(pEngineCoreBuilder->GetEngineCore());

	pEngineCore->RegisterListener(std::make_unique<CUtilityListener>());

	pEngineCore->Run();

	return 0;
}