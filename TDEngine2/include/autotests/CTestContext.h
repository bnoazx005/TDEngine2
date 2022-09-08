/*!
	\file CTestContext.h
	\date 03.09.2022
	\authors Kasimov Ildar
*/

#pragma once


#include "../core/CBaseObject.h"
#include <unordered_map>
#include <vector>
#include <string>


#if TDE2_EDITORS_ENABLED


namespace TDEngine2
{
	class IFileSystem;
	class ITestFixture;


	TDE2_DECLARE_SCOPED_PTR(IFileSystem);
	TDE2_DECLARE_SCOPED_PTR(ITestFixture);


	typedef struct TTestContextConfig
	{
		TPtr<IFileSystem> mpFileSystem;
		std::string       mTestReportFilePath = "TestResults.txt";
	} TTestContextConfig, *TTestContextConfigPtr;


	/*!
		class CTestContext

		\brief The type is a core point for all autotesting system. The API is used to define test fixtures and
		to run their execution
	*/

	class CTestContext: public CBaseObject
	{
		public:
			typedef std::unordered_map<std::string, bool> TTestResultsTable;
		public:
			TDE2_API E_RESULT_CODE Init(const TTestContextConfig& config);

			TDE2_API E_RESULT_CODE AddTestFixture(TPtr<ITestFixture> pFixture);

			TDE2_API E_RESULT_CODE RunAllTests();

			TDE2_API void AddTestResult(const std::string& testFixtureName, const std::string& testCaseName, bool hasPassed);
			
			TDE2_API E_RESULT_CODE Assert(bool actual, bool expected);

			TDE2_API void Update(F32 dt);

			/*!
				\brief The function is replacement of factory method for instances of this type.
				The only instance will be created per program's lifetime.

				\return A pointer to an instance of IProfiler type
			*/

			TDE2_API static TPtr<CTestContext> Get();
		private:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CTestContext);

		private:
			TPtr<IFileSystem>                                  mpFileSystem;

			std::unordered_map<std::string, TTestResultsTable> mTestResults; ///< The key is a test fixture's name, the table contains test cases execution's results
			
			std::vector<TPtr<ITestFixture>>                    mTestFixtures;

			bool                                               mIsRunning = false;

			U32                                                mTotalTestsCount = 0;
			U32                                                mPassedTestsCount = 0;
			U32                                                mFailedTestsCount = 0;
	};
}

#endif