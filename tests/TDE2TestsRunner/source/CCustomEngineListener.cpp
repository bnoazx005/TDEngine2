#include "../include/CCustomEngineListener.h"
#include <TDEngine2.h>
#include <iostream>


using namespace TDEngine2;


#if TDE2_EDITORS_ENABLED

E_RESULT_CODE CCustomEngineListener::OnStart()
{
	E_RESULT_CODE result = CTestContext::Get()->SetArtifactsOutputDirectory(
		CProgramOptions::Get()->GetValueOrDefault("output-artifacts-dir", mpFileSystem->CombinePath(mpFileSystem->GetCurrDirectory(), "TestArtifacts/")));

	result = result | CTestContext::Get()->RunAllTests();

	return result;
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

#else

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
	mpWindowSystem = mpEngineCoreInstance->GetSubsystem<IWindowSystem>();
	mpResourceManager = mpEngineCoreInstance->GetSubsystem<IResourceManager>();
	mpInputContext = mpEngineCoreInstance->GetSubsystem<IDesktopInputContext>();
	mpFileSystem = mpEngineCoreInstance->GetSubsystem<IFileSystem>();
}

#endif