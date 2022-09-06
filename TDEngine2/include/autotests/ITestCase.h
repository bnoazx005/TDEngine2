/*!
	/file ITestCase.h
	/date 04.09.2022
	/authors Kasimov Ildar
*/

#pragma once


#include "../utils/Types.h"
#include "../core/IBaseObject.h"
#include <functional>


namespace TDEngine2
{
	/*!
		interface ITestCase

		\brief The interface describes a functionality of a single integration test scenario
	*/

	class ITestCase : public virtual IBaseObject
	{
		public:
			/*!
				\brief The method initializes an initial state of an object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init() = 0;

			TDE2_API virtual void ExecuteAction(const std::function<void()>& action = nullptr) = 0;

			TDE2_API virtual void Wait(F32 delay) = 0;

			TDE2_API virtual void Update(F32 dt) = 0;

			TDE2_API virtual bool IsFinished() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ITestCase)
	};
}