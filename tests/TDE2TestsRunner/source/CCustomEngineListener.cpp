#include "../include/CCustomEngineListener.h"
#include <TDEngine2.h>
#include <iostream>


using namespace TDEngine2;


E_RESULT_CODE CCustomEngineListener::OnStart()
{
	CTestContext::Get()->Init({ mpEngineCoreInstance });
	CTestContext::Get()->RunAllTests();

	return RC_OK;
}

E_RESULT_CODE CCustomEngineListener::OnUpdate(const float& dt)
{
	CTestContext::Get()->Update(dt);

	if (CTestContext::Get()->IsFinished())
	{
		mpEngineCoreInstance->Quit();
	}

	return RC_OK;
}

E_RESULT_CODE CCustomEngineListener::OnFree()
{
	E_RESULT_CODE result = RC_OK;
	
	auto pTestResultsReporter = TPtr<ITestResultsReporter>(CreateTestResultsTextReporter(mpFileSystem, "TestResults.txt", result));
	if (!pTestResultsReporter || RC_OK != result)
	{
		return result;
	}

	CTestContext::Get()->WriteTestResults(pTestResultsReporter);

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
