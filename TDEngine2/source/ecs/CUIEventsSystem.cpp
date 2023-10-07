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
		auto& uiMasks        = mContext.mMaskRects;
		auto& canvases       = mContext.mCanvasesRanges;
		auto& parents        = mContext.mParents;

		transforms.clear();
		layoutElements.clear();
		inputReceivers.clear();
		priorities.clear();
		canvases.clear();
		parents.clear();

		/// \note Find main canvas which has no parent or its parent has no CLayoutElement component attached
		for (TEntityId currCanvasEntity : FindMainCanvases(pWorld))
		{
			CTransform* pTransform = pWorld->FindEntity(currCanvasEntity)->GetComponent<CTransform>();

			std::queue<std::tuple<TEntityId, U32, USIZE>> entitiesToVisit; // last arg is a parent's index

			entitiesToVisit.emplace(currCanvasEntity, 0, (std::numeric_limits<USIZE>::max)());

			CEntity* pEntity = nullptr;
			TEntityId currEntityId = TEntityId::Invalid;
			U32 currPriority = 0;

			USIZE canvasStartRange = transforms.size();
			USIZE parentIndex = 0;

			while (!entitiesToVisit.empty())
			{
				std::tie(currEntityId, currPriority, parentIndex) = entitiesToVisit.front();
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
					parents.push_back(parentIndex);
				}

				if (pTransform = pEntity->GetComponent<CTransform>())
				{
					for (TEntityId id : pTransform->GetChildren())
					{
						entitiesToVisit.emplace(id, currPriority + 1, transforms.empty() ? (std::numeric_limits<USIZE>::max)() : transforms.size() - 1);
					}
				}
			}

			canvases.emplace_back(canvasStartRange, transforms.size());
		}

		uiMasks.resize(transforms.size());
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


	static void UpdateUiMasksRects(IWorld* pWorld, CUIEventsSystem::TSystemContext& context, const TRange<USIZE>& canvasRange)
	{
		static const TRectF32 DefaultUiRect = TRectF32(TVector2(-10000.0f), TVector2(10000.0f));

		auto& layoutElements = context.mpLayoutElements;
		auto& uiMasks = context.mMaskRects;
		auto& parents = context.mParents;

		CLayoutElement* pLayoutElement = nullptr;

		CEntity* pCurrEntity = nullptr;

		for (USIZE i = canvasRange.mLeft; i < canvasRange.mRight; i++) 
		{
			pLayoutElement = layoutElements[i];
			
			if (parents[i] == (std::numeric_limits<USIZE>::max)())
			{
				uiMasks[i] = IntersectRects(DefaultUiRect, pLayoutElement->GetWorldRect());
				continue;
			}

			uiMasks[i] = IntersectRects(pLayoutElement->GetWorldRect(), layoutElements[parents[i]]->GetWorldRect());
		}
	}

	// The method returns true if the input was consumed by some of internal canvas element
	static bool ProcessCanvasInput(IWorld* pWorld, CUIEventsSystem::TSystemContext& context, IDesktopInputContext* pInputContext, 
		const TRange<USIZE>& canvasRange, bool isImGUIActive, std::string& inputBuffer)
	{
		auto& transforms = context.mpTransforms;
		auto& layoutElements = context.mpLayoutElements;
		auto& inputReceivers = context.mpInputReceivers;
		auto& priorities = context.mPriorities;
		auto& uiMasks = context.mMaskRects;

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
			pInputReceiver->mIsHovered = ContainsPoint(uiMasks[index], mousePosition);

			pInputReceiver->mCurrState = pInputReceiver->mIsHovered && pInputContext->IsMouseButton(0);

			pInputReceiver->mIsControlModifierActive = pInputContext->IsKey(E_KEYCODES::KC_LCONTROL) || pInputContext->IsKey(E_KEYCODES::KC_RCONTROL);
			pInputReceiver->mIsShiftModifierActive = pInputContext->IsKey(E_KEYCODES::KC_LSHIFT) || pInputContext->IsKey(E_KEYCODES::KC_RSHIFT);
			pInputReceiver->mActionType = GetActionType(pInputContext);

			pInputReceiver->mMouseShiftVec = pInputContext->GetMouseShiftVec();

			/// focus/unfocus logic
			{
				if (pInputContext->IsMouseButtonPressed(0) && pInputReceiver->mIsHovered && !pInputReceiver->mIsFocused)
				{
					pInputReceiver->mIsFocused = true;
				}

				if (pInputReceiver->mIsFocused)
				{
					pInputReceiver->mInputBuffer = inputBuffer;
				}

				const bool isCancelAction = pInputContext->IsKeyPressed(E_KEYCODES::KC_ESCAPE);

				if (!pInputReceiver->mIsHovered && (pInputContext->IsMouseButtonPressed(0) || isCancelAction) || pInputContext->IsKeyPressed(E_KEYCODES::KC_RETURN))
				{
					pInputReceiver->mIsFocused = false;

					inputBuffer.clear();
				}
			}

			pInputReceiver->mNormalizedInputPosition = PointToNormalizedCoords(pLayoutElement->GetWorldRect(), mousePosition);

			if (/*!pInputReceiver->mIsInputBypassEnabled && */pInputReceiver->mCurrState && pInputReceiver->mIsHovered)
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
			UpdateUiMasksRects(pWorld, mContext, currCanvasEntities);

			if (ProcessCanvasInput(pWorld, mContext, mpDesktopInputContext, currCanvasEntities, mpImGUIContext && mpImGUIContext->IsMouseOverUI(), mInputBuffer))
			{
				break;
			}
		}
	}


	TDE2_API ISystem* CreateUIEventsSystem(IInputContext* pInputContext, IImGUIContext* pImGUIContext, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem, CUIEventsSystem, result, pInputContext, pImGUIContext);
	}
}