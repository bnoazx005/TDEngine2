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

	pEngineCoreBuilder->ConfigureJobManager();
	pEngineCoreBuilder->ConfigureFileSystem();
	pEngineCoreBuilder->ConfigureEventManager();
	pEngineCoreBuilder->ConfigureResourceManager();
	pEngineCoreBuilder->ConfigureWindowSystem("Sandbox Game", 800, 600, P_RESIZEABLE);
	pEngineCoreBuilder->ConfigurePluginManager();
	pEngineCoreBuilder->ConfigureGraphicsContext(GCGT_DIRECT3D11);

	IEngineCore* pEngineCore = pEngineCoreBuilder->GetEngineCore();
	
	IWorld* pWorld = CreateWorld(result);

	auto entity = pWorld->CreateEntity();

	entity->AddComponent<CQuadSprite>();

	pWorld->Free();

	pEngineCoreBuilder->Free();
	
	IEngineListener* pCustomListener = new CCustomEngineListener();

	pEngineCore->RegisterListener(pCustomListener);

	pEngineCore->Run();
	
	pEngineCore->Free();

	delete pCustomListener;

	return 0;
}