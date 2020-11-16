#include "../../include/graphics/CStaticMeshContainer.h"


namespace TDEngine2
{
	CStaticMeshContainer::CStaticMeshContainer():
		CBaseComponent(), mMaterialName(), mMeshName()
	{
	}

	E_RESULT_CODE CStaticMeshContainer::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CStaticMeshContainer::Load(IArchiveReader* pReader)
	{
		return CBaseComponent::Load(pReader);
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

	U32 CStaticMeshContainer::GetSystemBuffersHandle() const
	{
		return mSystemBuffersHandle;
	}


	IComponent* CreateStaticMeshContainer(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CStaticMeshContainer, result);
	}


	CStaticMeshContainerFactory::CStaticMeshContainerFactory() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CStaticMeshContainerFactory::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CStaticMeshContainerFactory::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	IComponent* CStaticMeshContainerFactory::Create(const TBaseComponentParameters* pParams) const
	{
		if (!pParams)
		{
			return nullptr;
		}

		const TStaticMeshContainerParameters* transformParams = static_cast<const TStaticMeshContainerParameters*>(pParams);

		E_RESULT_CODE result = RC_OK;

		return CreateStaticMeshContainer(result);
	}

	IComponent* CStaticMeshContainerFactory::CreateDefault(const TBaseComponentParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		return CreateStaticMeshContainer(result);
	}

	TypeId CStaticMeshContainerFactory::GetComponentTypeId() const
	{
		return CStaticMeshContainer::GetTypeId();
	}


	IComponentFactory* CreateStaticMeshContainerFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CStaticMeshContainerFactory, result);
	}
}