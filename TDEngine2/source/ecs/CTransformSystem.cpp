#include "../../include/ecs/CTransformSystem.h"
#include "../../include/ecs/IWorld.h"
#include "../../include/ecs/CTransform.h"
#include "../../include/core/IGraphicsContext.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/ecs/components/CBoundsComponent.h"


namespace TDEngine2
{
	CTransformSystem::CTransformSystem() :
		CBaseSystem()
	{
	}

	E_RESULT_CODE CTransformSystem::Init(IGraphicsContext* pGraphicsContext)
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

	E_RESULT_CODE CTransformSystem::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	void CTransformSystem::InjectBindings(IWorld* pWorld)
	{
		auto&& entities = pWorld->FindEntitiesWithComponents<CTransform>();

		mTransformEntities.clear();

		std::unordered_map<TEntityId, std::vector<TEntityId>::const_iterator> parentEntitiesTable;

		// \note Sort all entities in the following order that every parent should precede its children

		for (TEntityId currEntityId : entities)
		{
			if (CEntity* pEntity = pWorld->FindEntity(currEntityId))
			{
				CTransform* pTransform = pEntity->GetComponent<CTransform>();

				if (TEntityId::Invalid == pTransform->GetParent())
				{
					mTransformEntities.push_back(currEntityId);
					parentEntitiesTable[currEntityId] = mTransformEntities.cend();
					continue;
				}

				const TEntityId parentId = pTransform->GetParent();
				
				auto it = parentEntitiesTable.find(parentId);
				if (it == parentEntitiesTable.cend())
				{
					mTransformEntities.push_back(currEntityId);
					parentEntitiesTable[currEntityId] = mTransformEntities.cend();
					
					continue;
				}

				mTransformEntities.insert(it->second, currEntityId);
			}
		}
	}

	void CTransformSystem::Update(IWorld* pWorld, F32 dt)
	{
		for (TEntityId currEntity : mTransformEntities)
		{
			if (CEntity* pEntity = pWorld->FindEntity(currEntity))
			{
				CTransform* pTransform = pEntity->GetComponent<CTransform>();

				const TEntityId parentEntityId = pTransform->GetParent();

				if (!pTransform->HasChanged() && !_hasParentEntityTransformChanged(pWorld, parentEntityId))
				{
					continue;
				}

				auto scale = pTransform->GetScale();
				scale.z *= mpGraphicsContext->GetPositiveZAxisDirection();

				TMatrix4 localToWorldMatrix = (TranslationMatrix(pTransform->GetPosition()) *
												RotationMatrix(pTransform->GetRotation())) *
												ScaleMatrix(scale);

				/// \note Implement parent-to-child relationship's update
				
				if (TEntityId::Invalid != parentEntityId)
				{
					if (auto pParentEntity = pWorld->FindEntity(parentEntityId))
					{
						CTransform* pParentTransform = pParentEntity->GetComponent<CTransform>();

						localToWorldMatrix = pParentTransform->GetLocalToWorldTransform() * localToWorldMatrix;
					}
				}

				pTransform->SetTransform(localToWorldMatrix);

				if (auto pBounds = pEntity->GetComponent<CBoundsComponent>())
				{
					pBounds->SetDirty(true);
				}
			}
		}

		// \note Reset dirty flag for all transforms
		for (TEntityId currEntity : mTransformEntities)
		{
			if (CEntity* pEntity = pWorld->FindEntity(currEntity))
			{
				CTransform* pTransform = pEntity->GetComponent<CTransform>();
				pTransform->SetDirtyFlag(false);
			}
		}
	}

	bool CTransformSystem::_hasParentEntityTransformChanged(IWorld* pWorld, TEntityId id)
	{
		if (CEntity* pEntity = pWorld->FindEntity(id))
		{
			CTransform* pTransform = pEntity->GetComponent<CTransform>();
			return pTransform->HasChanged();
		}

		return false;
	}


	TDE2_API ISystem* CreateTransformSystem(IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem, CTransformSystem, result, pGraphicsContext);
	}
}