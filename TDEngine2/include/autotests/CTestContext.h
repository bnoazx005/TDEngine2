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
	class IEngineCore;
	class ITestResultReporter;


	TDE2_DECLARE_SCOPED_PTR(IFileSystem);
	TDE2_DECLARE_SCOPED_PTR(ITestFixture);
	TDE2_DECLARE_SCOPED_PTR(ITestResultReporter);


	typedef struct TTestContextConfig
	{
		IEngineCore* mpEngineCore;
	} TTestContextConfig, *TTestContextConfigPtr;


#if TDE2_DEBUG_MODE
	#define TDE2_TEST_IS_TRUE(...) do { TDEngine2::CTestContext::Get()->Assert(TDE2_STRINGIFY(__VA_ARGS__), __VA_ARGS__, true, __FILE__, __LINE__); } while (false)
	#define TDE2_TEST_IS_FALSE(...) do { TDEngine2::CTestContext::Get()->Assert(TDE2_STRINGIFY(__VA_ARGS__), __VA_ARGS__, false, __FILE__, __LINE__); } while (false)

	#define TDE2_TEST_FIXTURE(Name)																										\
		static void TestFixtureBody(TPtr<ITestFixture> pTestFixture);																	\
		struct TTestFixtureEnvironment																									\
		{																																\
			TDE2_API TTestFixtureEnvironment()																							\
			{																															\
				E_RESULT_CODE result = RC_OK;																							\
				TPtr<ITestFixture> pTestFixtureInstance = TPtr<ITestFixture>(CreateBaseTestFixture(Name, result));						\
				TestFixtureBody(pTestFixtureInstance);																					\
				CTestContext::Get()->AddTestFixture(pTestFixtureInstance);																\
			}																															\
																																		\
		};																																\
		static TTestFixtureEnvironment registerTestFixture;																				\
		static void TestFixtureBody(TPtr<ITestFixture> pTestFixture)

	
	#define TDE2_TEST_CASE_IMPL(Name, TestCaseVariableName, ResultVariableName)								\
		E_RESULT_CODE ResultVariableName = RC_OK;															\
		TPtr<ITestCase> TestCaseVariableName = TPtr<ITestCase>(CreateBaseTestCase(ResultVariableName));		\
		pTestFixture->AddTestCase(Name, TestCaseVariableName);												\
		if (auto pTestCase = TestCaseVariableName)

	#define TDE2_TEST_CASE(Name) TDE2_TEST_CASE_IMPL(Name, TDE2_CONCAT(pTestCase, __COUNTER__), TDE2_CONCAT(result, __COUNTER__))


#else
	#define TDE2_TEST_IS_TRUE(...) (void)0
	#define TDE2_TEST_IS_FALSE(...) (void)0

	#define TDE2_TEST_FIXTURE(Name) static void TestFixtureBody(TPtr<ITestFixture> pTestFixture)
	#define TDE2_TEST_CASE(Name) if (false)
#endif


	/*!
		class CTestContext

		\brief The type is a core point for all autotesting system. The API is used to define test fixtures and
		to run their execution
	*/

	class CTestContext: public CBaseObject
	{
		public:
			struct TTestResultEntity
			{
				std::string mMessage;
				std::string mFilename;
				U32         mLine = 0;
				bool        mHasPassed = false;
			};

			typedef std::unordered_map<std::string, TTestResultEntity> TTestResultsTable;
		public:
			TDE2_API E_RESULT_CODE Init(const TTestContextConfig& config);

			TDE2_API E_RESULT_CODE AddTestFixture(TPtr<ITestFixture> pFixture);

			TDE2_API E_RESULT_CODE RunAllTests();

			TDE2_API void AddTestResult(const std::string& testFixtureName, const std::string& testCaseName, const TTestResultEntity& result);
			
			TDE2_API E_RESULT_CODE WriteTestResults(TPtr<ITestResultReporter> pReporter);

			TDE2_API E_RESULT_CODE Assert(const std::string& message, bool actual, bool expected, const std::string& filename = "", U32 line = 0);

			TDE2_API void Update(F32 dt);

			/*!
				\brief The function is replacement of factory method for instances of this type.
				The only instance will be created per program's lifetime.

				\return A pointer to an instance of IProfiler type
			*/

			TDE2_API static TPtr<CTestContext> Get();

			TDE2_API IEngineCore* GetEngineCore() const;
		private:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CTestContext);

		private:
			IEngineCore*                                       mpEngineCore;

			std::unordered_map<std::string, TTestResultsTable> mTestResults; ///< The key is a test fixture's name, the table contains test cases execution's results
			
			std::vector<TPtr<ITestFixture>>                    mTestFixtures;

			bool                                               mIsRunning = false;

			U32                                                mTotalTestsCount = 0;
			U32                                                mPassedTestsCount = 0;
			U32                                                mFailedTestsCount = 0;
	};
}

#endif