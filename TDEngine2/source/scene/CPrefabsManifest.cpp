#include "../../include/scene/CPrefabsManifest.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/core/IFileSystem.h"
#include "../../include/platform/CYAMLFile.h"
#include "stringUtils.hpp"
#include <cstring>


namespace TDEngine2
{
	struct TPrefabsManifestArchiveKeys
	{
		static const std::string mCollectionKey;
		static const std::string mEntityPrefabIdKey;
		static const std::string mEntityPathKey;
	};

	const std::string TPrefabsManifestArchiveKeys::mCollectionKey = "collection";
	const std::string TPrefabsManifestArchiveKeys::mEntityPrefabIdKey = "prefab_id";
	const std::string TPrefabsManifestArchiveKeys::mEntityPathKey = "path";


	CPrefabsManifest::CPrefabsManifest() :
		CBaseResource()
	{
	}

	E_RESULT_CODE CPrefabsManifest::Init(IResourceManager* pResourceManager, const std::string& name)
	{
		E_RESULT_CODE result = _init(pResourceManager, name);

		if (result != RC_OK)
		{
			return result;
		}

		mPrefabsMappingsTable = {};

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CPrefabsManifest::Reset()
	{
		mPrefabsMappingsTable.clear();

		return RC_OK;
	}

	E_RESULT_CODE CPrefabsManifest::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_INVALID_ARGS;
		}

		pWriter->BeginGroup("meta");
		{
			pWriter->SetString("resource_type", "prefabs_manifest");
			pWriter->SetUInt16("version_tag", mVersionTag);
		}
		pWriter->EndGroup();

		pWriter->BeginGroup(TPrefabsManifestArchiveKeys::mCollectionKey, true);
		{
			for (auto&& currPrefabEntity : mPrefabsMappingsTable)
			{
				pWriter->BeginGroup(Wrench::StringUtils::GetEmptyStr());
				{
					pWriter->SetString(TPrefabsManifestArchiveKeys::mEntityPrefabIdKey, currPrefabEntity.first);
					pWriter->SetString(TPrefabsManifestArchiveKeys::mEntityPathKey, currPrefabEntity.second);
				}
				pWriter->EndGroup();
			}
		}
		pWriter->EndGroup();

		return RC_OK;
	}

	E_RESULT_CODE CPrefabsManifest::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_INVALID_ARGS;
		}

		mPrefabsMappingsTable.clear();

		pReader->BeginGroup(TPrefabsManifestArchiveKeys::mCollectionKey);
		{
			while (pReader->HasNextItem())
			{
				pReader->BeginGroup(Wrench::StringUtils::GetEmptyStr());
				{
					mPrefabsMappingsTable.emplace(
						pReader->GetString(TPrefabsManifestArchiveKeys::mEntityPrefabIdKey),
						pReader->GetString(TPrefabsManifestArchiveKeys::mEntityPathKey));
				}
				pReader->EndGroup();
			}
		}
		pReader->EndGroup();

		return RC_OK;
	}

	const std::string& CPrefabsManifest::GetPathToPrefabById(const std::string& prefabId) const
	{
		auto it = mPrefabsMappingsTable.find(prefabId);
		return it == mPrefabsMappingsTable.cend() ? Wrench::StringUtils::GetEmptyStr() : it->second;
	}

	std::vector<std::string> CPrefabsManifest::GetPrefabsIdentifiers() const
	{
		std::vector<std::string> prefabsIdentifiers;

		for (auto&& prefabInfoEntity : mPrefabsMappingsTable)
		{
			prefabsIdentifiers.emplace_back(prefabInfoEntity.first);
		}

		return std::move(prefabsIdentifiers);
	}

	const TPtr<IResourceLoader> CPrefabsManifest::_getResourceLoader()
	{
		return mpResourceManager->GetResourceLoader<IPrefabsManifest>();
	}


	TDE2_API IPrefabsManifest* CreatePrefabsManifest(IResourceManager* pResourceManager, const std::string& name, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IPrefabsManifest, CPrefabsManifest, result, pResourceManager, name);
	}


	CPrefabsManifestLoader::CPrefabsManifestLoader() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CPrefabsManifestLoader::Init(IResourceManager* pResourceManager, IFileSystem* pFileSystem)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pResourceManager || !pFileSystem)
		{
			return RC_INVALID_ARGS;
		}

		mpResourceManager = pResourceManager;

		mpFileSystem = pFileSystem;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CPrefabsManifestLoader::LoadResource(IResource* pResource) const
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		if (TResult<TFileEntryId> prefabsManifestFileId = mpFileSystem->Open<IYAMLFileReader>(pResource->GetName()))
		{
			return dynamic_cast<IPrefabsManifest*>(pResource)->Load(mpFileSystem->Get<IYAMLFileReader>(prefabsManifestFileId.Get()));
		}

		return RC_FILE_NOT_FOUND;
	}

	TypeId CPrefabsManifestLoader::GetResourceTypeId() const
	{
		return IPrefabsManifest::GetTypeId();
	}


	TDE2_API IResourceLoader* CreatePrefabsManifestLoader(IResourceManager* pResourceManager, IFileSystem* pFileSystem, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IResourceLoader, CPrefabsManifestLoader, result, pResourceManager, pFileSystem);
	}


	CPrefabsManifestFactory::CPrefabsManifestFactory() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CPrefabsManifestFactory::Init(IResourceManager* pResourceManager)
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

	IResource* CPrefabsManifestFactory::Create(const std::string& name, const TBaseResourceParameters& params) const
	{
		const TPrefabsManifestParameters& PrefabsManifestParams = dynamic_cast<const TPrefabsManifestParameters&>(params);

		auto pResource = CreateDefault(name, params);

		return pResource;
	}

	IResource* CPrefabsManifestFactory::CreateDefault(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		return dynamic_cast<IResource*>(CreatePrefabsManifest(mpResourceManager, name, result));
	}

	TypeId CPrefabsManifestFactory::GetResourceTypeId() const
	{
		return IPrefabsManifest::GetTypeId();
	}


	TDE2_API IResourceFactory* CreatePrefabsManifestFactory(IResourceManager* pResourceManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IResourceFactory, CPrefabsManifestFactory, result, pResourceManager);
	}
}