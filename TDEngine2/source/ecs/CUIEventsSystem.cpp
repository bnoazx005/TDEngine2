#include "../../include/ecs/CUIEventsSystem.h"
#include "../../include/ecs/IWorld.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/ecs/CTransform.h"
#include "../../include/graphics/UI/CInputReceiverComponent.h"
#include "../../include/graphics/UI/CLayoutElementComponent.h"
#include "../../include/graphics/UI/CCanvasComponent.h"
#include "../../include/core/IInputContext.h"
#include "../../include/core/IImGUIContext.h"
#include "../../include/editor/CPerfProfiler.h"
#include <queue>
#include <unordered_set>


namespace TDEngine2
{
	CUIEventsSystem::CUIEventsSystem() :
		CBaseSystem()
	{
	}

	E_RESULT_CODE CUIEventsSystem::Init(IInputContext* pInputContext, IImGUIContext* pImGUIContext)
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

		mpImGUIContext = pImGUIContext;

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
		std::vector<CCanvas*> canvases;

		auto insertNewCanvas = [&](CCanvas* pCanvas, TEntityId entityId)
		{
			auto it = std::find_if(canvases.begin(), canvases.end(), [pCanvas](CCanvas* pCurrCanvas) { return pCanvas->GetPriority() > pCurrCanvas->GetPriority(); });
			it = canvases.insert(it, pCanvas);

			output.insert(output.begin() + std::distance(canvases.begin(), it), entityId);
		};

		for (TEntityId currEntityId : pWorld->FindEntitiesWithComponents<CCanvas>())
		{
			CEntity* pEntity = pWorld->FindEntity(currEntityId);
			CCanvas* pCanvas = pEntity->GetComponent<CCanvas>();

			if (CTransform* pTransform = pEntity->GetComponent<CTransform>())
			{
				if (TEntityId::Invalid == pTransform->GetParent())
				{
					insertNewCanvas(pCanvas, currEntityId);
				}

				if (CEntity* pParentEntity = pWorld->FindEntity(pTransform->GetParent()))
				{
					if (!pParentEntity->HasComponent<CLayoutElement>())
					{
						insertNewCanvas(pCanvas, currEntityId);
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
		auto& priorities     = mContext.mPriorities;
		auto& canvases       = mContext.mCanvasesRanges;

		transforms.clear();
		layoutElements.clear();
		inputReceivers.clear();
		priorities.clear();
		canvases.clear();

		/// \note Find main canvas which has no parent or its parent has no CLayoutElement component attached
		for (TEntityId currCanvasEntity : FindMainCanvases(pWorld))
		{
			CTransform* pTransform = pWorld->FindEntity(currCanvasEntity)->GetComponent<CTransform>();

			std::queue<std::tuple<TEntityId, U32>> entitiesToVisit;

			entitiesToVisit.emplace(currCanvasEntity, 0);

			CEntity* pEntity = nullptr;
			TEntityId currEntityId = TEntityId::Invalid;
			U32 currPriority = 0;

			USIZE canvasStartRange = transforms.size();

			while (!entitiesToVisit.empty())
			{
				std::tie(currEntityId, currPriority) = entitiesToVisit.front();
				entitiesToVisit.pop();

				pEntity = pWorld->FindEntity(currEntityId);
				if (pEntity->HasComponent<CDeactivatedComponent>() || pEntity->HasComponent<CDeactivatedGroupComponent>())
				{
					continue;
				}

				auto pInputReceiver = pEntity->GetComponent<CInputReceiver>();

				if (pInputReceiver || pEntity->HasComponent<CUIMaskComponent>())
				{
					transforms.push_back(pEntity->GetComponent<CTransform>());
					layoutElements.push_back(pEntity->GetComponent<CLayoutElement>());
					priorities.push_back(currPriority);
					inputReceivers.push_back(pInputReceiver);
				}

				if (pTransform = pEntity->GetComponent<CTransform>())
				{
					for (TEntityId id : pTransform->GetChildren())
					{
						entitiesToVisit.emplace(id, currPriority + 1);
					}
				}
			}

			canvases.emplace_back(canvasStartRange, transforms.size());
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


	// The method returns true if the input was consumed by some of internal canvas element
	static bool ProcessCanvasInput(IWorld* pWorld, CUIEventsSystem::TSystemContext& context, IDesktopInputContext* pInputContext, 
		const TRange<USIZE>& canvasRange, bool isImGUIActive, std::string& inputBuffer, TEntityId& currFocusedInputEntity)
	{
		auto& transforms = context.mpTransforms;
		auto& layoutElements = context.mpLayoutElements;
		auto& inputReceivers = context.mpInputReceivers;
		auto& priorities = context.mPriorities;

		CInputReceiver* pInputReceiver = nullptr;
		CLayoutElement* pLayoutElement = nullptr;
		CTransform* pTransform = nullptr;

		const TVector2 mousePosition = static_cast<TVector2>(pInputContext->GetMousePosition());

		CEntity* pCurrEntity = nullptr;

		for (USIZE i = canvasRange.mLeft; i < canvasRange.mRight; i++) // \note Elements processed in bottom-up order
		{
			const USIZE index = canvasRange.mRight - 1 - i;

			pTransform     = transforms[index];
			pInputReceiver = inputReceivers[index];
			pLayoutElement = layoutElements[index];

			if (!pInputReceiver || (pInputReceiver && pInputReceiver->mIsIgnoreInput))
			{
				continue;
			}

			pCurrEntity = pWorld->FindEntity(pTransform->GetOwnerId());
			if (!pCurrEntity || pCurrEntity->HasComponent<CDeactivatedComponent>() || pCurrEntity->HasComponent<CDeactivatedGroupComponent>())
			{
				continue;
			}

			if (pCurrEntity->HasComponent<CUIMaskComponent>() && !pInputReceiver)
			{
				continue;
			}

			/// \fixme For now it's the simplest solution for checking buttons 
			pInputReceiver->mIsHovered = ContainsPoint(IntersectRects(pLayoutElement->GetWorldRect(), pLayoutElement->GetParentWorldRect()), mousePosition);

			pInputReceiver->mCurrState = pInputReceiver->mIsHovered && pInputContext->IsMouseButton(0);

			pInputReceiver->mIsControlModifierActive = pInputContext->IsKey(E_KEYCODES::KC_LCONTROL) || pInputContext->IsKey(E_KEYCODES::KC_RCONTROL);
			pInputReceiver->mIsShiftModifierActive = pInputContext->IsKey(E_KEYCODES::KC_LSHIFT) || pInputContext->IsKey(E_KEYCODES::KC_RSHIFT);
			pInputReceiver->mActionType = GetActionType(pInputContext);

			pInputReceiver->mMouseShiftVec = pInputContext->GetMouseShiftVec();

			/// focus/unfocus logic
			{
				if ((TEntityId::Invalid == currFocusedInputEntity) && pInputContext->IsMouseButtonPressed(0) && pInputReceiver->mIsHovered && !pInputReceiver->mIsFocused)
				{
					pInputReceiver->mIsFocused = true;
					currFocusedInputEntity = pTransform->GetOwnerId();
				}

				if (pInputReceiver->mIsFocused)
				{
					pInputReceiver->mInputBuffer = inputBuffer;
				}
			}

			pInputReceiver->mNormalizedInputPosition = PointToNormalizedCoords(pLayoutElement->GetWorldRect(), mousePosition);

			if (/*!pInputReceiver->mIsInputBypassEnabled && */pInputReceiver->mCurrState || pInputReceiver->mIsHovered)
			{
				return true;
			}
		}

		return false;
	}


	static void ResetInputReceivers(std::vector<CInputReceiver*>& inputReceivers)
	{
		for (USIZE i = 0; i < inputReceivers.size(); i++)
		{
			CInputReceiver* pInputReceiver = inputReceivers[i];
			if (!pInputReceiver)
			{
				continue;
			}

			pInputReceiver->mIsHovered = false;

			pInputReceiver->mPrevState = pInputReceiver->mCurrState; // reset state
			pInputReceiver->mCurrState = false;
		}
	}


	void CUIEventsSystem::Update(IWorld* pWorld, F32 dt)
	{
		TDE2_PROFILER_SCOPE("CUIEventsSystem::Update");

		ResetInputReceivers(mContext.mpInputReceivers);

		for (auto&& currCanvasEntities : mContext.mCanvasesRanges)
		{
			if (ProcessCanvasInput(pWorld, mContext, mpDesktopInputContext, currCanvasEntities, mpImGUIContext && mpImGUIContext->IsMouseOverUI(),
				mInputBuffer, mCurrFocusedInputEntity))
			{
				break;
			}
		}

		// \note unfocus logic
		// \todo refactor this later
		const bool isCancelAction = mpDesktopInputContext->IsKeyPressed(E_KEYCODES::KC_ESCAPE);

		if (auto pEntity = pWorld->FindEntity(mCurrFocusedInputEntity))
		{
			if (auto pInputReceiver = pEntity->GetComponent<CInputReceiver>())
			{
				if (!pInputReceiver->mIsHovered && (mpDesktopInputContext->IsMouseButtonPressed(0) || isCancelAction) || mpDesktopInputContext->IsKeyPressed(E_KEYCODES::KC_RETURN))
				{
					pInputReceiver->mIsFocused = false;
					mCurrFocusedInputEntity = TEntityId::Invalid;
					mInputBuffer.clear();
				}
			}
		}
		else
		{
			mCurrFocusedInputEntity = TEntityId::Invalid;
		}
	}


	TDE2_API ISystem* CreateUIEventsSystem(IInputContext* pInputContext, IImGUIContext* pImGUIContext, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem, CUIEventsSystem, result, pInputContext, pImGUIContext);
	}
}