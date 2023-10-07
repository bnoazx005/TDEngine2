#include "../../include/autotests/CTestResultsTextReporter.h"
#include "../../include/core/IFileSystem.h"
#include "../../include/core/IFile.h"
#include "../../include/utils/CFileLogger.h"
#include "../../include/autotests/CTestContext.h"
#include "stringUtils.hpp"


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	static void LogMessage(ITextFileWriter* pFileWriter, const std::string& str)
	{
		pFileWriter->WriteLine(str);
		LOG_ERROR(str);
	}


	CTestResultsTextReporter::CTestResultsTextReporter() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CTestResultsTextReporter::Init(TPtr<IFileSystem> pFileSystem, const std::string& filename)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pFileSystem || filename.empty())
		{
			return RC_INVALID_ARGS;
		}

		mpFileSystem = pFileSystem;

		auto openFileResult = pFileSystem->Open<ITextFileWriter>(filename, true);
		if (openFileResult.HasError())
		{
			return openFileResult.GetError();
		}

		mFileHandler = openFileResult.Get();

		LogMessage(mpFileSystem->Get<ITextFileWriter>(mFileHandler), "Auto tests results:\n");

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CTestResultsTextReporter::WriteTestsStatistics(const TTestContextStatistics& stats)
	{
		LogMessage(mpFileSystem->Get<ITextFileWriter>(mFileHandler), 
			Wrench::StringUtils::Format("{0} / {1} test cases successfully passed", stats.mPassedTestsCount, stats.mFailedTestsCount + stats.mPassedTestsCount));

		if (stats.mFailedTestsCount > 0)
		{
			LogMessage(mpFileSystem->Get<ITextFileWriter>(mFileHandler), Wrench::StringUtils::Format("!!! {0} tests failed", stats.mFailedTestsCount));
		}

		return RC_OK;
	}

	E_RESULT_CODE CTestResultsTextReporter::EnterTestFixtureSection(const std::string& testFixtureName)
	{
		if (testFixtureName.empty())
		{
			return RC_INVALID_ARGS;
		}

		if (!mActiveTestFixtureName.empty())
		{
			TDE2_ASSERT(false); /// \note Non-empty field means that we've met unbalanced calls of Enter/ExitFixtureScetion
			return RC_FAIL;
		}

		mActiveTestFixtureName = testFixtureName;

		LogMessage(mpFileSystem->Get<ITextFileWriter>(mFileHandler), Wrench::StringUtils::Format("\n* \"{0}\" Test Fixture:", mActiveTestFixtureName));
		
		return RC_OK;
	}

	E_RESULT_CODE CTestResultsTextReporter::ExitTestFixtureSection()
	{
		if (mActiveTestFixtureName.empty())
		{
			TDE2_ASSERT(false); /// \note The empty field means that someone tries to call ExitTestFixtureSection without calling EnterTestFixtureSection
			return RC_FAIL;
		}

		mActiveTestFixtureName = Wrench::StringUtils::GetEmptyStr();

		return RC_OK;
	}

	void CTestResultsTextReporter::WriteTestResult(const std::string& testCaseName, const TTestResultEntity& testResult)
	{
		++mTotalTestsCount;

		LogMessage(
			mpFileSystem->Get<ITextFileWriter>(mFileHandler), 
			Wrench::StringUtils::Format("\t* {0} .......................................... {1}", testCaseName, testResult.mHasPassed ? "OK" : "FAILED"));

		if (!testResult.mHasPassed)
		{
			LogMessage(mpFileSystem->Get<ITextFileWriter>(mFileHandler), 
				Wrench::StringUtils::Format("Test case's failed:\nMessage: {0}\nat file: {1}:{2}\n", testResult.mMessage, testResult.mFilename, testResult.mLine));
		}
	}

	E_RESULT_CODE CTestResultsTextReporter::_onFreeInternal()
	{
		if (auto pFile = mpFileSystem->Get<ITextFileWriter>(mFileHandler))
		{
			return pFile->Close();
		}

		return RC_FAIL;
	}


	TDE2_API ITestResultsReporter* CreateTestResultsTextReporter(TPtr<IFileSystem> pFileSystem, const std::string& filename, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ITestResultsReporter, CTestResultsTextReporter, result, pFileSystem, filename);
	}
}

#endif