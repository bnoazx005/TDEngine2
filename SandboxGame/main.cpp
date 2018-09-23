#include <iostream>
#include <TDEngine2.h>


#pragma comment(lib, "TDEngine2.lib")


using namespace TDEngine2;


int main(int argc, char** argv)
{
	E_RESULT_CODE result = RC_OK;

	IEngineCoreBuilder* pEngineCoreBuilder = CreateDefaultEngineCoreBuilder(CreateEngineCore, result);

	if (result != RC_OK)
	{
		return -1;
	}

	pEngineCoreBuilder->ConfigureWindowSystem("Sandbox Game", 800, 600, P_RESIZEABLE);
	pEngineCoreBuilder->ConfigureGraphicsContext(GCGT_OPENGL3X);

	IEngineCore* pEngineCore = pEngineCoreBuilder->GetEngineCore();

	pEngineCoreBuilder->Free();

	pEngineCore->Run();

	pEngineCore->Free();

	return 0;
}