#include <iostream>
#include "include/CCustomEngineListener.h"
#include <TDEngine2.h>


#pragma comment(lib, "TDEngine2.lib")


using namespace TDEngine2;


class CTransform : public CBaseComponent<CTransform> {};


int main(int argc, char** argv)
{
	E_RESULT_CODE result = RC_OK;
	
	IEngineCoreBuilder* pEngineCoreBuilder = CreateDefaultEngineCoreBuilder(CreateEngineCore, result);

	if (result != RC_OK)
	{
		return -1;
	}

	pEngineCoreBuilder->ConfigureWindowSystem("Sandbox Game", 800, 600, P_RESIZEABLE);
	pEngineCoreBuilder->ConfigureGraphicsContext(GCGT_DIRECT3D11);

	IEngineCore* pEngineCore = pEngineCoreBuilder->GetEngineCore();
	
	pEngineCoreBuilder->Free();

	IWorld* pWorld = CreateWorld(result);

	CEntity* e = pWorld->CreateEntity();

	e->AddComponent<CTransform>();
	
	pWorld->Free();

	IEngineListener* pCustomListener = new CCustomEngineListener();

	pEngineCore->RegisterListener(pCustomListener);

	pEngineCore->Run();
	
	pEngineCore->Free();
	
	delete pCustomListener;

	return 0;
}