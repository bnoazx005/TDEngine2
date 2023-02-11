#include "../../include/autotests/CBaseTestCase.h"
#include "../../include/autotests/CTestContext.h"
#include <functional>


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	class ITestAction
	{
		public:
			TDE2_API virtual ~ITestAction() = default;

			TDE2_API virtual void Execute() = 0;
			TDE2_API virtual void Update(F32 dt) = 0;

			TDE2_API virtual bool IsFinished() const = 0;
	};


	class CFunctionalAction: public ITestAction
	{
		public:
			typedef std::function<bool()> TPredicate;
		public:
			TDE2_API explicit CFunctionalAction(const TPredicate& action = nullptr):
				mAction(action)
			{
			}

			TDE2_API ~CFunctionalAction() override
			{
			}

			TDE2_API void Execute() override
			{
				mIsFinished = mAction ? mAction() : true;
			}

			TDE2_API void Update(F32 dt) override
			{
				Execute();
			}

			TDE2_API bool IsFinished() const override
			{
				return mIsFinished;
			}
		private:
			TPredicate mAction = nullptr;
			bool mIsFinished = false;
	};


	class CWaitAction: public ITestAction
	{
		public:
			TDE2_API explicit CWaitAction(F32 delay) :
				mCurrTime(0.0f), mDelay(delay)
			{
			}

			TDE2_API ~CWaitAction() override
			{
			}

			TDE2_API void Execute() override
			{
			}

			TDE2_API void Update(F32 dt) override
			{
				if (mCurrTime > mDelay)
				{
					mIsFinished = true;
					return;
				}

				mCurrTime += dt;
			}

			TDE2_API bool IsFinished() const override
			{
				return mIsFinished;
			}
		private:
			F32  mCurrTime = 0.0f;
			F32  mDelay = 0.0f;

			bool mIsFinished = false;
	};


	class CWaitForNextFrameAction : public ITestAction
	{
		public:
			TDE2_API CWaitForNextFrameAction()
			{
			}

			TDE2_API ~CWaitForNextFrameAction() override
			{
			}

			TDE2_API void Execute() override
			{
			}

			TDE2_API void Update(F32 dt) override
			{
				mIsFinished = mCounter;
				++mCounter;
			}

			TDE2_API bool IsFinished() const override
			{
				return mIsFinished;
			}
		private:
			U32 mCounter = 0;

			bool mIsFinished = false;
	};


	class CSetCursorPositionAction : public ITestAction
	{
		public:
			TDE2_API explicit CSetCursorPositionAction(const TVector3& mousePosition):
				mMousePosition(mousePosition)
			{
			}

			TDE2_API ~CSetCursorPositionAction() override
			{
			}

			TDE2_API void Execute() override
			{
				CTestContext::Get()->SetMousePosition(mMousePosition);
				mIsFinished = true;
			}

			TDE2_API void Update(F32 dt) override
			{
				Execute();
			}

			TDE2_API bool IsFinished() const override
			{
				return mIsFinished;
			}
		private:
			TVector3 mMousePosition;
			bool mIsFinished = false;
	};


	class COnKeyPressAction : public ITestAction
	{
		public:
			TDE2_API explicit COnKeyPressAction(E_KEYCODES keyCode) :
				mKeyCode(keyCode)
			{
			}

			TDE2_API ~COnKeyPressAction() override
			{
			}

			TDE2_API void Execute() override
			{
				CTestContext::Get()->NotifyOnKeyPressEvent(mKeyCode);
				mIsFinished = true;
			}

			TDE2_API void Update(F32 dt) override
			{
				Execute();
			}

			TDE2_API bool IsFinished() const override
			{
				return mIsFinished;
			}
		private:
			E_KEYCODES mKeyCode;
			bool mIsFinished = false;
	};


	class COnMouseButtonPressAction : public ITestAction
	{
		public:
			TDE2_API explicit COnMouseButtonPressAction(U8 buttonId) :
				mButtonId(buttonId)
			{
			}

			TDE2_API ~COnMouseButtonPressAction() override
			{
			}

			TDE2_API void Execute() override
			{
				CTestContext::Get()->NotifyOnMouseButtonPressEvent(mButtonId);
				mIsFinished = true;
			}

			TDE2_API void Update(F32 dt) override
			{
				Execute();
			}

			TDE2_API bool IsFinished() const override
			{
				return mIsFinished;
			}
		private:
			U8 mButtonId;
			bool mIsFinished = false;
	};


	/*!
		\brief CBaseTestCase's definition
	*/

	CBaseTestCase::CBaseTestCase() :
		CBaseObject()
	{
	}

	CBaseTestCase::~CBaseTestCase()
	{
	}

	E_RESULT_CODE CBaseTestCase::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mActions.clear();

		mIsStarted = false;

		mIsInitialized = true;

		return RC_OK;
	}

	void CBaseTestCase::ExecuteAction(const std::function<void()>& action)
	{
		mActions.emplace_back(std::make_unique<CFunctionalAction>([action] 
		{ 
			if (action)
			{
				action();
			}

			return true;
		}));
	}

	void CBaseTestCase::Wait(F32 delay)
	{
		mActions.emplace_back(std::make_unique<CWaitAction>(delay));
	}

	void CBaseTestCase::WaitForNextFrame()
	{
		mActions.emplace_back(std::make_unique<CWaitForNextFrameAction>());
	}

	void CBaseTestCase::WaitForCondition(const std::function<bool()>& predicate)
	{
		mActions.emplace_back(std::make_unique<CFunctionalAction>(predicate));
	}

	void CBaseTestCase::SetCursorPosition(const TVector3& position)
	{
		mActions.emplace_back(std::make_unique<CSetCursorPositionAction>(position));
	}

	void CBaseTestCase::AddPressKey(E_KEYCODES keyCode)
	{
		mActions.emplace_back(std::make_unique<COnKeyPressAction>(keyCode));
	}
		
	void CBaseTestCase::AddPressMouseButton(U8 buttonId)
	{
		mActions.emplace_back(std::make_unique<COnMouseButtonPressAction>(buttonId));
	}

	void CBaseTestCase::Update(F32 dt)
	{
		if (mActions.empty())
		{
			return;
		}

		mIsStarted = true;

		ITestAction* pAction = mActions.front().get();
		if (!pAction->IsFinished())
		{
			pAction->Update(dt);
			return;
		}

		mActions.erase(mActions.cbegin());
	}

	bool CBaseTestCase::IsStarted() const
	{
		return mIsStarted;
	}

	bool CBaseTestCase::IsFinished() const
	{
		return mActions.empty();
	}

	void CBaseTestCase::SetInfo(const std::string& fixtureName, const std::string& name)
	{
		mOwningFixtureName = fixtureName;
		mName = name;
	}

	const std::string& CBaseTestCase::GetOwningFixtureName() const
	{
		return mOwningFixtureName;
	}

	const std::string& CBaseTestCase::GetName() const
	{
		return mName;
	}


	TDE2_API ITestCase* CreateBaseTestCase(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ITestCase, CBaseTestCase, result);
	}
}

#endif