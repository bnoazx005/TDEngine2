#include "../../include/graphics/CSkinnedMeshContainer.h"
#include "../../include/graphics/IMaterial.h"


namespace TDEngine2
{
	const std::string CSkinnedMeshContainer::mJointsArrayUniformVariableId = "mJoints";
	const std::string CSkinnedMeshContainer::mJointsCountUniformVariableId = "mUsedJointsCount";


	struct TSkinnedMeshContainerArchiveKeys
	{
		static const std::string mMeshKeyId;
		static const std::string mMaterialKeyId;
		static const std::string mSkeletonKeyId;
	};


	const std::string TSkinnedMeshContainerArchiveKeys::mMeshKeyId = "mesh";
	const std::string TSkinnedMeshContainerArchiveKeys::mMaterialKeyId = "material";
	const std::string TSkinnedMeshContainerArchiveKeys::mSkeletonKeyId = "skeleton";


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

		mMaterialName = pReader->GetString(TSkinnedMeshContainerArchiveKeys::mMaterialKeyId);
		mMeshName = pReader->GetString(TSkinnedMeshContainerArchiveKeys::mMeshKeyId);
		mSkeletonName = pReader->GetString(TSkinnedMeshContainerArchiveKeys::mSkeletonKeyId);

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

			pWriter->SetString(TSkinnedMeshContainerArchiveKeys::mMaterialKeyId, mMaterialName);
			pWriter->SetString(TSkinnedMeshContainerArchiveKeys::mMeshKeyId, mMeshName);
			pWriter->SetString(TSkinnedMeshContainerArchiveKeys::mSkeletonKeyId, mSkeletonName);
		}
		pWriter->EndGroup();

		return RC_OK;
	}

	E_RESULT_CODE CSkinnedMeshContainer::Clone(IComponent*& pDestObject) const
	{
		if (CSkinnedMeshContainer* pDestComponent = dynamic_cast<CSkinnedMeshContainer*>(pDestObject))
		{
			pDestComponent->mMaterialName = mMaterialName;
			pDestComponent->mMeshName = mMeshName;
			pDestComponent->mSubMeshId = mSubMeshId;
			pDestComponent->mSubMeshInfo.mIndicesCount = mSubMeshInfo.mIndicesCount;
			pDestComponent->mSubMeshInfo.mStartIndex = mSubMeshInfo.mStartIndex;
			pDestComponent->mSystemBuffersHandle = mSystemBuffersHandle;
			pDestComponent->mSkeletonName = mSkeletonName;
			pDestComponent->mShouldShowDebugSkeleton = mShouldShowDebugSkeleton;

			pDestComponent->mCurrAnimationPose.clear();
			std::copy(mCurrAnimationPose.begin(), mCurrAnimationPose.end(), std::back_inserter(pDestComponent->mCurrAnimationPose));

#if TDE2_EDITORS_ENABLED
			pDestComponent->mSubmeshesIdentifiers.clear();
			std::copy(mSubmeshesIdentifiers.begin(), mSubmeshesIdentifiers.end(), std::back_inserter(pDestComponent->mSubmeshesIdentifiers));
#endif

			pDestComponent->mIsDirty = true;
		}

		return RC_OK;
	}

	void CSkinnedMeshContainer::SetMaterialName(const std::string& materialName)
	{
		mMaterialName = materialName;
	}

	void CSkinnedMeshContainer::SetMeshName(const std::string& meshName)
	{
		mMeshName = meshName;
		mIsDirty = true;
	}


	void CSkinnedMeshContainer::SetSubMeshId(const std::string& meshName)
	{
		mSubMeshId = meshName;
		mIsDirty = true;
	}

	void CSkinnedMeshContainer::SetSubMeshRenderInfo(const TSubMeshRenderInfo& info)
	{
		mSubMeshInfo = info;
	}

	void CSkinnedMeshContainer::SetDirty(bool value)
	{
		mIsDirty = value;
	}

#if TDE2_EDITORS_ENABLED

	void CSkinnedMeshContainer::AddSubmeshIdentifier(const std::string& submeshId)
	{
		mSubmeshesIdentifiers.push_back(submeshId);
	}

#endif

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

	const std::string& CSkinnedMeshContainer::GetSubMeshId() const
	{
		return mSubMeshId;
	}

	const TSubMeshRenderInfo& CSkinnedMeshContainer::GetSubMeshInfo() const
	{
		return mSubMeshInfo;
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

	bool CSkinnedMeshContainer::IsDirty() const
	{
		return mIsDirty;
	}

#if TDE2_EDITORS_ENABLED

	const std::vector<std::string>& CSkinnedMeshContainer::GetSubmeshesIdentifiers() const
	{
		return mSubmeshesIdentifiers;
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