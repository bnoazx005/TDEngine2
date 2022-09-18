#include "../../include/core/CProxyInputContext.h"
#include "../../include/core/IEventManager.h"
#include "../../include/core/Event.h"
#include "../../include/core/IWindowSystem.h"


namespace TDEngine2
{
	CProxyInputContext::CProxyInputContext() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CProxyInputContext::Init(TPtr<IWindowSystem> pWindowSystem)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pWindowSystem)
		{
			return RC_INVALID_ARGS;
		}

		mpWindowSystem = pWindowSystem;

#if TDE2_EDITORS_ENABLED /// This subscription is needed only with editor mode's enabled
		if (auto pEventManager = pWindowSystem->GetEventManager())
		{
			pEventManager->Subscribe(TOnCharInputEvent::GetTypeId(), this);
		}
#endif

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CProxyInputContext::Update()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		memcpy(mpContextDesc->mPrevKeyboardState, mpContextDesc->mKeyboardState, sizeof(U8) * mpContextDesc->mKeysCount);
		memcpy(mpContextDesc->mPrevMouseButtonsState, mpContextDesc->mMouseButtonsState, sizeof(U8) * mpContextDesc->mMouseButtonsCount);

		mpContextDesc->mPrevMousePosition = mpContextDesc->mMousePosition;

		if (mpContextDesc->mFrameKeysInputBuffer.empty())
		{
			memset(mpContextDesc->mKeyboardState, 0, sizeof(U8) * mpContextDesc->mKeysCount);
		}
		else
		{
			for (auto&& currKeyCode : mpContextDesc->mFrameKeysInputBuffer)
			{
				mpContextDesc->mKeyboardState[static_cast<U16>(currKeyCode)] = 0x1;
			}

			mpContextDesc->mFrameKeysInputBuffer.clear();
		}

		if (mpContextDesc->mFrameMouseButtonsInputBuffer.empty())
		{
			memset(mpContextDesc->mMouseButtonsState, 0, sizeof(U8) * mpContextDesc->mMouseButtonsCount);
		}
		else
		{
			for (auto&& buttonId : mpContextDesc->mFrameMouseButtonsInputBuffer)
			{
				mpContextDesc->mMouseButtonsState[static_cast<U16>(buttonId)] = 0x1;
			}

			mpContextDesc->mFrameMouseButtonsInputBuffer.clear();
		}

		return RC_OK;
	}

	bool CProxyInputContext::IsKeyPressed(E_KEYCODES keyCode)
	{
		const U16 converterKeyCode = static_cast<U16>(keyCode);

		if (converterKeyCode >= mpContextDesc->mKeysCount)
		{
			return false;
		}

		return !mpContextDesc->mPrevKeyboardState[converterKeyCode] && mpContextDesc->mKeyboardState[converterKeyCode];
	}

	bool CProxyInputContext::IsKey(E_KEYCODES keyCode)
	{
		const U16 converterKeyCode = static_cast<U16>(keyCode);

		if (converterKeyCode >= mpContextDesc->mKeysCount)
		{
			return false;
		}

		return mpContextDesc->mKeyboardState[converterKeyCode];
	}

	bool CProxyInputContext::IsKeyUnpressed(E_KEYCODES keyCode)
	{
		const U16 converterKeyCode = static_cast<U16>(keyCode);

		if (converterKeyCode >= mpContextDesc->mKeysCount)
		{
			return false;
		}

		return mpContextDesc->mPrevKeyboardState[converterKeyCode] && !mpContextDesc->mKeyboardState[converterKeyCode];
	}

	bool CProxyInputContext::IsMouseButtonPressed(U8 button)
	{
		if (button >= mpContextDesc->mMouseButtonsCount)
		{
			return false;
		}

		return !mpContextDesc->mPrevMouseButtonsState[button] && mpContextDesc->mMouseButtonsState[button];
	}

	bool CProxyInputContext::IsMouseButton(U8 button)
	{
		if (button >= mpContextDesc->mMouseButtonsCount)
		{
			return false;
		}

		return mpContextDesc->mMouseButtonsState[button];
	}

	bool CProxyInputContext::IsMouseButtonUnpressed(U8 button)
	{
		if (button >= mpContextDesc->mMouseButtonsCount)
		{
			return false;
		}

		return mpContextDesc->mPrevMouseButtonsState[button] && !mpContextDesc->mMouseButtonsState[button];
	}

	E_RESULT_CODE CProxyInputContext::OnEvent(const TBaseEvent* pEvent)
	{
#if TDE2_EDITORS_ENABLED
		if (mOnCharInputCallback)
		{
			if (auto pInputEvent = dynamic_cast<const TOnCharInputEvent*>(pEvent))
			{
				mOnCharInputCallback(static_cast<TUtf8CodePoint>(pInputEvent->mCharCode));
			}
		}
#endif
		return RC_OK;
	}

	TEventListenerId CProxyInputContext::GetListenerId() const
	{
		return static_cast<TEventListenerId>(TDE2_TYPE_ID(CProxyInputContext));
	}

	TVector3 CProxyInputContext::GetMousePosition() const
	{
		return mpContextDesc->mMousePosition;
	}

	TVector2 CProxyInputContext::GetNormalizedMousePosition() const
	{
		const TVector3 pos = GetMousePosition();

		const auto& clientRect = mpWindowSystem->GetClientRect();

		const F32 width = static_cast<F32>(clientRect.width);
		const F32 height = static_cast<F32>(clientRect.height);

		return { CMathUtils::Clamp(-1.0f, 1.0f, 2.0f * pos.x / width - 1.0f), CMathUtils::Clamp(-1.0f, 1.0f, 2.0f * pos.y / height - 1.0f) };
	}

	TVector3 CProxyInputContext::GetMouseShiftVec() const
	{
		return mpContextDesc->mMousePosition - mpContextDesc->mPrevMousePosition;
	}

	E_ENGINE_SUBSYSTEM_TYPE CProxyInputContext::GetType() const
	{
		return EST_INPUT_CONTEXT;
	}

	TPtr<IGamepad> CProxyInputContext::GetGamepad(U8 gamepadId) const
	{
		return nullptr;
	}

#if TDE2_EDITORS_ENABLED

	void CProxyInputContext::SetOnCharInputCallback(const TOnCharActionCallback& onEventAction)
	{
		mOnCharInputCallback = onEventAction;
	}

#endif


	TDE2_API IInputContext* CreateProxyInputContext(TProxyInputContextDesc* pDesc, TPtr<IWindowSystem> pWindowSystem, E_RESULT_CODE& result)
	{
		return ::TDEngine2::CreateImpl<IInputContext, CProxyInputContext>(
			[pDesc] 
			{ 
				auto pPtr = new(std::nothrow) CProxyInputContext(); 
				TDE2_REGISTER_BASE_OBJECT(CProxyInputContext, pPtr); 

				pPtr->mpContextDesc = pDesc;
				
				return pPtr; 
			},
			[](CProxyInputContext*& pPtr) { TDE2_UNREGISTER_BASE_OBJECT(pPtr); delete pPtr; }, result, pWindowSystem);
	}
}