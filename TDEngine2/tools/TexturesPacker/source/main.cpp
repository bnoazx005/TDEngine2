#include "./../include/CUtilityListener.h"
#include <TDEngine2.h>
#include <iostream>
#include <memory>

using namespace TDEngine2;

#if defined (TDE2_USE_WINPLATFORM)
	#pragma comment(lib, "TDEngine2.lib")
#endif


I32 main(I32 argc, const C8** argv)
{
	E_RESULT_CODE result = RC_OK;

#if TDE2_EDITORS_ENABLED
	auto parseOptionsResult = ParseOptions(argc, argv);
	if (parseOptionsResult.HasError())
	{
		return parseOptionsResult.GetError();
	}
#endif

	TPtr<IEngineCoreBuilder> pEngineCoreBuilder = TPtr<IEngineCoreBuilder>(CreateConfigFileEngineCoreBuilder({ CreateEngineCore, "TDE2TexturesPacker.project" }, result));

	if (result != RC_OK)
	{
		return -1;
	}

	TPtr<IEngineCore> pEngineCore = TPtr<IEngineCore>(pEngineCoreBuilder->GetEngineCore());

#if TDE2_EDITORS_ENABLED
	pEngineCore->RegisterListener(std::make_unique<CUtilityListener>(parseOptionsResult.Get()));
#endif

	pEngineCore->Run();

	return 0;
}