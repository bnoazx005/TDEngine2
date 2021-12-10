#include "../../include/graphics/CSkinnedMeshContainer.h"
#include "../../include/graphics/IMaterial.h"


namespace TDEngine2
{
	const std::string CSkinnedMeshContainer::mJointsArrayUniformVariableId = "mJoints";
	const std::string CSkinnedMeshContainer::mJointsCountUniformVariableId = "mUsedJointsCount";


	CSkinnedMeshContainer::CSkinnedMeshContainer():
		CBaseComponent(), mMaterialName(), mMeshName()
	{
	}

	E_RESULT_CODE CSkinnedMeshContainer::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}

		mMaterialName = pReader->GetString("material");
		mMeshName = pReader->GetString("mesh");

		return RC_OK;
	}

	E_RESULT_CODE CSkinnedMeshContainer::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		pWriter->BeginGroup("component");
		{
			pWriter->SetUInt32("type_id", static_cast<U32>(CSkinnedMeshContainer::GetTypeId()));

			pWriter->SetString("material", mMaterialName);
			pWriter->SetString("mesh", mMeshName);
		}
		pWriter->EndGroup();

		return RC_OK;
	}

	void CSkinnedMeshContainer::SetMaterialName(const std::string& materialName)
	{
		mMaterialName = materialName;
	}

	void CSkinnedMeshContainer::SetMeshName(const std::string& meshName)
	{
		mMeshName = meshName;
	}

	void CSkinnedMeshContainer::SetMaterialInstanceHandle(TMaterialInstanceId materialInstanceId)
	{
		mMaterialInstanceId = materialInstanceId;
	}

	void CSkinnedMeshContainer::SetSkeletonName(const std::string& skeletonName)
	{
		mSkeletonName = skeletonName;
	}

	void CSkinnedMeshContainer::SetSystemBuffersHandle(U32 handle)
	{
		mSystemBuffersHandle = handle;
	}

	void CSkinnedMeshContainer::SetShowDebugSkeleton(bool value)
	{
		mShouldShowDebugSkeleton = value;
	}

	const std::string& CSkinnedMeshContainer::GetMaterialName() const
	{
		return mMaterialName;
	}

	const std::string& CSkinnedMeshContainer::GetMeshName() const
	{
		return mMeshName;
	}

	TMaterialInstanceId CSkinnedMeshContainer::GetMaterialInstanceHandle() const
	{
		return mMaterialInstanceId;
	}

	const std::string& CSkinnedMeshContainer::GetSkeletonName() const
	{
		return mSkeletonName;
	}

	U32 CSkinnedMeshContainer::GetSystemBuffersHandle() const
	{
		return mSystemBuffersHandle;
	}

	std::vector<TMatrix4>& CSkinnedMeshContainer::GetCurrentAnimationPose()
	{
		return mCurrAnimationPose;
	}

	bool CSkinnedMeshContainer::ShouldShowDebugSkeleton() const
	{
		return mShouldShowDebugSkeleton;
	}


	IComponent* CreateSkinnedMeshContainer(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CSkinnedMeshContainer, result);
	}


	/*!
		\brief CSkinnedMeshContainerFactory's definition
	*/

	CSkinnedMeshContainerFactory::CSkinnedMeshContainerFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* CSkinnedMeshContainerFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return CreateSkinnedMeshContainer(result);
	}

	E_RESULT_CODE CSkinnedMeshContainerFactory::SetupComponent(CSkinnedMeshContainer* pComponent, const TSkinnedMeshContainerParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}

		return RC_OK;
	}


	IComponentFactory* CreateSkinnedMeshContainerFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CSkinnedMeshContainerFactory, result);
	}
}