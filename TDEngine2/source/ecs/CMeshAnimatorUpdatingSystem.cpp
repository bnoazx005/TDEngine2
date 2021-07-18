#include "../../include/ecs/CMeshAnimatorUpdatingSystem.h"
#include "../../include/ecs/IWorld.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/graphics/animation/CMeshAnimatorComponent.h"
#include "../../include/graphics/CSkinnedMeshContainer.h"
#include "../../include/graphics/ISkeleton.h"
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

	E_RESULT_CODE CMeshAnimatorUpdatingSystem::Free()
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

	void CMeshAnimatorUpdatingSystem::InjectBindings(IWorld* pWorld)
	{
		mProcessingEntities = pWorld->FindEntitiesWithComponents<CSkinnedMeshContainer, CMeshAnimatorComponent>();
	}

	void CMeshAnimatorUpdatingSystem::Update(IWorld* pWorld, F32 dt)
	{
		CEntity* pEntity = nullptr;

		for (TEntityId currEntityId : mProcessingEntities)
		{
			pEntity = pWorld->FindEntity(currEntityId);
			if (!pEntity)
			{
				continue;
			}

			CSkinnedMeshContainer* pMeshContainer = pEntity->GetComponent<CSkinnedMeshContainer>();
			const TResourceId skeletonResourceId = mpResourceManager->Load<ISkeleton>(pMeshContainer->GetSkeletonName());

			if (TResourceId::Invalid == skeletonResourceId)
			{
				continue;
			}

			ISkeleton* pSkeleton = mpResourceManager->GetResource<ISkeleton>(skeletonResourceId);

			CMeshAnimatorComponent* pMeshAnimator = pEntity->GetComponent<CMeshAnimatorComponent>();
			auto& updatedJointsPose = pMeshAnimator->GetCurrAnimationPose();

			if (updatedJointsPose.empty())
			{
				pSkeleton->ForEachJoint([&updatedJointsPose](TJoint* pJoint)
				{
					updatedJointsPose.push_back(Inverse(pJoint->mInvBindTransform));
				});
			}

			/// \todo Implement real update

			auto& currAnimationPose = pMeshContainer->GetCurrentAnimationPose();

			if (TResourceId::Invalid != skeletonResourceId)
			{
				/// \todo Refactor this fragment later
				currAnimationPose.clear();

				U32 index = 0;

				pSkeleton->ForEachJoint([&currAnimationPose, &updatedJointsPose, &index](TJoint* pJoint)
				{
					currAnimationPose.push_back(Transpose(Mul(updatedJointsPose[index++], pJoint->mInvBindTransform)));
				});
			}
		}
	}


	TDE2_API ISystem* CreateMeshAnimatorUpdatingSystem(IResourceManager* pResourceManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem, CMeshAnimatorUpdatingSystem, result, pResourceManager);
	}
}