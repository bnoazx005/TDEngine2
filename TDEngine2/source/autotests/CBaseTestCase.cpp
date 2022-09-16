#include "../../include/autotests/CBaseTestCase.h"
#include "../../include/autotests/CTestContext.h"
#include <functional>


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
			typedef std::function<void()> TActionFunctor;
		public:
			TDE2_API CFunctionalAction(const TActionFunctor& action = nullptr):
				mAction(action)
			{
			}

			TDE2_API ~CFunctionalAction() override
			{
			}

			TDE2_API void Execute() override
			{
				if (mAction)
				{
					mAction();
				}

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
			TActionFunctor mAction = nullptr;
			bool mIsFinished = false;
	};


	class CWaitAction: public ITestAction
	{
		public:
			TDE2_API CWaitAction(F32 delay) :
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
			TDE2_API CSetCursorPositionAction(const TVector3& mousePosition):
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
		mActions.emplace_back(std::make_unique<CFunctionalAction>(action));
	}

	void CBaseTestCase::Wait(F32 delay)
	{
		mActions.emplace_back(std::make_unique<CWaitAction>(delay));
	}

	void CBaseTestCase::WaitForNextFrame()
	{
		mActions.emplace_back(std::make_unique<CWaitForNextFrameAction>());
	}

	void CBaseTestCase::SetCursorPosition(const TVector3& position)
	{
		mActions.emplace_back(std::make_unique<CSetCursorPositionAction>(position));
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

	TDE2_API ITestCase* CreateBaseTestCase(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ITestCase, CBaseTestCase, result);
	}
}