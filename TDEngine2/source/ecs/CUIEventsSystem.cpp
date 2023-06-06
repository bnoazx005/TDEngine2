#include "../../include/ecs/CUIEventsSystem.h"
#include "../../include/ecs/IWorld.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/ecs/CTransform.h"
#include "../../include/graphics/UI/CInputReceiverComponent.h"
#include "../../include/graphics/UI/CLayoutElementComponent.h"
#include "../../include/graphics/UI/CCanvasComponent.h"
#include "../../include/core/IInputContext.h"
#include "../../include/editor/CPerfProfiler.h"
#include <stack>
#include <unordered_set>


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

		mpDesktopInputContext->SetOnCharInputCallback([this](TUtf8CodePoint codePoint)
		{
			if (static_cast<C8>(codePoint) == '\b')
			{
				return;
			}

			mInputBuffer = CU8String::UTF8CodePointToString(codePoint);
		});

		mIsInitialized = true;

		return RC_OK;
	}


	TDE2_API std::vector<TEntityId> FindMainCanvases(IWorld* pWorld)
	{
		std::vector<TEntityId> output;

		for (TEntityId currEntityId : pWorld->FindEntitiesWithComponents<CCanvas>())
		{
			CEntity* pEntity = pWorld->FindEntity(currEntityId);

			if (CTransform* pTransform = pEntity->GetComponent<CTransform>())
			{
				if (TEntityId::Invalid == pTransform->GetParent())
				{
					output.push_back(currEntityId);
				}

				if (CEntity* pParentEntity = pWorld->FindEntity(pTransform->GetParent()))
				{
					if (!pParentEntity->HasComponent<CLayoutElement>())
					{
						output.push_back(currEntityId);
					}
				}
			}
		}

		return output;
	}


	void CUIEventsSystem::InjectBindings(IWorld* pWorld)
	{
		auto& transforms     = mContext.mpTransforms;
		auto& layoutElements = mContext.mpLayoutElements;
		auto& inputReceivers = mContext.mpInputReceivers;

		transforms.clear();
		layoutElements.clear();
		inputReceivers.clear();

		/// \note Find main canvas which has no parent or its parent has no CLayoutElement component attached
		for (TEntityId currCanvasEntity : FindMainCanvases(pWorld))
		{
			CTransform* pTransform = pWorld->FindEntity(currCanvasEntity)->GetComponent<CTransform>();

			/// \note Sort all entities based on computed priority (children're first)
			std::stack<TEntityId> entitiesToVisit;
			std::unordered_set<TEntityId> visitedEntities;

			for (TEntityId id : pTransform->GetChildren())
			{
				entitiesToVisit.push(id);
			}

			CEntity* pEntity = nullptr;

			while (!entitiesToVisit.empty())
			{
				const TEntityId currEntityId = entitiesToVisit.top();

				pEntity = pWorld->FindEntity(currEntityId);
				pTransform = pEntity->GetComponent<CTransform>();

				if (!pTransform->GetChildren().empty())
				{
					USIZE prevEntitiesToVisitCount = entitiesToVisit.size();

					for (TEntityId id : pTransform->GetChildren())
					{
						if (visitedEntities.find(id) == visitedEntities.cend())
						{
							entitiesToVisit.push(id);
						}
					}

					if (prevEntitiesToVisitCount != entitiesToVisit.size())
					{
						continue;
					}
				}

				visitedEntities.emplace(currEntityId);
				entitiesToVisit.pop();

				if (auto pInputReceiver = pEntity->GetComponent<CInputReceiver>())
				{
					inputReceivers.push_back(pInputReceiver);
					transforms.push_back(pEntity->GetComponent<CTransform>());
					layoutElements.push_back(pEntity->GetComponent<CLayoutElement>());
				}
			}
		}
	}


	static E_INPUT_ACTIONS GetActionType(IDesktopInputContext* pInputContext)
	{
		if (pInputContext->IsKeyPressed(E_KEYCODES::KC_LEFT)) { return E_INPUT_ACTIONS::MOVE_LEFT; }
		if (pInputContext->IsKeyPressed(E_KEYCODES::KC_RIGHT)) { return E_INPUT_ACTIONS::MOVE_RIGHT; }
		if (pInputContext->IsKeyPressed(E_KEYCODES::KC_DOWN)) { return E_INPUT_ACTIONS::MOVE_DOWN; }
		if (pInputContext->IsKeyPressed(E_KEYCODES::KC_UP)) { return E_INPUT_ACTIONS::MOVE_UP; }
		if (pInputContext->IsKeyPressed(E_KEYCODES::KC_DELETE)) { return E_INPUT_ACTIONS::DELETE_CHAR; }
		if (pInputContext->IsKeyPressed(E_KEYCODES::KC_BACKSPACE)) { return E_INPUT_ACTIONS::BACKSPACE; }
		if (pInputContext->IsKeyPressed(E_KEYCODES::KC_HOME)) { return E_INPUT_ACTIONS::MOVE_HOME; }
		if (pInputContext->IsKeyPressed(E_KEYCODES::KC_END)) { return E_INPUT_ACTIONS::MOVE_END; }
		if (pInputContext->IsKeyPressed(E_KEYCODES::KC_ESCAPE)) { return E_INPUT_ACTIONS::CANCEL_INPUT; }
		
		if (CMathUtils::Abs(pInputContext->GetMouseShiftVec().z) > FloatEpsilon) 
		{
			return E_INPUT_ACTIONS::SCROLL; 
		}

		bool isAlphaCharKeyPressed = false;

		ForEachAlphaNumericKeyCode([pInputContext, &isAlphaCharKeyPressed](E_KEYCODES keyCode)
		{
			isAlphaCharKeyPressed = pInputContext->IsKeyPressed(keyCode);
			return !isAlphaCharKeyPressed;
		});

		if (isAlphaCharKeyPressed)
		{
			return E_INPUT_ACTIONS::CHAR_INPUT;
		}

		return E_INPUT_ACTIONS::NONE;
	}


	void CUIEventsSystem::Update(IWorld* pWorld, F32 dt)
	{
		TDE2_PROFILER_SCOPE("CUIEventsSystem::Update");

		CInputReceiver* pInputReceiver = nullptr;
		CLayoutElement* pLayoutElement = nullptr;
		CTransform* pTransform = nullptr;

		const TVector3 mousePosition3d = mpDesktopInputContext->GetMousePosition();
		const TVector2 mousePosition { mousePosition3d.x, mousePosition3d.y };

		auto& transforms     = mContext.mpTransforms;
		auto& layoutElements = mContext.mpLayoutElements;
		auto& inputReceivers = mContext.mpInputReceivers;

		CEntity* pCurrEntity = nullptr;

		/// \note Update is executed in order of existing hierarchy of elements
		for (USIZE i = 0; i < transforms.size(); ++i)
		{		
			pTransform     = transforms[i];
			pInputReceiver = inputReceivers[i];
			pLayoutElement = layoutElements[i];

			pInputReceiver->mIsHovered = false;

			if (pInputReceiver->mIsIgnoreInput)
			{
				pInputReceiver->mPrevState = pInputReceiver->mCurrState; // reset state
				pInputReceiver->mCurrState = false;

				continue;
			}

			pCurrEntity = pWorld->FindEntity(pTransform->GetOwnerId());
			if (!pCurrEntity || pCurrEntity->HasComponent<CDeactivatedComponent>() || pCurrEntity->HasComponent<CDeactivatedGroupComponent>())
			{
				continue;
			}

			/// \fixme For now it's the simplest solution for checking buttons 
			pInputReceiver->mIsHovered = ContainsPoint(pLayoutElement->GetWorldRect(), mousePosition);
			pInputReceiver->mPrevState = pInputReceiver->mCurrState;
			pInputReceiver->mCurrState = /*pInputReceiver->mIsHovered && */mpDesktopInputContext->IsMouseButton(0);

			pInputReceiver->mIsControlModifierActive = mpDesktopInputContext->IsKey(E_KEYCODES::KC_LCONTROL) || mpDesktopInputContext->IsKey(E_KEYCODES::KC_RCONTROL);
			pInputReceiver->mIsShiftModifierActive = mpDesktopInputContext->IsKey(E_KEYCODES::KC_LSHIFT) || mpDesktopInputContext->IsKey(E_KEYCODES::KC_RSHIFT);
			pInputReceiver->mActionType = GetActionType(mpDesktopInputContext);
			
			pInputReceiver->mMouseShiftVec = mpDesktopInputContext->GetMouseShiftVec();

			/// focus/unfocus logic
			{
				if (mpDesktopInputContext->IsMouseButtonPressed(0) && pInputReceiver->mIsHovered && !pInputReceiver->mIsFocused)
				{
					pInputReceiver->mIsFocused = true;
				}

				if (pInputReceiver->mIsFocused)
				{
					pInputReceiver->mInputBuffer = mInputBuffer;
				}

				const bool isCancelAction = mpDesktopInputContext->IsKeyPressed(E_KEYCODES::KC_ESCAPE);

				if (!pInputReceiver->mIsHovered && (mpDesktopInputContext->IsMouseButtonPressed(0) || isCancelAction) || mpDesktopInputContext->IsKeyPressed(E_KEYCODES::KC_RETURN))
				{
					pInputReceiver->mIsFocused = false;

					mInputBuffer.clear();
				}
			}

			pInputReceiver->mNormalizedInputPosition = PointToNormalizedCoords(pLayoutElement->GetWorldRect(), mousePosition);
			
			if (pInputReceiver->mCurrState && pInputReceiver->mIsHovered)
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