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

	pEngineCoreBuilder->ConfigureFileSystem();
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
	
	IFileSystem* pFileSystem = dynamic_cast<IFileSystem*>(pEngineCore->GetSubsystem(EST_FILE_SYSTEM));

	pFileSystem->Mount(pFileSystem->GetCurrDirectory() + "\\data", "\\foo");
	pFileSystem->Mount(pFileSystem->GetCurrDirectory() + "\\test", "\\test\\.\\..");

	std::string path = pFileSystem->ResolveVirtualPath("\\");
	path = pFileSystem->ResolveVirtualPath("\\foo.txt");
	path = pFileSystem->ResolveVirtualPath("\\foo");
	path = pFileSystem->ResolveVirtualPath(".\\foo\\text.txt");
	path = pFileSystem->ResolveVirtualPath("\\foo\\text.txt");
	path = pFileSystem->ResolveVirtualPath("\\foo\\.\\..\\text");

	TVertDeclElementDesc desc;
	desc.mOGL.mFormatType = FT_UNKNOWN;
	
	pEngineCore->Free();
	
	delete pCustomListener;

	return 0;
}