#include "../../include/ecs/CMeshAnimatorUpdatingSystem.h"
#include "../../include/ecs/IWorld.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/ecs/components/CBoundsComponent.h"
#include "../../include/ecs/CTransform.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/editor/CPerfProfiler.h"
#include "../../include/graphics/animation/CMeshAnimatorComponent.h"
#include "../../include/graphics/CSkinnedMeshContainer.h"
#include "../../include/graphics/ISkeleton.h"
#include "../../include/graphics/animation/CAnimationContainerComponent.h"
#include "../../include/utils/CFileLogger.h"


namespace TDEngine2
{
	CMeshAnimatorUpdatingSystem::CMeshAnimatorUpdatingSystem() :
		CBaseSystem()
	{
	}

	E_RESULT_CODE CMeshAnimatorUpdatingSystem::Init(IResourceManager* pResourceManager)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pResourceManager)
		{
			return RC_INVALID_ARGS;
		}

		mpResourceManager = pResourceManager;

		mIsInitialized = true;

		return RC_OK;
	}

	void CMeshAnimatorUpdatingSystem::InjectBindings(IWorld* pWorld)
	{
		mEntitiesContext = pWorld->CreateLocalComponentsSlice<CSkinnedMeshContainer, CMeshAnimatorComponent, CAnimationContainerComponent, CBoundsComponent>();
	}

	void CMeshAnimatorUpdatingSystem::Update(IWorld* pWorld, F32 dt)
	{
		TDE2_PROFILER_SCOPE("CMeshAnimatorUpdatingSystem::Update");

		auto& skinnedMeshContainers = std::get<std::vector<CSkinnedMeshContainer*>>(mEntitiesContext.mComponentsSlice);
		auto& animationContainers   = std::get<std::vector<CAnimationContainerComponent*>>(mEntitiesContext.mComponentsSlice);
		auto& animators             = std::get<std::vector<CMeshAnimatorComponent*>>(mEntitiesContext.mComponentsSlice);
		auto& bounds                = std::get<std::vector<CBoundsComponent*>>(mEntitiesContext.mComponentsSlice);
		
		for (USIZE i = 0; i < mEntitiesContext.mComponentsCount; ++i)
		{
			CSkinnedMeshContainer* pMeshContainer = skinnedMeshContainers[i];
			const TResourceId skeletonResourceId = mpResourceManager->Load<ISkeleton>(pMeshContainer->GetSkeletonName());

			if (TResourceId::Invalid == skeletonResourceId)
			{
				continue;
			}

			auto pSkeleton = mpResourceManager->GetResource<ISkeleton>(skeletonResourceId);

			CMeshAnimatorComponent* pMeshAnimator = animators[i];
			auto& updatedJointsPose = pMeshAnimator->GetCurrAnimationPose();

			if (updatedJointsPose.empty())
			{
				auto& jointsTable = pMeshAnimator->GetJointsTable();

				U32 index = 0;

				pSkeleton->ForEachJoint([&updatedJointsPose, &jointsTable, &index](TJoint* pJoint)
				{
					if (updatedJointsPose.size() <= pJoint->mIndex + 1)
					{
						updatedJointsPose.resize(pJoint->mIndex + 1);
					}

					updatedJointsPose[pJoint->mIndex] = Inverse(pJoint->mInvBindTransform);
					jointsTable.emplace(pJoint->mName, pJoint->mIndex);
				});
			}

			auto pAnimationContainer = animationContainers[i];
			if (pAnimationContainer && pAnimationContainer->IsPlaying())
			{
				/// \note Update bind transform matrices
				auto& positions = pMeshAnimator->GetJointPositionsArray();
				auto& rotations = pMeshAnimator->GetJointRotationsArray();

				if (!positions.empty() && !rotations.empty())
				{
					pSkeleton->ForEachJoint([&updatedJointsPose, &positions, &rotations](TJoint* pJoint)
					{
						const U32 index = pJoint->mIndex;

						const TVector3& position = positions[index];
						const TQuaternion& rotation = rotations[index];

						updatedJointsPose[index] = Mul(TranslationMatrix(position), RotationMatrix(rotation));

						if (pJoint->mParentIndex >= 0)
						{
							updatedJointsPose[index] = Mul(updatedJointsPose[pJoint->mParentIndex], updatedJointsPose[index]);
						}
					});
				}
			}

			/// \note Update matrices for the mesh
			auto& currAnimationPose = pMeshContainer->GetCurrentAnimationPose();

			if (TResourceId::Invalid != skeletonResourceId)
			{
				/// \todo Refactor this fragment later
				currAnimationPose.clear();

				U32 index = 0;

				pSkeleton->ForEachJoint([&currAnimationPose, &updatedJointsPose, &index](TJoint* pJoint)
				{
					if (currAnimationPose.size() <= pJoint->mIndex + 1)
					{
						currAnimationPose.resize(pJoint->mIndex + 1);
					}

					currAnimationPose[pJoint->mIndex] = Transpose(Mul(updatedJointsPose[pJoint->mIndex], pJoint->mInvBindTransform));
				});
			}

			if (auto pBounds = bounds[i])
			{
				pBounds->SetDirty(true);
			}
		}
	}


	TDE2_API ISystem* CreateMeshAnimatorUpdatingSystem(IResourceManager* pResourceManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem, CMeshAnimatorUpdatingSystem, result, pResourceManager);
	}
}