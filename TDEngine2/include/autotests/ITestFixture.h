/*!
	/file ITestFixture.h
	/date 04.09.2022
	/authors Kasimov Ildar
*/

#pragma once


#include "../utils/Types.h"
#include "../core/IBaseObject.h"


namespace TDEngine2
{
	class ITestCase;


	TDE2_DECLARE_SCOPED_PTR(ITestCase)


	/*!
		interface ITestFixture

		\brief The interface describes a functionality of a set of in-engine automatic tests
	*/

	class ITestFixture : public virtual IBaseObject
	{
		public:
			/*!
				\brief The method initializes an initial state of an object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(const std::string& name) = 0;

			TDE2_API virtual E_RESULT_CODE AddTestCase(const std::string& name, TPtr<ITestCase> pTestCase) = 0;

			TDE2_API virtual void Update(F32 dt) = 0;

			TDE2_API virtual bool IsFinished() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ITestFixture)
	};


	TDE2_DECLARE_SCOPED_PTR(ITestFixture)
}