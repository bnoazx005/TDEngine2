#include "../../include/graphics/CStaticMeshContainer.h"


namespace TDEngine2
{
	TDE2_REGISTER_COMPONENT_FACTORY(CreateStaticMeshContainerFactory)


	CStaticMeshContainer::CStaticMeshContainer():
		CBaseComponent(), mMaterialName(), mMeshName()
	{
	}

	E_RESULT_CODE CStaticMeshContainer::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}

		mMaterialName = pReader->GetString("material");
		mMeshName = pReader->GetString("mesh");
		mSubMeshId = pReader->GetString("sub_mesh_id");

		if (mSubMeshId == "\n" || mSubMeshId == "\r")
		{
			mSubMeshId = Wrench::StringUtils::GetEmptyStr();
		}

		mIsDirty = true;

		return RC_OK;
	}

	E_RESULT_CODE CStaticMeshContainer::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		pWriter->BeginGroup("component");
		{
			pWriter->SetUInt32("type_id", static_cast<U32>(CStaticMeshContainer::GetTypeId()));

			pWriter->SetString("material", mMaterialName);
			pWriter->SetString("mesh", mMeshName);
			pWriter->SetString("sub_mesh_id", mSubMeshId);
		}
		pWriter->EndGroup();

		return RC_OK;
	}

	E_RESULT_CODE CStaticMeshContainer::Clone(IComponent*& pDestObject) const
	{
		if (CStaticMeshContainer* pDestComponent = dynamic_cast<CStaticMeshContainer*>(pDestObject))
		{
			pDestComponent->mMaterialName              = mMaterialName;
			pDestComponent->mMeshName                  = mMeshName;
			pDestComponent->mSubMeshId                 = mSubMeshId;
			pDestComponent->mSubMeshInfo.mIndicesCount = mSubMeshInfo.mIndicesCount;
			pDestComponent->mSubMeshInfo.mStartIndex   = mSubMeshInfo.mStartIndex;
			pDestComponent->mSystemBuffersHandle       = mSystemBuffersHandle;

#if TDE2_EDITORS_ENABLED
			pDestComponent->mSubmeshesIdentifiers.clear();
			std::copy(mSubmeshesIdentifiers.begin(), mSubmeshesIdentifiers.end(), std::back_inserter(pDestComponent->mSubmeshesIdentifiers));
#endif

			pDestComponent->mIsDirty = true;
		}

		return RC_OK;
	}

	void CStaticMeshContainer::SetMaterialName(const std::string& materialName)
	{
		mMaterialName = materialName;
	}

	void CStaticMeshContainer::SetMeshName(const std::string& meshName)
	{
		mMeshName = meshName;
		mIsDirty = true;
	}

	void CStaticMeshContainer::SetSubMeshId(const std::string& meshName)
	{
		mSubMeshId = meshName;
		mIsDirty = true;
	}

	void CStaticMeshContainer::SetSubMeshRenderInfo(const TSubMeshRenderInfo& info)
	{
		mSubMeshInfo = info;
	}

	void CStaticMeshContainer::SetSystemBuffersHandle(U32 handle)
	{
		mSystemBuffersHandle = handle;
	}

	void CStaticMeshContainer::SetDirty(bool value)
	{
		mIsDirty = value;
	}

#if TDE2_EDITORS_ENABLED

	void CStaticMeshContainer::AddSubmeshIdentifier(const std::string& submeshId)
	{
		mSubmeshesIdentifiers.push_back(submeshId);
	}

#endif

	const std::string& CStaticMeshContainer::GetMaterialName() const
	{
		return mMaterialName;
	}

	const std::string& CStaticMeshContainer::GetMeshName() const
	{
		return mMeshName;
	}

	const std::string& CStaticMeshContainer::GetSubMeshId() const
	{
		return mSubMeshId;
	}

	const TSubMeshRenderInfo& CStaticMeshContainer::GetSubMeshInfo() const
	{
		return mSubMeshInfo;
	}

	U32 CStaticMeshContainer::GetSystemBuffersHandle() const
	{
		return mSystemBuffersHandle;
	}

	bool CStaticMeshContainer::IsDirty() const
	{
		return mIsDirty;
	}

#if TDE2_EDITORS_ENABLED

	const std::vector<std::string>& CStaticMeshContainer::GetSubmeshesIdentifiers() const
	{
		return mSubmeshesIdentifiers;
	}

#endif


	IComponent* CreateStaticMeshContainer(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CStaticMeshContainer, result);
	}


	/*!
		\brief CStaticMeshContainerFactory's definition
	*/

	CStaticMeshContainerFactory::CStaticMeshContainerFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* CStaticMeshContainerFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return CreateStaticMeshContainer(result);
	}

	E_RESULT_CODE CStaticMeshContainerFactory::SetupComponent(CStaticMeshContainer* pComponent, const TStaticMeshContainerParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}

		return RC_OK;
	}


	IComponentFactory* CreateStaticMeshContainerFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CStaticMeshContainerFactory, result);
	}
}