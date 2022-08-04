#include "../include/CCustomEngineListener.h"
#include <TDEngine2.h>
#include <iostream>


using namespace TDEngine2;


E_RESULT_CODE CCustomEngineListener::OnStart()
{
	return RC_OK;
}

E_RESULT_CODE CCustomEngineListener::OnUpdate(const float& dt)
{
	return RC_OK;
}

E_RESULT_CODE CCustomEngineListener::OnFree()
{
	return RC_OK;
}

void CCustomEngineListener::SetEngineInstance(IEngineCore* pEngineCore)
{
	if (!pEngineCore)
	{
		return;
	}

	mpEngineCoreInstance = pEngineCore;

	mpGraphicsContext = mpEngineCoreInstance->GetSubsystem<IGraphicsContext>();
	mpWindowSystem    = mpEngineCoreInstance->GetSubsystem<IWindowSystem>();
	mpResourceManager = mpEngineCoreInstance->GetSubsystem<IResourceManager>();
	mpInputContext    = mpEngineCoreInstance->GetSubsystem<IDesktopInputContext>();
	mpFileSystem      = mpEngineCoreInstance->GetSubsystem<IFileSystem>();
}
