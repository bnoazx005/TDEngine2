/*!
	/file CBaseTestCase.h
	/date 04.09.2022
	/authors Kasimov Ildar
*/

#pragma once


#include "ITestCase.h"
#include "../core/CBaseObject.h"


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CBaseTestCase's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CBaseTestCase's implementation
	*/

	TDE2_API ITestCase* CreateBaseTestCase(E_RESULT_CODE& result);


	/*!
		class CBaseTestCase

		\brief The implementation of the test case is an updatable object which executes commands and rejects them when they're completed
	*/

	class CBaseTestCase : public CBaseObject, public ITestCase
	{
		public:
			friend TDE2_API ITestCase* CreateBaseTestCase(E_RESULT_CODE&);
		public:
			/*!
				\brief The method initializes an initial state of an object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init() override;

			TDE2_API void ExecuteAction(const std::function<void()>& action = nullptr) override;

			TDE2_API void Wait(F32 delay) override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseTestCase)
		protected:
	};
}