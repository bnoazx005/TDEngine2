#include "../../include/graphics/CSkinnedMeshContainer.h"
#include "../../include/graphics/IMaterial.h"
#define META_EXPORT_ECS_SECTION
#include "../../include/metadata.h"


namespace TDEngine2
{
	TDE2_REGISTER_COMPONENT_FACTORY(CreateSkinnedMeshContainerFactory)
	TDE2_DEFINE_COMPONENT_META(TSkinnedMeshContainerComponentData)


	const std::string CSkinnedMeshContainer::mJointsArrayUniformVariableId = "mJoints";
	const std::string CSkinnedMeshContainer::mJointsCountUniformVariableId = "mUsedJointsCount";


	CSkinnedMeshContainer::CSkinnedMeshContainer():
		CBaseComponentT()
	{
	}

	void CSkinnedMeshContainer::SetMaterialName(const std::string& materialName)
	{
		mData.mMaterialName = materialName;
	}

	void CSkinnedMeshContainer::SetMeshName(const std::string& meshName)
	{
		mData.mMeshName = meshName;
		mData.mIsDirty = true;
	}


	void CSkinnedMeshContainer::SetSubMeshId(const std::string& meshName)
	{
		mData.mSubMeshId = meshName;
		mData.mIsDirty = true;
	}

	void CSkinnedMeshContainer::SetSubMeshRenderInfo(const TSubMeshRenderInfo& info)
	{
		mData.mSubMeshInfo = info;
	}

	void CSkinnedMeshContainer::SetDirty(bool value)
	{
		mData.mIsDirty = value;
	}

#if TDE2_EDITORS_ENABLED

	void CSkinnedMeshContainer::AddSubmeshIdentifier(const std::string& submeshId)
	{
		mData.mSubmeshesIdentifiers.push_back(submeshId);
	}

#endif

	void CSkinnedMeshContainer::SetMaterialInstanceHandle(TMaterialInstanceId materialInstanceId)
	{
		mData.mMaterialInstanceId = materialInstanceId;
	}

	void CSkinnedMeshContainer::SetSkeletonName(const std::string& skeletonName)
	{
		mData.mSkeletonName = skeletonName;
	}

	void CSkinnedMeshContainer::SetSystemBuffersHandle(U32 handle)
	{
		mData.mSystemBuffersHandle = handle;
	}

	void CSkinnedMeshContainer::SetShowDebugSkeleton(bool value)
	{
		mData.mShouldShowDebugSkeleton = value;
	}

	const std::string& CSkinnedMeshContainer::GetMaterialName() const
	{
		return mData.mMaterialName;
	}

	const std::string& CSkinnedMeshContainer::GetMeshName() const
	{
		return mData.mMeshName;
	}

	const std::string& CSkinnedMeshContainer::GetSubMeshId() const
	{
		return mData.mSubMeshId;
	}

	const TSubMeshRenderInfo& CSkinnedMeshContainer::GetSubMeshInfo() const
	{
		return mData.mSubMeshInfo;
	}

	TMaterialInstanceId CSkinnedMeshContainer::GetMaterialInstanceHandle() const
	{
		return mData.mMaterialInstanceId;
	}

	const std::string& CSkinnedMeshContainer::GetSkeletonName() const
	{
		return mData.mSkeletonName;
	}

	U32 CSkinnedMeshContainer::GetSystemBuffersHandle() const
	{
		return mData.mSystemBuffersHandle;
	}

	std::vector<TMatrix4>& CSkinnedMeshContainer::GetCurrentAnimationPose()
	{
		return mData.mCurrAnimationPose;
	}

	bool CSkinnedMeshContainer::ShouldShowDebugSkeleton() const
	{
		return mData.mShouldShowDebugSkeleton;
	}

	bool CSkinnedMeshContainer::IsDirty() const
	{
		return mData.mIsDirty;
	}

#if TDE2_EDITORS_ENABLED

	const std::vector<std::string>& CSkinnedMeshContainer::GetSubmeshesIdentifiers() const
	{
		return mData.mSubmeshesIdentifiers;
	}

#endif


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