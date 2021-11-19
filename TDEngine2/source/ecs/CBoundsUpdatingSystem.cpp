#include "../../include/ecs/CBoundsUpdatingSystem.h"
#include "../../include/ecs/IWorld.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/ecs/CTransform.h"
#include "../../include/ecs/components/CBoundsComponent.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/core/CProjectSettings.h"
#include "../../include/graphics/CStaticMesh.h"
#include "../../include/graphics/CStaticMeshContainer.h"
#include "../../include/graphics/CSkinnedMesh.h"
#include "../../include/graphics/CSkinnedMeshContainer.h"
#include "../../include/graphics/CQuadSprite.h"
#include "../../include/graphics/IDebugUtility.h"
#include "../../include/graphics/ISkeleton.h"
#include "../../include/utils/CFileLogger.h"
#include "../../include/math/TVector4.h"
#include "../../include/editor/ecs/EditorComponents.h"
#include "../../include/scene/ISceneManager.h"
#include "../../include/scene/IScene.h"
#include "../../include/math/TAABB.h"
#include "../../include/editor/CPerfProfiler.h"


namespace TDEngine2
{
	CBoundsUpdatingSystem::CBoundsUpdatingSystem() :
		CBaseSystem()
	{
	}

	E_RESULT_CODE CBoundsUpdatingSystem::Init(IResourceManager* pResourceManager, IDebugUtility* pDebugUtility, ISceneManager* pSceneManager)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pResourceManager || !pSceneManager)
		{
			return RC_INVALID_ARGS;
		}

		mpResourceManager = pResourceManager;
		mpDebugUtility    = pDebugUtility;
		mpSceneManager    = pSceneManager;

		mIsInitialized = true;

		return RC_OK;
	}

	void CBoundsUpdatingSystem::InjectBindings(IWorld* pWorld)
	{
		mStaticMeshesEntities  = pWorld->FindEntitiesWithComponents<CStaticMeshContainer>();
		mSkinnedMeshesEntities = pWorld->FindEntitiesWithComponents<CSkinnedMeshContainer>();
		mSpritesEntities       = pWorld->FindEntitiesWithComponents<CQuadSprite>();

#if TDE2_EDITORS_ENABLED
		mScenesBoundariesEntities = pWorld->FindEntitiesWithComponents<CSceneInfoComponent>();
#endif
	}


	static void ProcessEntities(IDebugUtility* pDebugUtility, IWorld* pWorld, const std::vector<TEntityId>& entities, bool isUpdateNeeded, const std::function<void(CEntity*)>& processCallback)
	{
		CEntity* pEntity = nullptr;

		for (TEntityId currEntity : entities)
		{
			if (!(pEntity = pWorld->FindEntity(currEntity)))
			{
				continue;
			}

			if (!pEntity->HasComponent<CBoundsComponent>())
			{
				pEntity->AddComponent<CBoundsComponent>();
				return;
			}

			if (CBoundsComponent* pBounds = pEntity->GetComponent<CBoundsComponent>())
			{
				if (pDebugUtility)
				{
					pDebugUtility->DrawAABB(pBounds->GetBounds(), TColorUtils::mWhite);
				}

				if (!isUpdateNeeded || !pBounds->IsDirty())
				{
					continue;
				}

				if (processCallback) // \note Compute bounds for the entity
				{
					processCallback(pEntity);
				}

				pBounds->SetDirty(false);
			}
		}
	}

	static void ComputeStaticMeshBounds(IResourceManager* pResourceManager, CEntity* pEntity)
	{
		TDE2_PROFILER_SCOPE("ComputeStaticMeshBounds");

		CBoundsComponent* pBounds = pEntity->GetComponent<CBoundsComponent>();

		if (CStaticMeshContainer* pStaticMeshContainer = pEntity->GetComponent<CStaticMeshContainer>())
		{
			const TResourceId meshId = pResourceManager->Load<IStaticMesh>(pStaticMeshContainer->GetMeshName());

			/// \note Skip meshes that's not been loaded yet
			if (E_RESOURCE_STATE_TYPE::RST_LOADED != pResourceManager->GetResource<IResource>(meshId)->GetState())
			{
				return;
			}

			if (auto pStaticMesh = pResourceManager->GetResource<IStaticMesh>(meshId))
			{
				auto&& vertices = pStaticMesh->GetPositionsArray();

				if (CTransform* pTransform = pEntity->GetComponent<CTransform>())
				{
					const TMatrix4& worldMatrix = pTransform->GetLocalToWorldTransform();

					TVector4 min{ (std::numeric_limits<F32>::max)() };
					TVector4 max{ -(std::numeric_limits<F32>::max)() };

					for (auto&& v : vertices)
					{
						TVector4 transformedVertex = worldMatrix * v;

						min = Min(min, transformedVertex);
						max = Max(max, transformedVertex);
					}

					pBounds->SetBounds(TAABB{ min, max });
				}
			}
		}
	}

	static void ComputeSkinnedMeshBounds(IResourceManager* pResourceManager, CEntity* pEntity)
	{
		TDE2_PROFILER_SCOPE("ComputeSkinnedMeshBounds");

		CBoundsComponent* pBounds = pEntity->GetComponent<CBoundsComponent>();

		if (CSkinnedMeshContainer* pSkinnedMeshContainer = pEntity->GetComponent<CSkinnedMeshContainer>())
		{
			const TResourceId meshId = pResourceManager->Load<ISkinnedMesh>(pSkinnedMeshContainer->GetMeshName());

			/// \note Skip meshes that's not been loaded yet
			if (E_RESOURCE_STATE_TYPE::RST_LOADED != pResourceManager->GetResource<IResource>(meshId)->GetState())
			{
				return;
			}

			if (auto pSkinnedMesh = pResourceManager->GetResource<ISkinnedMesh>(meshId))
			{
				if (CTransform* pTransform = pEntity->GetComponent<CTransform>())
				{
					auto&& currAnimationPose = pSkinnedMeshContainer->GetCurrentAnimationPose();
					auto&& vertices = pSkinnedMesh->GetPositionsArray();
					auto&& jointIndices = pSkinnedMesh->GetJointIndicesArray();
					auto&& jointWeights = pSkinnedMesh->GetJointWeightsArray();

					const TMatrix4& worldMatrix = pTransform->GetLocalToWorldTransform();

					TVector4 min{ (std::numeric_limits<F32>::max)() };
					TVector4 max{ -(std::numeric_limits<F32>::max)() };

					/// \note Compute whole CPU skinning for correct updates of bounds when the model is animated

					for (U32 i = 0; i < vertices.size(); ++i)
					{
						TVector4 skinVertex = TVector4(ZeroVector3, 1.0f);
						const TVector4& currVertex = vertices[i];

						for (U8 k = 0; k < jointWeights[i].size(); ++k)
						{
							const U32 jointIndex = jointIndices[i][k];

							const auto& jointMatrix = (jointIndex < ISkeleton::mMaxNumOfJoints && jointIndex < currAnimationPose.size()) ? currAnimationPose[jointIndex] : ZeroMatrix4;

							skinVertex = skinVertex + jointWeights[i][k] * Mul(jointMatrix, currVertex);
						}
						
						TVector4 transformedVertex = worldMatrix * vertices[i];

						min = Min(min, transformedVertex);
						max = Max(max, transformedVertex);
					}

					pBounds->SetBounds(TAABB{ min, max });
				}
			}
		}
	}

	static void ComputeSpritesBounds(CEntity* pEntity)
	{
		TDE2_PROFILER_SCOPE("ComputeSpritesBounds");

		CBoundsComponent* pBounds = pEntity->GetComponent<CBoundsComponent>();

		static const std::array<TVector4, 4> spriteVerts
		{
			TVector4 { -0.5f, 0.5f, 0.0f, 1.0f },
			TVector4 { 0.5f, 0.5f, 0.0f, 1.0f },
			TVector4 { -0.5f, -0.5f, 0.0f, 1.0f },
			TVector4 { 0.5f, -0.5f, 0.0f, 1.0f },
		};

		if (CTransform* pSpriteTransform = pEntity->GetComponent<CTransform>())
		{
			const TMatrix4& worldMatrix = pSpriteTransform->GetLocalToWorldTransform();

			TVector4 min{ (std::numeric_limits<F32>::max)() };
			TVector4 max{ -(std::numeric_limits<F32>::max)() };

			for (auto&& v : spriteVerts)
			{
				TVector4 transformedVertex = worldMatrix * v;

				min = Min(min, transformedVertex);
				max = Max(max, transformedVertex);
			}

			pBounds->SetBounds(TAABB{ min, max });
		}
	}


	void CBoundsUpdatingSystem::Update(IWorld* pWorld, F32 dt)
	{
		const bool isUpdateNeeded = mCurrTimer >= CProjectSettings::Get()->mWorldSettings.mEntitiesBoundsUpdateInterval;

		if (isUpdateNeeded)
		{
			mCurrTimer = 0.0f;
		}

		TDE2_PROFILER_SCOPE("CBoundsUpdatingSystem::Update");

		ProcessEntities(mpDebugUtility, pWorld, mStaticMeshesEntities, isUpdateNeeded, [this](CEntity* pEntity) { ComputeStaticMeshBounds(mpResourceManager, pEntity); });
		ProcessEntities(mpDebugUtility, pWorld, mSkinnedMeshesEntities, isUpdateNeeded, [this](CEntity* pEntity) { ComputeSkinnedMeshBounds(mpResourceManager, pEntity); });
		ProcessEntities(mpDebugUtility, pWorld, mSpritesEntities, isUpdateNeeded, std::bind(&ComputeSpritesBounds, std::placeholders::_1));

#if TDE2_EDITORS_ENABLED
		_processScenesEntities(pWorld);
#endif

		mCurrTimer += dt;
	}

#if TDE2_EDITORS_ENABLED
	void CBoundsUpdatingSystem::_processScenesEntities(IWorld* pWorld)
	{
		TDE2_PROFILER_SCOPE("CBoundsUpdatingSystem::_processScenesEntities");

		CEntity* pEntity = nullptr;

		for (TEntityId currEntity : mScenesBoundariesEntities)
		{
			if (!(pEntity = pWorld->FindEntity(currEntity)))
			{
				continue;
			}

			if (!pEntity->HasComponent<CBoundsComponent>())
			{
				pEntity->AddComponent<CBoundsComponent>();
			}

			if (CBoundsComponent* pBounds = pEntity->GetComponent<CBoundsComponent>())
			{
				TAABB currBounds = pBounds->GetBounds();

				if (CSceneInfoComponent* pSceneInfo = pEntity->GetComponent<CSceneInfoComponent>())
				{
					auto pScene = mpSceneManager->GetScene(mpSceneManager->GetSceneId(pSceneInfo->GetSceneId()));
					if (pScene.IsOk())
					{
						pScene.Get()->ForEachEntity([&currBounds](CEntity* pOtherEntity)
						{
							if (CBoundsComponent* pEntityBounds = pOtherEntity->GetComponent<CBoundsComponent>())
							{
								currBounds = UnionBoundingBoxes(currBounds, pEntityBounds->GetBounds());
							}
						});
					}
				}

				pBounds->SetBounds(currBounds);

				if (mpDebugUtility)
				{
					mpDebugUtility->DrawAABB(currBounds, { 1.0f, 0.0f, 0.5f, 1.0f });
				}
			}
		}
	}
#endif

	TDE2_API ISystem* CreateBoundsUpdatingSystem(IResourceManager* pResourceManager, IDebugUtility* pDebugUtility, ISceneManager* pSceneManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem, CBoundsUpdatingSystem, result, pResourceManager, pDebugUtility, pSceneManager);
	}
}