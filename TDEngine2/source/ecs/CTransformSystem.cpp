#include "../../include/ecs/CTransformSystem.h"
#include "../../include/ecs/IWorld.h"
#include "../../include/ecs/CTransform.h"
#include "../../include/core/IGraphicsContext.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/ecs/CSystemManager.h"
#include "../../include/ecs/components/CBoundsComponent.h"
#include "../../include/graphics/CCamera.h"
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
		TDE2_PROFILER_SCOPE("CTransformSystem::InjectBindings");

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
		std::unordered_map<TEntityId, std::vector<std::tuple<CEntity*, CTransform*>>> parentToChildRelations;

		for (TEntityId currEntityId : entities)
		{
			if (CEntity* pEntity = pWorld->FindEntity(currEntityId))
			{
				CTransform* pTransform = pEntity->GetComponent<CTransform>();
				parentToChildRelations[pTransform->GetParent()].emplace_back(pEntity, pTransform);
			}
		}

		std::vector<std::tuple<CEntity*, CTransform*, U32>> entitiesToProcess;
		entitiesToProcess.reserve(entities.size());

		for (auto [pCurrEntity, pCurrTransform] : parentToChildRelations[TEntityId::Invalid])
		{
			entitiesToProcess.emplace_back(pCurrEntity, pCurrTransform, InvalidParentIndex);
		}

		{
			TDE2_PROFILER_SCOPE("Sort");
			CEntity* pCurrEntity = nullptr;
			CTransform* pCurrTransform = nullptr;
			U32 currParentElementIndex = 0;

			while (!entitiesToProcess.empty())
			{
				std::tie(pCurrEntity, pCurrTransform, currParentElementIndex) = entitiesToProcess.back();
				entitiesToProcess.pop_back();

				transforms.push_back(pCurrTransform);
				bounds.push_back(pCurrEntity->GetComponent<CBoundsComponent>());
				hasCameras.push_back(pCurrEntity->HasComponent<CCamera>());
				parentsTable.push_back(currParentElementIndex);

				const U32 parentIndex = static_cast<U32>(transforms.size() - 1);

				for (auto [pCurrChildEntity, pCurrChildTransform] : parentToChildRelations[pCurrEntity->GetId()])
				{
					entitiesToProcess.emplace_back(pCurrChildEntity, pCurrChildTransform, parentIndex);
				}
			}
		}
	}


	static bool HasParentEntityTransformChanged(const CTransformSystem::TSystemContext& systemContext, USIZE index)
	{
		const U32 parentIndex = systemContext.mParentsHashTable[index];
		return (InvalidParentIndex != parentIndex) ? systemContext.mpTransforms[parentIndex]->HasChanged() : false;
	}


	static CTransform* GetTransformFromContextByEntityId(const Vector<CTransform*>& transforms, TEntityId id)
	{
		auto it = std::find_if(transforms.cbegin(), transforms.cend(), [id](auto&& t) { return t->GetOwnerId() == id; });
		return it == transforms.cend() ? nullptr : *it;
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

			if (pTransform->HasHierarchyChanged())
			{
				if (InvalidParentIndex != parentsTable[i] && !pTransform->IsFirstFrameAfterCreation())
				{
					auto pParentTransform = transforms[parentsTable[i]];
					const TMatrix4& parent2Child = Inverse(pParentTransform->GetChildToParentTransform());

					pTransform->SetPosition(parent2Child * pTransform->GetPosition());
					pTransform->SetRotation( pTransform->GetRotation());
					pTransform->SetScale(parent2Child * TVector4(pTransform->GetScale(), 0.0f));
				}
				else if (TEntityId::Invalid != pTransform->GetPrevParent())
				{
					auto pParentTransform = GetTransformFromContextByEntityId(transforms, pTransform->GetPrevParent());
					const TMatrix4& child2Parent = pParentTransform->GetChildToParentTransform();

					pTransform->SetPosition(child2Parent * pTransform->GetPosition());
					pTransform->SetRotation( pTransform->GetRotation());
					pTransform->SetScale(child2Parent * TVector4(pTransform->GetScale(), 0.0f));
				}

				pTransform->SetHierarchyChangedFlag(pTransform->GetParent());
			}
			
			const TMatrix4 translationMatrix = TranslationMatrix(pTransform->GetPosition());
			const TMatrix4 rotationMatrix = RotationMatrix(pTransform->GetRotation());

			/// \note For transforms of cameras the order of multiplication is different
			TMatrix4 translateRotateMatrix = hasCameras[i] ? (rotationMatrix * translationMatrix) : (TranslationMatrix(pTransform->GetPivot()) * translationMatrix * rotationMatrix);

			TMatrix4 localToWorldMatrix = translateRotateMatrix * ScaleMatrix(pTransform->GetScale() * zAxisDirection) * TranslationMatrix(-pTransform->GetPivot());
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

			pTransform->ResetFirstFrameAfterCreationFlag();
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


	struct TTransformSystemAutoInitializer : TSystemAutoInitializer
	{
		virtual ~TTransformSystemAutoInitializer() = default;

		ISystem* GetSystem(IWorld* pWorld, const TRequestSubsystemCallback& subsystemsProviderCallback)
		{
			E_RESULT_CODE result = RC_OK;
			return CreateTransformSystem(PolymorphicCast<IGraphicsContext*>(subsystemsProviderCallback(E_ENGINE_SUBSYSTEM_TYPE::EST_GRAPHICS_CONTEXT)), result);
		}
	};


	TDE2_REGISTER_SYSTEM(TTransformSystemAutoInitializer);
}