#include "../../include/autotests/CTestResultsTextReporter.h"
#include "../../include/core/IFileSystem.h"
#include "../../include/core/IFile.h"


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
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

		mIsInitialized = true;

		return RC_OK;
	}

	void CTestResultsTextReporter::WriteTestResult(const TTestResultEntity& testResult)
	{
		if (auto pFile = mpFileSystem->Get<ITextFileReader>(mFileHandler))
		{
			pFile->
		}
	}

	E_RESULT_CODE CTestResultsTextReporter::_onFreeInternal()
	{
		if (auto pFile = mpFileSystem->Get<ITextFileReader>(mFileHandler))
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