#include "../../include/core/CResourcesRuntimeManifest.h"
#include "../../include/core/IResourceFactory.h"
#include "../../include/graphics/CBaseTexture2D.h"
#include "../../include/graphics/IShader.h"
#include "stringUtils.hpp"
#include <algorithm>


namespace TDEngine2
{
	struct TResourcesRuntimeManifestArchiveKeys
	{
		static const std::string mResourcesMetaCollectionKeyId;
		static const std::string mSingleResourceKeyId;
		static const std::string mResourceIdKeyId;
	};


	const std::string TResourcesRuntimeManifestArchiveKeys::mResourcesMetaCollectionKeyId = "resources_meta_collection";
	const std::string TResourcesRuntimeManifestArchiveKeys::mSingleResourceKeyId = "resource_meta";
	const std::string TResourcesRuntimeManifestArchiveKeys::mResourceIdKeyId = "id";


	static std::unique_ptr<TBaseResourceParameters> Deserialize(IArchiveReader* pReader)
	{
		static const std::unordered_map<TypeId, std::function<std::unique_ptr<TBaseResourceParameters>()>> factories
		{
			{ TDE2_TYPE_ID(TTexture2DParameters), []() { return std::make_unique<TTexture2DParameters>(); } },
			{ TDE2_TYPE_ID(TShaderParameters), []() { return std::make_unique<TShaderParameters>(); } },
		};

		auto it = factories.find(TypeId(pReader->GetUInt32("type_id")));
		if (it == factories.cend())
		{
			return nullptr;
		}

		auto pInfoPtr = std::move(it->second());
		if (!pInfoPtr)
		{
			return nullptr;
		}

		E_RESULT_CODE result = pInfoPtr->Load(pReader);
		TDE2_ASSERT(RC_OK == result);

		return std::move(pInfoPtr);
	}



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

		E_RESULT_CODE result = pReader->BeginGroup(TResourcesRuntimeManifestArchiveKeys::mResourcesMetaCollectionKeyId);

		mpResourcesMetaInfos.clear();
		mRuntimeToOriginalResourcesPathsTable.clear();

		while (pReader->HasNextItem())
		{
			result = result | pReader->BeginGroup(Wrench::StringUtils::GetEmptyStr());
			{
				result = result | pReader->BeginGroup(TResourcesRuntimeManifestArchiveKeys::mSingleResourceKeyId);

				auto&& originalPath = pReader->GetString(TResourcesRuntimeManifestArchiveKeys::mResourceIdKeyId);
				auto&& resourcePath = mBaseResourcesPathPrefix + (Wrench::StringUtils::StartsWith(originalPath, ".") ? originalPath.substr(1) : originalPath);

				mRuntimeToOriginalResourcesPathsTable[resourcePath] = originalPath;
				mpResourcesMetaInfos[resourcePath] = std::move(Deserialize(pReader));

				result = result | pReader->EndGroup();
			}
			result = result | pReader->EndGroup();
		}

		result = result | pReader->EndGroup();

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
			result = result | pWriter->SetString("resource_type", "resources_runtime_manifest");
			result = result | pWriter->SetUInt16("version_tag", mVersionTag);
		}
		result = result | pWriter->EndGroup();

		result = result | pWriter->BeginGroup(TResourcesRuntimeManifestArchiveKeys::mResourcesMetaCollectionKeyId, true);

		for (auto&& pCurrResourceConfig : mpResourcesMetaInfos)
		{
			result = result | pWriter->BeginGroup(Wrench::StringUtils::GetEmptyStr(), false);
			{
				result = result | pWriter->BeginGroup(TResourcesRuntimeManifestArchiveKeys::mSingleResourceKeyId, false);
				result = result | pWriter->SetString(TResourcesRuntimeManifestArchiveKeys::mResourceIdKeyId, mRuntimeToOriginalResourcesPathsTable[pCurrResourceConfig.first]);
				result = result | pCurrResourceConfig.second->Save(pWriter);
				result = result | pWriter->EndGroup();
			}
			result = result | pWriter->EndGroup();
		}

		result = result | pWriter->EndGroup();

		return RC_OK;
	}

	E_RESULT_CODE CResourcesRuntimeManifest::AddResourceMeta(const std::string& resourceId, std::unique_ptr<TBaseResourceParameters> pResourceMeta)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (resourceId.empty() || !pResourceMeta)
		{
			return RC_INVALID_ARGS;
		}

		mRuntimeToOriginalResourcesPathsTable[resourceId] = resourceId;

		auto it = mpResourcesMetaInfos.find(resourceId);
		if (it == mpResourcesMetaInfos.cend())
		{
			mpResourcesMetaInfos.emplace(resourceId, std::move(pResourceMeta));
			return RC_OK;
		}

		it->second = std::move(pResourceMeta);

		return RC_OK;
	}

	E_RESULT_CODE CResourcesRuntimeManifest::SetBaseResourcesPath(const std::string& value)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		mBaseResourcesPathPrefix = value;

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
			return nullptr;
		}

		return it->second.get();
	}


	TDE2_API IResourcesRuntimeManifest* CreateResourcesRuntimeManifest(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IResourcesRuntimeManifest, CResourcesRuntimeManifest, result);
	}
}