#include "../../include/core/CResourcesRuntimeManifest.h"
#include "../../include/core/IResourceFactory.h"
#include <algorithm>


namespace TDEngine2
{
	CResourcesRuntimeManifest::CResourcesRuntimeManifest() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CResourcesRuntimeManifest::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CResourcesRuntimeManifest::Load(IArchiveReader* pReader)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (!pReader)
		{
			return RC_INVALID_ARGS;
		}

		//E_RESULT_CODE result = pReader->BeginGroup(TResourcesRuntimeManifestArchiveKeys::mResourcesCollectionKeyId);

		//while (pReader->HasNextItem())
		//{
		//	result = result | pReader->BeginGroup(Wrench::StringUtils::GetEmptyStr());
		//	{
		//		result = result | pReader->BeginGroup(TResourcesRuntimeManifestArchiveKeys::mSingleResourceKeyId);

		//		mpResourcesRuntimeConfigs.emplace_back(Deserialize(pReader));

		//		result = result | pReader->EndGroup();
		//	}
		//	result = result | pReader->EndGroup();
		//}

		//result = result | pReader->EndGroup();

		return RC_OK;
	}

	E_RESULT_CODE CResourcesRuntimeManifest::Save(IArchiveWriter* pWriter)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (!pWriter)
		{
			return RC_INVALID_ARGS;
		}

		E_RESULT_CODE result = RC_OK;

		result = result | pWriter->BeginGroup("meta");
		{
			result = result | pWriter->SetString("resource_type", "resources_Runtime_manifest");
			result = result | pWriter->SetUInt16("version_tag", mVersionTag);
		}
		result = result | pWriter->EndGroup();

		/*result = result | pWriter->BeginGroup(TResourcesRuntimeManifestArchiveKeys::mResourcesCollectionKeyId, true);

		for (auto&& pCurrResourceConfig : mpResourcesRuntimeConfigs)
		{
			result = result | pWriter->BeginGroup(Wrench::StringUtils::GetEmptyStr(), false);
			{
				result = result | pWriter->BeginGroup(TResourcesRuntimeManifestArchiveKeys::mSingleResourceKeyId, false);
				result = result | pCurrResourceConfig->Save(pWriter);
				result = result | pWriter->EndGroup();
			}
			result = result | pWriter->EndGroup();
		}

		result = result | pWriter->EndGroup();*/

		return RC_OK;
	}

	E_RESULT_CODE CResourcesRuntimeManifest::AddResourceMeta(const std::string& resourceId, std::unique_ptr<TBaseResourceParameters> pResourceMeta)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (resourceId.empty() || !pResourceMeta)
		{
			return RC_INVALID_ARGS;
		}

		auto it = mpResourcesMetaInfos.find(resourceId);
		if (it == mpResourcesMetaInfos.cend())
		{
			mpResourcesMetaInfos.emplace(resourceId, std::move(pResourceMeta));
			return RC_OK;
		}

		it->second = std::move(pResourceMeta);

		return RC_OK;
	}

	bool CResourcesRuntimeManifest::HasResourceMeta(const std::string& resourceId) const
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return mpResourcesMetaInfos.find(resourceId) != mpResourcesMetaInfos.cend();
	}

	const TBaseResourceParameters* const CResourcesRuntimeManifest::GetResourceMeta(const std::string& resourceId) const
	{
		std::lock_guard<std::mutex> lock(mMutex);

		auto it = mpResourcesMetaInfos.find(resourceId);
		if (it == mpResourcesMetaInfos.cend())
		{
			return it->second.get();
		}

		return nullptr;
	}


	TDE2_API IResourcesRuntimeManifest* CreateResourcesRuntimeManifest(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IResourcesRuntimeManifest, CResourcesRuntimeManifest, result);
	}
}