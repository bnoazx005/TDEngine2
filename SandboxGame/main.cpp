#include <iostream>
#include "include/CCustomEngineListener.h"
#include <TDEngine2.h>

#if defined (TDE2_USE_WIN32PLATFORM)
	#pragma comment(lib, "TDEngine2.lib")
#endif

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

	pEngineCoreBuilder->ConfigureJobManager();
	pEngineCoreBuilder->ConfigureFileSystem();
	pEngineCoreBuilder->ConfigureResourceManager();
	pEngineCoreBuilder->ConfigureWindowSystem("Sandbox Game", 800, 600, P_RESIZEABLE);
	pEngineCoreBuilder->ConfigurePluginManager();
	pEngineCoreBuilder->ConfigureGraphicsContext(GCGT_OPENGL3X);

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

	//pFileSystem->Mount(pFileSystem->GetCurrDirectory() + "\\data", "\\foo");
	//pFileSystem->Mount(pFileSystem->GetCurrDirectory() + "\\test", "\\test\\.\\..");

	//std::string path = pFileSystem->ResolveVirtualPath("\\");
	//path = pFileSystem->ResolveVirtualPath("\\foo.txt");
	//path = pFileSystem->ResolveVirtualPath("\\foo");
	//path = pFileSystem->ResolveVirtualPath(".\\foo\\text.txt");
	//path = pFileSystem->ResolveVirtualPath("\\foo\\text.txt");
	//path = pFileSystem->ResolveVirtualPath("\\foo\\.\\..\\text");

	IResourceManager* pResourceManager = dynamic_cast<IResourceManager*>(pEngineCore->GetSubsystem(EST_RESOURCE_MANAGER));

	IJobManager* pJobManager = dynamic_cast<IJobManager*>(pEngineCore->GetSubsystem(EST_JOB_MANAGER));

	CFileLogger* pFileLogger = new CFileLogger("log.txt");
	
	pJobManager->SubmitJob<CFileLogger*>([](CFileLogger* pFileLogger) 
	{
		for (int i = 0; i < 1000; ++i)
		{
			pFileLogger->LogMessage("t1 message...");
		}
	}, pFileLogger);

	pJobManager->SubmitJob<CFileLogger*>([](CFileLogger* pFileLogger)
	{
		for (int i = 0; i < 1000; ++i)
		{
			pFileLogger->LogMessage("t2 message...");
		}
	}, pFileLogger);

	IConfigFileReader* pConfigFile = pFileSystem->Create<CConfigFileReader>("engine.cfg", result);
	auto p = pConfigFile->GetBool("script", "shared_state");

	pEngineCore->Free();
	
	delete pCustomListener;

	return 0;
}