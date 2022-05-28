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


	template <typename TContextType, typename TComponentType>
	static void InitContext(TContextType& context, IWorld* pWorld)
	{
		context.mpBounds.clear();
		context.mpElements.clear();
		context.mpTransforms.clear();

		for (TEntityId id : pWorld->FindEntitiesWithComponents<TComponentType>())
		{
			if (CEntity* pEntity = pWorld->FindEntity(id))
			{
				if (!pEntity->HasComponent<CBoundsComponent>())
				{
					pEntity->AddComponent<CBoundsComponent>();
				}

				context.mpBounds.push_back(pEntity->GetComponent<CBoundsComponent>());
				context.mpTransforms.push_back(pEntity->GetComponent<CTransform>());
				context.mpElements.push_back(pEntity->GetComponent<TComponentType>());
			}
		}
	}


	void CBoundsUpdatingSystem::InjectBindings(IWorld* pWorld)
	{
		InitContext<CBoundsUpdatingSystem::TStaticMeshesBoundsContext, CStaticMeshContainer>(mStaticMeshesContext, pWorld);
		InitContext<CBoundsUpdatingSystem::TSkinnedMeshesBoundsContext, CSkinnedMeshContainer>(mSkinnedMeshesContext, pWorld);
		InitContext<CBoundsUpdatingSystem::TSpritesBoundsContext, CQuadSprite>(mSpritesContext, pWorld);
	}


	static void ComputeStaticMeshBounds(IResourceManager* pResourceManager, CBoundsUpdatingSystem::TStaticMeshesBoundsContext& staticMeshesContext, USIZE id)
	{
		TDE2_PROFILER_SCOPE("ComputeStaticMeshBounds");

		CBoundsComponent* pBounds = staticMeshesContext.mpBounds[id];

		if (CStaticMeshContainer* pStaticMeshContainer = staticMeshesContext.mpElements[id])
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

				if (CTransform* pTransform = staticMeshesContext.mpTransforms[id])
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

	static void ComputeSkinnedMeshBounds(IResourceManager* pResourceManager, CBoundsUpdatingSystem::TSkinnedMeshesBoundsContext& skinnedMeshesContext, USIZE id)
	{
		TDE2_PROFILER_SCOPE("ComputeSkinnedMeshBounds");

		CBoundsComponent* pBounds = skinnedMeshesContext.mpBounds[id];

		if (CSkinnedMeshContainer* pSkinnedMeshContainer = skinnedMeshesContext.mpElements[id])
		{
			const TResourceId meshId = pResourceManager->Load<ISkinnedMesh>(pSkinnedMeshContainer->GetMeshName());

			/// \note Skip meshes that's not been loaded yet
			if (E_RESOURCE_STATE_TYPE::RST_LOADED != pResourceManager->GetResource<IResource>(meshId)->GetState())
			{
				return;
			}

			if (auto pSkinnedMesh = pResourceManager->GetResource<ISkinnedMesh>(meshId))
			{
				if (CTransform* pTransform = skinnedMeshesContext.mpTransforms[id])
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

	template <typename T, typename TFunc>
	static void ProcessMeshesBounds(IResourceManager* pResourceManager, IDebugUtility* pDebugUtility, T& meshesContext, bool isUpdateNeeded, const TFunc& functor)
	{
		TDE2_PROFILER_SCOPE("CBoundsUpdatingSystem::ProcessMeshesBounds");

		auto& bounds = meshesContext.mpBounds;

		for (USIZE i = 0; i < bounds.size(); ++i)
		{
			if (CBoundsComponent* pBounds = bounds[i])
			{
				if (pDebugUtility)
				{
					pDebugUtility->DrawAABB(pBounds->GetBounds(), TColorUtils::mWhite);
				}

				if (!isUpdateNeeded || !pBounds->IsDirty())
				{
					continue;
				}

				functor(pResourceManager, meshesContext, i);

				pBounds->SetDirty(false);
			}
		}
	}


	static void ComputeSpritesBounds(CBoundsUpdatingSystem::TSpritesBoundsContext& spritesContext, USIZE id)
	{
		TDE2_PROFILER_SCOPE("ComputeSpritesBounds");

		CBoundsComponent* pBounds = spritesContext.mpBounds[id];

		static const std::array<TVector4, 4> spriteVerts
		{
			TVector4 { -0.5f, 0.5f, 0.0f, 1.0f },
			TVector4 { 0.5f, 0.5f, 0.0f, 1.0f },
			TVector4 { -0.5f, -0.5f, 0.0f, 1.0f },
			TVector4 { 0.5f, -0.5f, 0.0f, 1.0f },
		};

		if (CTransform* pSpriteTransform = spritesContext.mpTransforms[id])
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

	static void ProcessSpritesBounds(IDebugUtility* pDebugUtility, CBoundsUpdatingSystem::TSpritesBoundsContext& spritesContext, bool isUpdateNeeded)
	{
		TDE2_PROFILER_SCOPE("CBoundsUpdatingSystem::ProcessSpritesBounds");

		auto& bounds = spritesContext.mpBounds;

		for (USIZE i = 0; i < bounds.size(); ++i)
		{
			if (CBoundsComponent* pBounds = bounds[i])
			{
				if (pDebugUtility)
				{
					pDebugUtility->DrawAABB(pBounds->GetBounds(), TColorUtils::mWhite);
				}

				if (!isUpdateNeeded || !pBounds->IsDirty())
				{
					continue;
				}

				ComputeSpritesBounds(spritesContext, i);

				pBounds->SetDirty(false);
			}
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

		ProcessMeshesBounds(mpResourceManager, mpDebugUtility, mStaticMeshesContext, isUpdateNeeded, ComputeStaticMeshBounds);
		ProcessMeshesBounds(mpResourceManager, mpDebugUtility, mSkinnedMeshesContext, isUpdateNeeded, ComputeSkinnedMeshBounds);
		ProcessSpritesBounds(mpDebugUtility, mSpritesContext, isUpdateNeeded);

		mCurrTimer += dt;
	}

	TDE2_API ISystem* CreateBoundsUpdatingSystem(IResourceManager* pResourceManager, IDebugUtility* pDebugUtility, ISceneManager* pSceneManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem, CBoundsUpdatingSystem, result, pResourceManager, pDebugUtility, pSceneManager);
	}
}