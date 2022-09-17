/*!
	/file CBaseTestCase.h
	/date 04.09.2022
	/authors Kasimov Ildar
*/

#pragma once


#include "ITestCase.h"
#include "../core/CBaseObject.h"
#include <vector>
#include <memory>


namespace TDEngine2
{
	class ITestAction;


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
			TDE2_API void WaitForNextFrame() override;
			TDE2_API void WaitForCondition(const std::function<bool()>& predicate = nullptr) override;

			TDE2_API void SetCursorPosition(const TVector3& position) override;
			TDE2_API void AddPressKey(E_KEYCODES keyCode) override;
			TDE2_API void AddPressMouseButton(U8 buttonId) override;

			TDE2_API void Update(F32 dt) override;

			TDE2_API bool IsStarted() const override;

			TDE2_API bool IsFinished() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS_NO_DCTR(CBaseTestCase)
			~CBaseTestCase() override;
		protected:
			std::vector<std::unique_ptr<ITestAction>> mActions;
			bool                                      mIsStarted = false;
	};
}