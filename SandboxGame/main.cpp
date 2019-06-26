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
	
	IEngineCoreBuilder* pEngineCoreBuilder = CreateConfigFileEngineCoreBuilder(CreateEngineCore, "settings.cfg", result);

	if (result != RC_OK)
	{
		return -1;
	}

	IEngineCore* pEngineCore = pEngineCoreBuilder->GetEngineCore();

	pEngineCoreBuilder->Free();
	
	IEngineListener* pCustomListener = new CCustomEngineListener();

	pEngineCore->RegisterListener(pCustomListener);

	pEngineCore->Run();
	
	pEngineCore->Free();

	delete pCustomListener;

	return 0;
}