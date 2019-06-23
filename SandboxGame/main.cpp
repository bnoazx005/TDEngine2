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
	
	TEngineSettings settings;
	settings.mGraphicsContextType         = GCGT_OPENGL3X;
	settings.mApplicationName             = "Sandbox Game";
	settings.mWindowWidth                 = 800;
	settings.mWindowHeight                = 600;
	settings.mFlags                       = P_RESIZEABLE | P_ZBUFFER_ENABLED;
	settings.mMaxNumOfWorkerThreads       = std::thread::hardware_concurrency() - 1;
	settings.mTotalPreallocatedMemorySize = DefaultGlobalMemoryBlockSize;

	IEngineCoreBuilder* pEngineCoreBuilder = CreateDefaultEngineCoreBuilder(CreateEngineCore, settings, result);

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