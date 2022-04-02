#include "../../include/graphics/CStaticMeshContainer.h"


namespace TDEngine2
{
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

	void CStaticMeshContainer::SetMaterialName(const std::string& materialName)
	{
		mMaterialName = materialName;
	}

	void CStaticMeshContainer::SetMeshName(const std::string& meshName)
	{
		mMeshName = meshName;
	}

	void CStaticMeshContainer::SetSubMeshId(const std::string& meshName)
	{
		mSubMeshId = meshName;
	}

	void CStaticMeshContainer::SetSystemBuffersHandle(U32 handle)
	{
		mSystemBuffersHandle = handle;
	}

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

	U32 CStaticMeshContainer::GetSystemBuffersHandle() const
	{
		return mSystemBuffersHandle;
	}


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