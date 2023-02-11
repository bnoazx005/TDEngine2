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
	struct TVector3;

	enum class E_KEYCODES : U16;


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
			TDE2_API virtual void WaitForNextFrame() = 0;
			TDE2_API virtual void WaitForCondition(const std::function<bool()>& predicate = nullptr) = 0;

			TDE2_API virtual void SetCursorPosition(const TVector3& position) = 0;
			TDE2_API virtual void AddPressKey(E_KEYCODES keyCode) = 0;
			TDE2_API virtual void AddPressMouseButton(U8 buttonId) = 0;

			TDE2_API virtual void Update(F32 dt) = 0;

			TDE2_API virtual bool IsStarted() const = 0;

			TDE2_API virtual bool IsFinished() const = 0;

			TDE2_API virtual void SetInfo(const std::string& fixtureName, const std::string& name) = 0;
			TDE2_API virtual const std::string& GetOwningFixtureName() const = 0;
			TDE2_API virtual const std::string& GetName() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ITestCase)
	};
}