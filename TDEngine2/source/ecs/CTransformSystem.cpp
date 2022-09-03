#include "../../include/ecs/CTransformSystem.h"
#include "../../include/ecs/IWorld.h"
#include "../../include/ecs/CTransform.h"
#include "../../include/core/IGraphicsContext.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/ecs/components/CBoundsComponent.h"
#include "../../include/graphics/CPerspectiveCamera.h"
#include "../../include/graphics/COrthoCamera.h"
#include "../../include/editor/CPerfProfiler.h"


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


	static constexpr U32 InvalidParentIndex = (std::numeric_limits<U32>::max)();


	void CTransformSystem::InjectBindings(IWorld* pWorld)
	{
		auto&& entities = pWorld->FindEntitiesWithComponents<CTransform>();

		auto& transforms = mComponentsContext.mpTransforms;
		auto& bounds = mComponentsContext.mpBounds;
		auto& parentsTable = mComponentsContext.mParentsHashTable;
		auto& hasCameras = mComponentsContext.mHasCameras;

		transforms.clear();
		bounds.clear();
		parentsTable.clear();
		hasCameras.clear();

		/// \note Fill up relationships table to sort entities based on their dependencies 
		std::unordered_map<TEntityId, std::vector<TEntityId>> parentToChildRelations;

		for (TEntityId currEntityId : entities)
		{
			if (CEntity* pEntity = pWorld->FindEntity(currEntityId))
			{
				parentToChildRelations[pEntity->GetComponent<CTransform>()->GetParent()].push_back(pEntity->GetId());
			}
		}

		std::stack<std::tuple<TEntityId, U32>> entitiesToProcess;

		for (TEntityId currEntityId : parentToChildRelations[TEntityId::Invalid])
		{
			entitiesToProcess.push({ currEntityId, InvalidParentIndex });
		}

		TEntityId currEntityId;
		U32 currParentElementIndex = 0;

		while (!entitiesToProcess.empty())
		{
			std::tie(currEntityId, currParentElementIndex) = entitiesToProcess.top();
			entitiesToProcess.pop();

			if (CEntity* pEntity = pWorld->FindEntity(currEntityId))
			{
				transforms.push_back(pEntity->GetComponent<CTransform>());
				bounds.push_back(pEntity->GetComponent<CBoundsComponent>());
				hasCameras.push_back(pEntity->HasComponent<CPerspectiveCamera>() || pEntity->HasComponent<COrthoCamera>());
				parentsTable.push_back(currParentElementIndex);
			}

			const U32 parentIndex = static_cast<U32>(transforms.size() - 1);

			for (TEntityId currEntityId : parentToChildRelations[currEntityId])
			{
				entitiesToProcess.push({ currEntityId, parentIndex });
			}
		}
	}


	static bool HasParentEntityTransformChanged(const CTransformSystem::TSystemContext& systemContext, USIZE index)
	{
		const U32 parentIndex = systemContext.mParentsHashTable[index];
		return (InvalidParentIndex != parentIndex) ? systemContext.mpTransforms[parentIndex]->HasChanged() : false;
	}


	void CTransformSystem::Update(IWorld* pWorld, F32 dt)
	{
		TDE2_PROFILER_SCOPE("CTransformSystem::Update");

		auto& transforms   = mComponentsContext.mpTransforms;
		auto& bounds       = mComponentsContext.mpBounds;
		auto& parentsTable = mComponentsContext.mParentsHashTable;
		auto& hasCameras   = mComponentsContext.mHasCameras;

		const F32 zAxisDirection = mpGraphicsContext->GetPositiveZAxisDirection();

		for (USIZE i = 0; i < mComponentsContext.mpTransforms.size(); ++i)
		{
			CTransform* pTransform = transforms[i];

			if (!pTransform->HasChanged() && !HasParentEntityTransformChanged(mComponentsContext, i))
			{
				continue;
			}

			const TMatrix4 translationMatrix = TranslationMatrix(pTransform->GetPosition());
			const TMatrix4 rotationMatrix = RotationMatrix(pTransform->GetRotation());

			/// \note For transforms of cameras the order of multiplication is different
			TMatrix4 translateRotateMatrix = hasCameras[i] ? rotationMatrix * translationMatrix : translationMatrix * rotationMatrix;

			TMatrix4 localToWorldMatrix = translateRotateMatrix * ScaleMatrix(pTransform->GetScale() * zAxisDirection);
			const TMatrix4 localTransform = localToWorldMatrix;

			/// \note Implement parent-to-child relationship's update
			if (InvalidParentIndex != parentsTable[i])
			{
				localToWorldMatrix = transforms[parentsTable[i]]->GetLocalToWorldTransform() * localToWorldMatrix;
			}

			pTransform->SetTransform(localToWorldMatrix, localTransform);

			if (auto pBounds = bounds[i])
			{
				pBounds->SetDirty(true);
			}
		}

		// \note Reset dirty flag for all transforms
		for (CTransform* pCurrTransform : transforms)
		{
			if (pCurrTransform)
			{
				pCurrTransform->SetDirtyFlag(false);
			}
		}
	}


	TDE2_API ISystem* CreateTransformSystem(IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem, CTransformSystem, result, pGraphicsContext);
	}
}