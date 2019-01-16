#include <iostream>
#include "include/CCustomEngineListener.h"
#include <TDEngine2.h>

#if defined (TDE2_USE_WIN32PLATFORM)
	#pragma comment(lib, "TDEngine2.lib")
#endif

using namespace TDEngine2;


int main(int argc, char** argv)
{
	E_RESULT_CODE result = RC_OK;
	
	IEngineCoreBuilder* pEngineCoreBuilder = CreateDefaultEngineCoreBuilder(CreateEngineCore, result);

	if (result != RC_OK)
	{
		return -1;
	}

	PANIC_ON_FAILURE(pEngineCoreBuilder->ConfigureMemoryManager(DefaultGlobalMemoryBlockSize));
	PANIC_ON_FAILURE(pEngineCoreBuilder->ConfigureJobManager());
	PANIC_ON_FAILURE(pEngineCoreBuilder->ConfigureFileSystem());
	PANIC_ON_FAILURE(pEngineCoreBuilder->ConfigureEventManager());
	PANIC_ON_FAILURE(pEngineCoreBuilder->ConfigureResourceManager());
	PANIC_ON_FAILURE(pEngineCoreBuilder->ConfigureWindowSystem("Sandbox Game", 800, 600, P_RESIZEABLE | P_ZBUFFER_ENABLED));
	PANIC_ON_FAILURE(pEngineCoreBuilder->ConfigurePluginManager());
	PANIC_ON_FAILURE(pEngineCoreBuilder->ConfigureGraphicsContext(GCGT_OPENGL3X));
	PANIC_ON_FAILURE(pEngineCoreBuilder->ConfigureInputContext());
	PANIC_ON_FAILURE(pEngineCoreBuilder->ConfigureRenderer());

	IEngineCore* pEngineCore = pEngineCoreBuilder->GetEngineCore();

	pEngineCoreBuilder->Free();
	
	IEngineListener* pCustomListener = new CCustomEngineListener();

	pEngineCore->RegisterListener(pCustomListener);

	pEngineCore->Run();
	
	pEngineCore->Free();

	delete pCustomListener;

	return 0;
}