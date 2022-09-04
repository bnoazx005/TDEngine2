/*!
	/file CBaseTestFixture.h
	/date 04.09.2022
	/authors Kasimov Ildar
*/

#pragma once


#include "ITestFixture.h"
#include "../core/CBaseObject.h"
#include <vector>
#include <string>
#include <tuple>


namespace TDEngine2
{
	class ITestCase;


	TDE2_DECLARE_SCOPED_PTR(ITestCase)


	/*!
		\brief A factory function for creation objects of CBaseTestFixture's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CBaseTestFixture's implementation
	*/

	TDE2_API ITestFixture* CreateBaseTestFixture(const std::string& name, E_RESULT_CODE& result);


	/*!
		class CBaseTestFixture

		\brief The implementation is a simple set of test cases which are executed sequentially
	*/

	class CBaseTestFixture : public CBaseObject, public ITestFixture
	{
		public:
			friend TDE2_API ITestFixture* CreateBaseTestFixture(const std::string&, E_RESULT_CODE&);
		public:
			/*!
				\brief The method initializes an initial state of an object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(const std::string& name) override;

			TDE2_API E_RESULT_CODE AddTestCase(const std::string& name, TPtr<ITestCase> pTestCase) override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseTestFixture)
		protected:
			std::string mName;

			std::vector<std::tuple<std::string, TPtr<ITestCase>>> mTestCases;
	};
}