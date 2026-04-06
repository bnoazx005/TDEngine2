#include "../../include/ecs/CLODMeshSwitchSystem.h"
#include "../../include/editor/CPerfProfiler.h"
#include "../../include/ecs/IWorld.h"
#include "../../include/ecs/CTransform.h"
#include "../../include/ecs/CSystemManager.h"
#include "../../include/ecs/CStaticMeshRendererSystem.h"
#include "../../include/ecs/CSkinnedMeshRendererSystem.h"
#include "../../include/ecs/CCameraSystem.h"
#include "../../include/graphics/CStaticMeshContainer.h"
#include "../../include/graphics/CSkinnedMeshContainer.h"
#include "../../include/graphics/CBaseCamera.h"
#include "../../include/scene/components/CLODStrategyComponent.h"


namespace TDEngine2
{
	CLODMeshSwitchSystem::CLODMeshSwitchSystem() :
		CBaseSystem()
	{
	}

	E_RESULT_CODE CLODMeshSwitchSystem::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	void CLODMeshSwitchSystem::InjectBindings(IWorld* pWorld)
	{
		mStaticMeshesLODs = pWorld->CreateLocalComponentsSlice<CLODStrategyComponent, CStaticMeshContainer, CTransform>();
		mSkinnedMeshesLODs = pWorld->CreateLocalComponentsSlice<CLODStrategyComponent, CSkinnedMeshContainer, CTransform>();

		mpCamerasContext = pWorld->FindEntity(pWorld->FindEntityWithUniqueComponent<CCamerasContextComponent>())->GetComponent<CCamerasContextComponent>();
		mpCurrActiveCameraTransform = pWorld->FindEntity(mpCamerasContext->GetActiveCameraEntityId())->GetComponent<CTransform>();
	}


	template <typename TGraphicsComponentType, typename TPredicate>
	static void UpdateLODSEntities(const TComponentsQueryLocalSlice<CLODStrategyComponent, TGraphicsComponentType, CTransform>& context, CTransform* pCameraTransform,
									const TPredicate& changeMeshLODAction)
	{
		TDE2_PROFILER_SCOPE("UpdateLODSEntities");

		auto& transforms  = std::get<Vector<CTransform*>>(context.mComponentsSlice);
		auto& meshes      = std::get<Vector<TGraphicsComponentType*>>(context.mComponentsSlice);
		auto& lodStrategy = std::get<Vector<CLODStrategyComponent*>>(context.mComponentsSlice);

		const TVector3 cameraWorldPosition = pCameraTransform->GetPosition();

		for (USIZE i = 0; i < context.mComponentsCount; ++i)
		{
			CLODStrategyComponent* pLODStrategy = lodStrategy[i];

			const TVector3 objectPosition = transforms[i]->GetPosition();

			if (auto pLODInstance = pLODStrategy->GetLODInfo(Length(objectPosition - cameraWorldPosition)))
			{
				changeMeshLODAction(meshes[i], *pLODInstance);
			}
		}
	}


	template <typename TGraphicsComponentType>
	static void AssignMeshLODValues(TGraphicsComponentType* pMesh, TLODInstanceInfo& info)
	{
		if (HasEnumFlag(info.mActiveParams, E_LOD_INSTANCE_ACTIVE_PARAMS::MESH_ID)) { pMesh->SetMeshName(info.mMeshId); }
		if (HasEnumFlag(info.mActiveParams, E_LOD_INSTANCE_ACTIVE_PARAMS::SUBMESH_ID)) { pMesh->SetSubMeshId(info.mSubMeshId); }
		if (HasEnumFlag(info.mActiveParams, E_LOD_INSTANCE_ACTIVE_PARAMS::MATERIAL_ID)) { pMesh->SetMaterialName(info.mMaterialId); }
	}


	void CLODMeshSwitchSystem::Update(IWorld* pWorld, F32 dt)
	{
		TDE2_PROFILER_SCOPE("CLODMeshSwitchSystem::Update");

		UpdateLODSEntities(mStaticMeshesLODs, mpCurrActiveCameraTransform, AssignMeshLODValues<CStaticMeshContainer>);
		UpdateLODSEntities(mSkinnedMeshesLODs, mpCurrActiveCameraTransform, AssignMeshLODValues<CSkinnedMeshContainer>);
	}


	TDE2_API ISystem* CreateLODMeshSwitchSystem(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem, CLODMeshSwitchSystem, result);
	}


	struct TLODMeshSwitchSystemAutoInitializer : TSystemAutoInitializer
	{
		virtual ~TLODMeshSwitchSystemAutoInitializer() = default;

		E_RESULT_CODE ManageDependencies(IWorld* pWorld) override
		{
			std::array<TSystemId, 3> dependenciesHandles
			{
				pWorld->FindSystem<CCameraSystem>(),
				pWorld->FindSystem<CStaticMeshRendererSystem>(),
				pWorld->FindSystem<CSkinnedMeshRendererSystem>(),
			};

			E_RESULT_CODE result = RC_OK;

			for (const TSystemId& currDependencyHandle : dependenciesHandles)
			{
				result = result | (currDependencyHandle == TSystemId::Invalid ? RC_FAIL : RC_OK);
				result = result | pSystemInstance->AddDependency(pWorld->GetSystem(currDependencyHandle));
			}
			return result;
		}

		ISystem* GetSystem(IWorld* pWorld, const TRequestSubsystemCallback& subsystemsProviderCallback)
		{
			E_RESULT_CODE result = RC_OK;
			pSystemInstance = CreateLODMeshSwitchSystem(result);

			return pSystemInstance;
		}

		ISystem* pSystemInstance = nullptr;
	};


	TDE2_REGISTER_SYSTEM(TLODMeshSwitchSystemAutoInitializer);
}