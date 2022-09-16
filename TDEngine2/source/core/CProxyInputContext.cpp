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

		return RC_OK;
	}

	bool CProxyInputContext::IsKeyPressed(E_KEYCODES keyCode)
	{
		return false;
	}

	bool CProxyInputContext::IsKey(E_KEYCODES keyCode)
	{
		return false;
	}

	bool CProxyInputContext::IsKeyUnpressed(E_KEYCODES keyCode)
	{
		return false;
	}

	bool CProxyInputContext::IsMouseButtonPressed(U8 button)
	{
		return false;
	}

	bool CProxyInputContext::IsMouseButton(U8 button)
	{
		return false;
	}

	bool CProxyInputContext::IsMouseButtonUnpressed(U8 button)
	{
		return false;
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
		return ZeroVector3;
	}

	TVector2 CProxyInputContext::GetNormalizedMousePosition() const
	{
		return ZeroVector2;
	}

	TVector3 CProxyInputContext::GetMouseShiftVec() const
	{
		return ZeroVector3;
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


	TDE2_API IInputContext* CreateProxyInputContext(const TProxyInputContextDesc& desc, TPtr<IWindowSystem> pWindowSystem, E_RESULT_CODE& result)
	{
		return ::TDEngine2::CreateImpl<IInputContext, CProxyInputContext>(
			[&desc] 
			{ 
				auto pPtr = new(std::nothrow) CProxyInputContext(); 
				TDE2_REGISTER_BASE_OBJECT(CProxyInputContext, pPtr); 

				pPtr->mpMousePosition = desc.mpMousePosition;
				
				return pPtr; 
			},
			[](CProxyInputContext*& pPtr) { TDE2_UNREGISTER_BASE_OBJECT(pPtr); delete pPtr; }, result, pWindowSystem);
	}
}