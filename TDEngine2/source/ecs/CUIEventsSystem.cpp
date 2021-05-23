#include "../../include/ecs/CUIEventsSystem.h"
#include "../../include/ecs/IWorld.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/ecs/CTransform.h"
#include "../../include/graphics/UI/CInputReceiverComponent.h"
#include "../../include/graphics/UI/CLayoutElementComponent.h"
#include "../../include/graphics/UI/CCanvasComponent.h"
#include "../../include/core/IInputContext.h"
#include <stack>


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


	static TEntityId FindEntityWithMainCanvas(IWorld* pWorld)
	{
		auto&& canvases = pWorld->FindEntitiesWithComponents<CCanvas>();

		for (TEntityId currEntityId : canvases)
		{
			CEntity* pEntity = pWorld->FindEntity(currEntityId);

			if (CTransform* pTransform = pEntity->GetComponent<CTransform>())
			{
				if (TEntityId::Invalid == pTransform->GetParent())
				{
					return currEntityId;
				}

				if (CEntity* pParentEntity = pWorld->FindEntity(pTransform->GetParent()))
				{
					if (!pParentEntity->HasComponent<CLayoutElement>())
					{
						return currEntityId;
					}
				}
			}
		}

		return TEntityId::Invalid;
	}


	void CUIEventsSystem::InjectBindings(IWorld* pWorld)
	{
		mInputReceivers.clear();

		/// \note Find main canvas which has no parent or its parent has no CLayoutElement component attached
		const TEntityId mainCanvasEntityId = FindEntityWithMainCanvas(pWorld);
		if (TEntityId::Invalid == mainCanvasEntityId)
		{
			return;
		}

		CTransform* pTransform = pWorld->FindEntity(mainCanvasEntityId)->GetComponent<CTransform>();

		/// \note Sort all entities based on computed priority (children're first)
		std::stack<TEntityId> entitiesToVisit;

		for (TEntityId id : pTransform->GetChildren())
		{
			entitiesToVisit.push(id);
		}

		CEntity* pEntity = nullptr;

		while (!entitiesToVisit.empty())
		{
			const TEntityId currEntityId = entitiesToVisit.top();
			entitiesToVisit.pop();

			pEntity = pWorld->FindEntity(currEntityId);

			if (auto pInputReceiver = pEntity->GetComponent<CInputReceiver>())
			{
				if (!pInputReceiver->IsIgnoreInputFlag())
				{
					mInputReceivers.emplace_back(currEntityId);
				}
			}

			if (pTransform = pEntity->GetComponent<CTransform>())
			{
				for (TEntityId id : pTransform->GetChildren())
				{
					entitiesToVisit.push(id);
				}
			}
		}
	}

	void CUIEventsSystem::Update(IWorld* pWorld, F32 dt)
	{
		CEntity* pEntity = nullptr;

		CInputReceiver* pInputReceiver = nullptr;
		CLayoutElement* pLayoutElement = nullptr;
		CTransform* pTransform = nullptr;

		const TVector3 mousePosition3d = mpDesktopInputContext->GetMousePosition();
		const TVector2 mousePosition { mousePosition3d.x, mousePosition3d.y };

		/// \note Update is executed in order of existing hierarchy of elements
		for (auto&& currEntityId : mInputReceivers)
		{
			pEntity = pWorld->FindEntity(currEntityId);
		
			pTransform = pEntity->GetComponent<CTransform>();
			pInputReceiver = pEntity->GetComponent<CInputReceiver>();
			pLayoutElement = pEntity->GetComponent<CLayoutElement>();

			if (pInputReceiver->IsIgnoreInputFlag())
			{
				pInputReceiver->SetPressedFlag(false); // reset state
				continue;
			}

			/// \fixme For now it's the simplest solution for checking buttons 
			pInputReceiver->SetPressedFlag(ContainsPoint(pLayoutElement->GetWorldRect(), mousePosition) && mpDesktopInputContext->IsMouseButton(0));
			if (pInputReceiver->IsPressed())
			{
				return;
			}
		}
	}


	TDE2_API ISystem* CreateUIEventsSystem(IInputContext* pInputContext, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem, CUIEventsSystem, result, pInputContext);
	}
}