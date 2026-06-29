#include "../../include/graphics/CStaticMeshContainer.h"
#define META_EXPORT_ECS_SECTION
#include "../../include/metadata.h"


namespace TDEngine2
{
	TDE2_REGISTER_COMPONENT_FACTORY(CreateStaticMeshContainerFactory)
	TDE2_DEFINE_COMPONENT_META(TStaticMeshContainerComponentData)


	CStaticMeshContainer::CStaticMeshContainer():
		CBaseComponentT()
	{
	}

	void CStaticMeshContainer::SetMaterialName(const std::string& materialName)
	{
		mData.mMaterialName = materialName;
	}

	void CStaticMeshContainer::SetMeshName(const std::string& meshName)
	{
		mData.mMeshName = meshName;
		mData.mIsDirty = true;
	}

	void CStaticMeshContainer::SetSubMeshId(const std::string& meshName)
	{
		mData.mSubMeshId = meshName;
		mData.mIsDirty = true;
	}

	void CStaticMeshContainer::SetSubMeshRenderInfo(const TSubMeshRenderInfo& info)
	{
		mData.mSubMeshInfo = info;
	}

	void CStaticMeshContainer::SetSystemBuffersHandle(U32 handle)
	{
		mData.mSystemBuffersHandle = handle;
	}

	void CStaticMeshContainer::SetDirty(bool value)
	{
		mData.mIsDirty = value;
	}

#if TDE2_EDITORS_ENABLED

	void CStaticMeshContainer::AddSubmeshIdentifier(const std::string& submeshId)
	{
		mData.mSubmeshesIdentifiers.push_back(submeshId);
	}

#endif

	const std::string& CStaticMeshContainer::GetMaterialName() const
	{
		return mData.mMaterialName;
	}

	const std::string& CStaticMeshContainer::GetMeshName() const
	{
		return mData.mMeshName;
	}

	const std::string& CStaticMeshContainer::GetSubMeshId() const
	{
		return mData.mSubMeshId;
	}

	const TSubMeshRenderInfo& CStaticMeshContainer::GetSubMeshInfo() const
	{
		return mData.mSubMeshInfo;
	}

	U32 CStaticMeshContainer::GetSystemBuffersHandle() const
	{
		return mData.mSystemBuffersHandle;
	}

	bool CStaticMeshContainer::IsDirty() const
	{
		return mData.mIsDirty;
	}

#if TDE2_EDITORS_ENABLED

	const std::vector<std::string>& CStaticMeshContainer::GetSubmeshesIdentifiers() const
	{
		return mData.mSubmeshesIdentifiers;
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