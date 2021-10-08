#include <iostream>
#include "include/CCustomEngineListener.h"
#include <TDEngine2.h>
#include <thread>

#if defined (TDE2_USE_WIN32PLATFORM)
	#pragma comment(lib, "TDEngine2.lib")
#endif

using namespace TDEngine2;


int main(int argc, char** argv)
{
	E_RESULT_CODE result = RC_OK;
	
	TPtr<IEngineCoreBuilder> pEngineCoreBuilder = TPtr<IEngineCoreBuilder>(CreateConfigFileEngineCoreBuilder({ CreateEngineCore, "SandboxGame.project" }, result));

	if (result != RC_OK)
	{
		return -1;
	}

	TPtr<IEngineCore> pEngineCore = TPtr<IEngineCore>(pEngineCoreBuilder->GetEngineCore());

	pEngineCore->RegisterListener(std::make_unique<CCustomEngineListener>());

	pEngineCore->Run();

	return 0;
}