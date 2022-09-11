/*!
	/file CTestResultsTextReporter.h
	/date 10.09.2022
	/authors Kasimov Ildar
*/

#pragma once


#include "ITestResultsReporter.h"
#include "../core/CBaseObject.h"


#if TDE2_EDITORS_ENABLED


namespace TDEngine2
{
	enum class TFileEntryId :U32;


	/*!
		\brief A factory function for creation objects of CTestResultsTextReporter's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CTestResultsTextReporter's implementation
	*/

	TDE2_API ITestResultsReporter* CreateTestResultsTextReporter(TPtr<IFileSystem> pFileSystem, const std::string& filename, E_RESULT_CODE& result);


	/*!
		class CTestResultsTextReporter

		\brief The implementation is a simple text formatter of auto-tests results
	*/

	class CTestResultsTextReporter : public CBaseObject, public ITestResultsReporter
	{
		public:
			friend TDE2_API ITestResultsReporter* CreateTestResultsTextReporter(TPtr<IFileSystem>, const std::string&, E_RESULT_CODE&);
		public:
			/*!
				\brief The method initializes an initial state of an object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(TPtr<IFileSystem> pFileSystem, const std::string& filename) override;

			TDE2_API E_RESULT_CODE EnterTestFixtureSection(const std::string& testFixtureName) override;
			TDE2_API E_RESULT_CODE ExitTestFixtureSection() override;

			TDE2_API void WriteTestResult(const std::string& testCaseName, const TTestResultEntity& testResult) override;

		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CTestResultsTextReporter)

			TDE2_API E_RESULT_CODE _onFreeInternal() override;
		protected:
			TPtr<IFileSystem> mpFileSystem;

			TFileEntryId mFileHandler;

			std::string mActiveTestFixtureName;

			U32 mTotalTestsCount = 0;
	};
}

#endif