#include "../../include/ecs/CUIElementsProcessSystem.h"
#include "../../include/ecs/IWorld.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/ecs/CTransform.h"
#include "../../include/graphics/UI/CCanvasComponent.h"
#include "../../include/graphics/UI/CLayoutElementComponent.h"
#include "../../include/core/IGraphicsContext.h"


namespace TDEngine2
{
	CUIElementsProcessSystem::CUIElementsProcessSystem() :
		CBaseSystem()
	{
	}

	E_RESULT_CODE CUIElementsProcessSystem::Init(IGraphicsContext* pGraphicsContext)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pGraphicsContext)
		{
			return RC_INVALID_ARGS;
		}

		mpGraphicsContext = pGraphicsContext;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CUIElementsProcessSystem::Free()
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


	static void SortLayoutElementEntities(IWorld* pWorld, std::vector<TEntityId>& entities)
	{
		auto&& layoutElementsEntities = pWorld->FindEntitiesWithComponents<CTransform, CLayoutElement>();

		entities.clear();

		std::unordered_map<TEntityId, std::vector<TEntityId>::const_iterator> parentEntitiesTable;

		// \note Sort all entities in the following order that every parent should precede its children

		for (TEntityId currEntityId : layoutElementsEntities)
		{
			if (CEntity* pEntity = pWorld->FindEntity(currEntityId))
			{
				if (pEntity->HasComponent<CCanvas>())
				{
					continue; // skip entities with CCanvas component their will be processed before any LayoutElement ones
				}

				CTransform* pTransform = pEntity->GetComponent<CTransform>();

				if (TEntityId::Invalid == pTransform->GetParent())
				{
					entities.push_back(currEntityId);
					parentEntitiesTable[currEntityId] = entities.cend();
					continue;
				}

				const TEntityId parentId = pTransform->GetParent();

				auto it = parentEntitiesTable.find(parentId);
				if (it == parentEntitiesTable.cend())
				{
					entities.push_back(currEntityId);
					parentEntitiesTable[currEntityId] = entities.cend();

					continue;
				}

				entities.insert(it->second, currEntityId);
			}
		}
	}


	void CUIElementsProcessSystem::InjectBindings(IWorld* pWorld)
	{
		SortLayoutElementEntities(pWorld, mLayoutElementsEntities);

		mCanvasEntities = pWorld->FindEntitiesWithComponents<CTransform, CCanvas>();

		/// \note Add LayoutElement for each Canvas
		for (TEntityId currEntity : mCanvasEntities)
		{
			CEntity* pEntity = pWorld->FindEntity(currEntity);
			if (pEntity->HasComponent<CLayoutElement>())
			{
				continue;
			}

			CLayoutElement* pLayoutElement = pEntity->AddComponent<CLayoutElement>(); /// \note use both anchors that're stretched
			pLayoutElement->SetMinAnchor(ZeroVector2);
			pLayoutElement->SetMaxAnchor(TVector2(1.0f));
			pLayoutElement->SetMinOffset(ZeroVector2);
			pLayoutElement->SetMaxOffset(ZeroVector2);
		}
	}


	static void UpdateLayoutElementData(IWorld* pWorld, CEntity* pEntity)
	{
		CLayoutElement* pLayoutElement = pEntity->GetComponent<CLayoutElement>();
		CTransform* pTransform = pEntity->GetComponent<CTransform>();

		if (pEntity->HasComponent<CCanvas>())
		{
			CCanvas* pCanvas = pEntity->GetComponent<CCanvas>();
			const TVector2 canvasSizes{ static_cast<F32>(pCanvas->GetWidth()), static_cast<F32>(pCanvas->GetHeight()) };

			const TVector2 leftBottom = pLayoutElement->GetMinOffset() + Scale(pLayoutElement->GetMinAnchor(), canvasSizes);
			const TVector2 rightTop = pLayoutElement->GetMaxOffset() + Scale(pLayoutElement->GetMaxAnchor(), canvasSizes);

			const TVector2 rectSizes = rightTop - leftBottom;
			const TVector3 position = pTransform->GetPosition();

			pLayoutElement->SetWorldRect({ position.x - leftBottom.x, position.y - leftBottom.y, rectSizes.x, rectSizes.y });

			return;
		}

		if (TEntityId::Invalid == pTransform->GetParent())
		{
			TDE2_ASSERT(false);
			return;
		}

		CEntity* pParentEntity = pWorld->FindEntity(pTransform->GetParent());
		if (!pParentEntity)
		{
			TDE2_ASSERT(false);
			return;
		}

		CLayoutElement* pParentLayoutElement = pParentEntity->GetComponent<CLayoutElement>();
		if (!pParentLayoutElement)
		{
			TDE2_ASSERT(false);
			return;
		}

		auto parentWorldRect = pParentLayoutElement->GetWorldRect();

		const TVector2 parentLBRect = parentWorldRect.GetLeftBottom();
		const TVector2 parentRectSize = parentWorldRect.GetSizes();

		const TRectF32 worldRect
		{
			parentLBRect + parentRectSize * pLayoutElement->GetMinAnchor() + pLayoutElement->GetMinOffset(),
			parentLBRect + parentRectSize * pLayoutElement->GetMaxAnchor() + pLayoutElement->GetMaxOffset()
		};

		pLayoutElement->SetWorldRect(worldRect);

		const TVector2 position = worldRect.GetLeftBottom() + worldRect.GetSizes() * pLayoutElement->GetPivot();
		pTransform->SetPosition(TVector3(position.x, position.y, 0.0f));
	}


	static void UpdateCanvasData(IGraphicsContext* pGraphicsContext, IWorld* pWorld, CEntity* pEntity)
	{
		TDE2_ASSERT(pEntity->HasComponent<CCanvas>());

		if (CCanvas* pCanvas = pEntity->GetComponent<CCanvas>())
		{
			if (!pCanvas->IsDirty())
			{
				return;
			}

			/// \note The canvas's origin is a left-bottom corner
			pCanvas->SetProjMatrix(pGraphicsContext->CalcOrthographicMatrix(0.0f, static_cast<F32>(pCanvas->GetHeight()), static_cast<F32>(pCanvas->GetWidth()), 0.0f, 0.0f, 1.0f, true));
		}
	}

	static TEntityId FindParentCanvasEntityId(IWorld* pWorld, CEntity* pEntity)
	{
		CEntity* pCurrEntity = pEntity;

		CTransform* pTransform = pEntity->GetComponent<CTransform>();
		TEntityId currParentId = pTransform->GetParent();

		while ((TEntityId::Invalid != currParentId) && !pCurrEntity->HasComponent<CCanvas>())
		{
			pCurrEntity = pWorld->FindEntity(currParentId);
			
			pTransform = pCurrEntity->GetComponent<CTransform>();
			currParentId = pTransform->GetParent();
		}

		return (pCurrEntity == pEntity) ? TEntityId::Invalid : pCurrEntity->GetId();
	}


	void CUIElementsProcessSystem::Update(IWorld* pWorld, F32 dt)
	{
		CEntity* pEntity = nullptr;

		/// \note Update canvas entities
		for (TEntityId currEntity : mCanvasEntities)
		{
			pEntity = pWorld->FindEntity(currEntity);

			UpdateCanvasData(mpGraphicsContext, pWorld, pEntity);
			UpdateLayoutElementData(pWorld, pEntity); 
		}

		/// \note Process LayoutElement entities
		for (TEntityId currEntity : mLayoutElementsEntities)
		{
			pEntity = pWorld->FindEntity(currEntity);
			UpdateLayoutElementData(pWorld, pEntity);
			
			if (auto pLayoutElement = pEntity->GetComponent<CLayoutElement>())
			{
				pLayoutElement->SetOwnerCanvasId(FindParentCanvasEntityId(pWorld, pEntity));
			}
		}
	}


	TDE2_API ISystem* CreateUIElementsProcessSystem(IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem, CUIElementsProcessSystem, result, pGraphicsContext);
	}
}