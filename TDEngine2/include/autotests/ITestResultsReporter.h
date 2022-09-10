/*!
	/file ITestResultsReporter.h
	/date 10.09.2022
	/authors Kasimov Ildar
*/

#pragma once


#include "../utils/Types.h"
#include "../core/IBaseObject.h"
#include <string>

namespace TDEngine2
{
	class IFileSystem;
	struct TTestResultEntity;


	TDE2_DECLARE_SCOPED_PTR(IFileSystem)


	/*!
		interface ITestReporter

		\brief The interface describes a functionality for output results of automatic tests
	*/

	class ITestResultsReporter : public virtual IBaseObject
	{
		public:
			/*!
				\brief The method initializes an initial state of an object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(TPtr<IFileSystem> pFileSystem, const std::string& filename) = 0;

			TDE2_API virtual void WriteTestResult(const TTestResultEntity& testResult) = 0;

		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ITestResultsReporter)
	};


	TDE2_DECLARE_SCOPED_PTR(ITestResultsReporter)
}