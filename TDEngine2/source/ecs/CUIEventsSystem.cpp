#include "../../include/ecs/CUIEventsSystem.h"
#include "../../include/ecs/IWorld.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/graphics/UI/CInputReceiverComponent.h"
#include "../../include/graphics/UI/CLayoutElementComponent.h"
#include "../../include/core/IInputContext.h"


namespace TDEngine2
{
	CUIEventsSystem::CUIEventsSystem() :
		CBaseSystem()
	{
	}

	E_RESULT_CODE CUIEventsSystem::Init(IInputContext* pInputContext)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pInputContext)
		{
			return RC_INVALID_ARGS;
		}

		mpInputContext = pInputContext;
		mpDesktopInputContext = dynamic_cast<IDesktopInputContext*>(mpInputContext);

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CUIEventsSystem::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		--mRefCounter;

		if (!mRefCounter)
		{
			mIsInitialized = false;
			delete this;
		}

		return RC_OK;
	}

	void CUIEventsSystem::InjectBindings(IWorld* pWorld)
	{
		mInputReceivers = pWorld->FindEntitiesWithComponents<CInputReceiver, CLayoutElement>();
	}

	void CUIEventsSystem::Update(IWorld* pWorld, F32 dt)
	{
		CEntity* pEntity = nullptr;

		CInputReceiver* pInputReceiver = nullptr;
		CLayoutElement* pLayoutElement = nullptr;

		const TVector3 mousePosition3d = mpDesktopInputContext->GetMousePosition();
		const TVector2 mousePosition { mousePosition3d.x, mousePosition3d.y };

		for (auto&& currEntityId : mInputReceivers)
		{
			pEntity = pWorld->FindEntity(currEntityId);
		
			pInputReceiver = pEntity->GetComponent<CInputReceiver>();
			pLayoutElement = pEntity->GetComponent<CLayoutElement>();

			/// \fixme For now it's the simplest solution for checking buttons 
			pInputReceiver->SetPressedFlag(ContainsPoint(pLayoutElement->GetWorldRect(), mousePosition) && mpDesktopInputContext->IsMouseButton(0));
		}
	}


	TDE2_API ISystem* CreateUIEventsSystem(IInputContext* pInputContext, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem, CUIEventsSystem, result, pInputContext);
	}
}