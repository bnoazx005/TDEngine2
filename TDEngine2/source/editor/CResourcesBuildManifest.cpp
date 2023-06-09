#include "../../include/editor/CResourcesBuildManifest.h"
#include "../../include/core/IFileSystem.h"
#include "../../include/core/IFile.h"
#include <functional>
#include <algorithm>
#include "stringUtils.hpp"
#define META_EXPORT_GRAPHICS_SECTION
#include "../../include/metadata.h"

#if _HAS_CXX17
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	struct TResourcesBuildManifestArchiveKeys
	{
		static const std::string mResourcesCollectionKeyId;
		static const std::string mSingleResourceKeyId;
	};


	const std::string TResourcesBuildManifestArchiveKeys::mResourcesCollectionKeyId = "resources_collection";
	const std::string TResourcesBuildManifestArchiveKeys::mSingleResourceKeyId = "resource_info";


	struct TResourceBuildInfoArchiveKeys
	{
		static const std::string mResourcePath;
	};


	const std::string TResourceBuildInfoArchiveKeys::mResourcePath = "path";


	struct TMeshResourceBuildInfoArchiveKeys
	{
		static const std::string mIsSkinnedMeshKeyId;
		static const std::string mImportTangentsKeyId;
		static const std::string mAnimationsImportKeyId;
		static const std::string mSingleAnimationImportKeyId;

		struct TAnimationImportKeys
		{
			static const std::string mOutputAnimationPathKeyId;
			static const std::string mIsLoopedKeyId;
			static const std::string mStartRangeKeyId;
			static const std::string mEndRangeKeyId;
		};
	};


	const std::string TMeshResourceBuildInfoArchiveKeys::mIsSkinnedMeshKeyId = "is_skinned_mesh";
	const std::string TMeshResourceBuildInfoArchiveKeys::mImportTangentsKeyId = "import_tangents";
	const std::string TMeshResourceBuildInfoArchiveKeys::mAnimationsImportKeyId = "animation_clips";
	const std::string TMeshResourceBuildInfoArchiveKeys::mSingleAnimationImportKeyId = "animation_info";

	const std::string TMeshResourceBuildInfoArchiveKeys::TAnimationImportKeys::mOutputAnimationPathKeyId = "output_animation_clip_path";
	const std::string TMeshResourceBuildInfoArchiveKeys::TAnimationImportKeys::mIsLoopedKeyId = "is_looped";
	const std::string TMeshResourceBuildInfoArchiveKeys::TAnimationImportKeys::mStartRangeKeyId = "start_range";
	const std::string TMeshResourceBuildInfoArchiveKeys::TAnimationImportKeys::mEndRangeKeyId = "end_range";


	struct TTexture2DResourceBuildInfoArchiveKeys
	{
		static const std::string mFilterTypeKeyId;
		static const std::string mAddressModeKeyId;
		static const std::string mGenerateMipMapsKeyId;
		static const std::string mUseHDRFormatKeyId;
	};


	const std::string TTexture2DResourceBuildInfoArchiveKeys::mFilterTypeKeyId = "filter_type";
	const std::string TTexture2DResourceBuildInfoArchiveKeys::mAddressModeKeyId = "address_mode";
	const std::string TTexture2DResourceBuildInfoArchiveKeys::mGenerateMipMapsKeyId = "mipmaps_generation";
	const std::string TTexture2DResourceBuildInfoArchiveKeys::mUseHDRFormatKeyId = "hdr_format";


	static std::unique_ptr<TResourceBuildInfo> Deserialize(IArchiveReader* pReader)
	{
		static const std::unordered_map<TypeId, std::function<std::unique_ptr<TResourceBuildInfo>()>> factories
		{
			{ TDE2_TYPE_ID(TMeshResourceBuildInfo), []() { return std::make_unique<TMeshResourceBuildInfo>(); } },
			{ TDE2_TYPE_ID(TTexture2DResourceBuildInfo), []() { return std::make_unique<TTexture2DResourceBuildInfo>(); } },
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


	/*!
		\brief TResourceBuildInfo's definition
	*/

	E_RESULT_CODE TResourceBuildInfo::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_INVALID_ARGS;
		}

		mRelativePathToResource = pReader->GetString(TResourceBuildInfoArchiveKeys::mResourcePath);

		return RC_OK;
	}

	E_RESULT_CODE TResourceBuildInfo::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_INVALID_ARGS;
		}

		return pWriter->SetString(TResourceBuildInfoArchiveKeys::mResourcePath, mRelativePathToResource);
	}


	/*!
		\brief TMeshResourceBuildInfo's definition
	*/

	E_RESULT_CODE TMeshResourceBuildInfo::Load(IArchiveReader* pReader)
	{
		E_RESULT_CODE result = TResourceBuildInfo::Load(pReader);

		mIsSkinned = pReader->GetBool(TMeshResourceBuildInfoArchiveKeys::mIsSkinnedMeshKeyId);
		mImportTangents = pReader->GetBool(TMeshResourceBuildInfoArchiveKeys::mImportTangentsKeyId);

		result = result | pReader->BeginGroup(TMeshResourceBuildInfoArchiveKeys::mAnimationsImportKeyId);
		{
			while (pReader->HasNextItem())
			{
				result = result | pReader->BeginGroup(Wrench::StringUtils::GetEmptyStr());
				{
					result = result | pReader->BeginGroup(TMeshResourceBuildInfoArchiveKeys::mSingleAnimationImportKeyId);

					TAnimationClipImportInfo clipImportInfo;

					clipImportInfo.mOutputAnimationPath = pReader->GetString(TMeshResourceBuildInfoArchiveKeys::TAnimationImportKeys::mOutputAnimationPathKeyId);
					clipImportInfo.mIsLooped = pReader->GetBool(TMeshResourceBuildInfoArchiveKeys::TAnimationImportKeys::mIsLoopedKeyId);
					clipImportInfo.mStartRange = pReader->GetUInt32(TMeshResourceBuildInfoArchiveKeys::TAnimationImportKeys::mStartRangeKeyId);
					clipImportInfo.mEndRange = pReader->GetUInt32(TMeshResourceBuildInfoArchiveKeys::TAnimationImportKeys::mEndRangeKeyId);

					mAnimations.emplace_back(clipImportInfo);

					result = result | pReader->EndGroup();
				}
				result = result | pReader->EndGroup();
			}
		}
		result = result | pReader->EndGroup();

		return result;
	}

	E_RESULT_CODE TMeshResourceBuildInfo::Save(IArchiveWriter* pWriter)
	{
		E_RESULT_CODE result = TResourceBuildInfo::Save(pWriter);

		result = result | pWriter->SetUInt32("type_id", static_cast<U32>(TDE2_TYPE_ID(TMeshResourceBuildInfo)));

		result = result | pWriter->SetBool(TMeshResourceBuildInfoArchiveKeys::mIsSkinnedMeshKeyId, mIsSkinned);
		result = result | pWriter->SetBool(TMeshResourceBuildInfoArchiveKeys::mImportTangentsKeyId, mImportTangents);

		result = result | pWriter->BeginGroup(TMeshResourceBuildInfoArchiveKeys::mAnimationsImportKeyId, true);
		{
			for (auto&& currAnimationClipInfo : mAnimations)
			{
				result = result | pWriter->BeginGroup(Wrench::StringUtils::GetEmptyStr(), false);
				{
					result = result | pWriter->BeginGroup(TMeshResourceBuildInfoArchiveKeys::mSingleAnimationImportKeyId, false);					

					pWriter->SetString(TMeshResourceBuildInfoArchiveKeys::TAnimationImportKeys::mOutputAnimationPathKeyId, currAnimationClipInfo.mOutputAnimationPath);
					pWriter->SetBool(TMeshResourceBuildInfoArchiveKeys::TAnimationImportKeys::mIsLoopedKeyId, currAnimationClipInfo.mIsLooped);
					pWriter->SetUInt32(TMeshResourceBuildInfoArchiveKeys::TAnimationImportKeys::mStartRangeKeyId, currAnimationClipInfo.mStartRange);
					pWriter->SetUInt32(TMeshResourceBuildInfoArchiveKeys::TAnimationImportKeys::mEndRangeKeyId, currAnimationClipInfo.mEndRange);

					result = result | pWriter->EndGroup();
				}
				result = result | pWriter->EndGroup();
			}
		}
		result = result | pWriter->EndGroup();

		return result;
	}


	/*!
		\brief TTexture2DResourceBuildInfo's definition
	*/

	E_RESULT_CODE TTexture2DResourceBuildInfo::Load(IArchiveReader* pReader)
	{
		E_RESULT_CODE result = TResourceBuildInfo::Load(pReader);

		mFilteringType = Meta::EnumTrait<E_TEXTURE_FILTER_TYPE>::FromString(pReader->GetString(TTexture2DResourceBuildInfoArchiveKeys::mFilterTypeKeyId));
		mAddressMode = Meta::EnumTrait<E_ADDRESS_MODE_TYPE>::FromString(pReader->GetString(TTexture2DResourceBuildInfoArchiveKeys::mAddressModeKeyId));
		mGenerateMipMaps = pReader->GetBool(TTexture2DResourceBuildInfoArchiveKeys::mGenerateMipMapsKeyId);
		mIsDynamicRangeEnabled = pReader->GetBool(TTexture2DResourceBuildInfoArchiveKeys::mUseHDRFormatKeyId, false);

		return result;
	}

	E_RESULT_CODE TTexture2DResourceBuildInfo::Save(IArchiveWriter* pWriter)
	{
		E_RESULT_CODE result = TResourceBuildInfo::Save(pWriter);

		result = result | pWriter->SetUInt32("type_id", static_cast<U32>(TDE2_TYPE_ID(TTexture2DResourceBuildInfo)));

		result = result | pWriter->SetString(TTexture2DResourceBuildInfoArchiveKeys::mFilterTypeKeyId, Meta::EnumTrait<E_TEXTURE_FILTER_TYPE>::ToString(mFilteringType));
		result = result | pWriter->SetString(TTexture2DResourceBuildInfoArchiveKeys::mAddressModeKeyId, Meta::EnumTrait<E_ADDRESS_MODE_TYPE>::ToString(mAddressMode));
		result = result | pWriter->SetBool(TTexture2DResourceBuildInfoArchiveKeys::mGenerateMipMapsKeyId, mGenerateMipMaps);
		result = result | pWriter->SetBool(TTexture2DResourceBuildInfoArchiveKeys::mUseHDRFormatKeyId, mIsDynamicRangeEnabled);

		return result;
	}


	/*!
		\brief CResourcesBuildManifest's definition
	*/

	CResourcesBuildManifest::CResourcesBuildManifest() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CResourcesBuildManifest::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CResourcesBuildManifest::Load(IArchiveReader* pReader)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (!pReader)
		{
			return RC_INVALID_ARGS;
		}

		E_RESULT_CODE result = pReader->BeginGroup(TResourcesBuildManifestArchiveKeys::mResourcesCollectionKeyId);

		while (pReader->HasNextItem())
		{
			result = result | pReader->BeginGroup(Wrench::StringUtils::GetEmptyStr());
			{
				result = result | pReader->BeginGroup(TResourcesBuildManifestArchiveKeys::mSingleResourceKeyId);
				
				mpResourcesBuildConfigs.emplace_back(Deserialize(pReader));

				result = result | pReader->EndGroup();
			}
			result = result | pReader->EndGroup();
		}

		result = result | pReader->EndGroup();

		return RC_OK;
	}

	E_RESULT_CODE CResourcesBuildManifest::Save(IArchiveWriter* pWriter)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (!pWriter)
		{
			return RC_INVALID_ARGS;
		}

		E_RESULT_CODE result = RC_OK;

		result = result | pWriter->BeginGroup("meta");
		{
			result = result | pWriter->SetString("resource_type", "resources_build_manifest");
			result = result | pWriter->SetUInt16("version_tag", mVersionTag);
		}
		result = result | pWriter->EndGroup();

		result = result | pWriter->BeginGroup(TResourcesBuildManifestArchiveKeys::mResourcesCollectionKeyId, true);
		
		for (auto&& pCurrResourceConfig : mpResourcesBuildConfigs)
		{
			result = result | pWriter->BeginGroup(Wrench::StringUtils::GetEmptyStr(), false);
			{
				result = result | pWriter->BeginGroup(TResourcesBuildManifestArchiveKeys::mSingleResourceKeyId, false);
				result = result | pCurrResourceConfig->Save(pWriter);
				result = result | pWriter->EndGroup();
			}
			result = result | pWriter->EndGroup();
		}

		result = result | pWriter->EndGroup();

		return RC_OK;
	}

	E_RESULT_CODE CResourcesBuildManifest::AddResourceBuildInfo(std::unique_ptr<TResourceBuildInfo> pResourceInfo)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (!pResourceInfo)
		{
			return RC_INVALID_ARGS;
		}

		const std::string& id = pResourceInfo->mRelativePathToResource;

		auto it = std::find_if(mpResourcesBuildConfigs.begin(), mpResourcesBuildConfigs.end(), [&id](auto&& entity) { return entity->mRelativePathToResource == id; });
		if (it == mpResourcesBuildConfigs.end())
		{
			mpResourcesBuildConfigs.push_back(std::move(pResourceInfo));
			return RC_OK;
		}

		*it = std::move(pResourceInfo);

		return RC_OK;
	}

	E_RESULT_CODE CResourcesBuildManifest::RemoveResourceBuildInfo(const std::string& relativePath)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		auto it = std::find_if(mpResourcesBuildConfigs.begin(), mpResourcesBuildConfigs.end(), [&relativePath](auto&& entity) { return entity->mRelativePathToResource == relativePath; });
		if (it == mpResourcesBuildConfigs.end())
		{
			return RC_FAIL;
		}

		mpResourcesBuildConfigs.erase(it);

		return RC_OK;
	}

	TResourceBuildInfo* CResourcesBuildManifest::FindResourceBuildInfo(const std::string& relativePath)
	{
		const std::string path = Wrench::StringUtils::ReplaceAll(relativePath, mBaseFilePath, ".");

		auto&& it = std::find_if(mpResourcesBuildConfigs.begin(), mpResourcesBuildConfigs.end(), [&path](auto&& pInfo) { return fs::path(pInfo->mRelativePathToResource).string() == path; });
		return it == mpResourcesBuildConfigs.end() ? nullptr : it->get();
	}

	E_RESULT_CODE CResourcesBuildManifest::ForEachRegisteredResource(const std::function<bool(const TResourceBuildInfo&)>& action)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (!action)
		{
			return RC_INVALID_ARGS;
		}

		for (auto&& pCurrResourceInfo : mpResourcesBuildConfigs)
		{
			if (!action(*pCurrResourceInfo))
			{
				return RC_OK;
			}
		}

		return RC_OK;
	}

	E_RESULT_CODE CResourcesBuildManifest::SetBaseResourcesPath(const std::string& value)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		mBaseFilePath = value;

		return RC_OK;
	}

	const std::string& CResourcesBuildManifest::GetBaseResourcesPath() const
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return mBaseFilePath;
	}

	E_RESULT_CODE CResourcesBuildManifest::_forEachTypedResource(TypeId resourceTypeId, const TResourceInfoVisitFunctior& action)
	{
		if (!action)
		{
			return RC_INVALID_ARGS;
		}

		for (auto&& pCurrResourceInfo : mpResourcesBuildConfigs)
		{
			if (pCurrResourceInfo->GetResourceTypeId() != resourceTypeId)
			{
				continue;
			}

			if (!action(*pCurrResourceInfo))
			{
				return RC_OK;
			}
		}

		return RC_OK;
	}


	TDE2_API CResourcesBuildManifest* CreateResourcesBuildManifest(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(CResourcesBuildManifest, CResourcesBuildManifest, result);
	}


	TDE2_API TPtr<CResourcesBuildManifest> LoadResourcesBuildManifest(TPtr<IFileSystem> pFileSystem, const std::string& path)
	{
		E_RESULT_CODE result = RC_OK;

		TPtr<CResourcesBuildManifest> pResourcesManifest = TPtr<CResourcesBuildManifest>(CreateResourcesBuildManifest(result));
		TDE2_ASSERT(RC_OK == result);

		if (pResourcesManifest)
		{
			auto openFileResult = pFileSystem->Open<IYAMLFileReader>(path);
			if (openFileResult.HasError())
			{
				return nullptr;
			}

			if (auto pArchiveReader = pFileSystem->Get<IYAMLFileReader>(openFileResult.Get()))
			{
				result = pResourcesManifest->Load(pArchiveReader);
				TDE2_ASSERT(RC_OK == result);

				pResourcesManifest->SetBaseResourcesPath(fs::path(path).parent_path().string());

				pArchiveReader->Close();
			}
		}

		return pResourcesManifest;
	}


	TDE2_API std::unique_ptr<TResourceBuildInfo> CreateResourceBuildInfoForFilePath(const std::string& path)
	{
		std::string resourceFileExtension;
		std::transform(path.cbegin(), path.cend(), std::back_inserter(resourceFileExtension), [](auto&& ch) { return std::tolower(ch); });

		if (Wrench::StringUtils::EndsWith(resourceFileExtension, "fbx") ||
			Wrench::StringUtils::EndsWith(resourceFileExtension, "obj") || 
			Wrench::StringUtils::EndsWith(resourceFileExtension, "dae"))
		{
			return std::make_unique<TMeshResourceBuildInfo>();
		}

		if (Wrench::StringUtils::EndsWith(resourceFileExtension, "png") ||
			Wrench::StringUtils::EndsWith(resourceFileExtension, "jpg") ||
			Wrench::StringUtils::EndsWith(resourceFileExtension, "tga") ||
			Wrench::StringUtils::EndsWith(resourceFileExtension, "hdr"))
		{
			return std::make_unique<TTexture2DResourceBuildInfo>();
		}

		TDE2_UNREACHABLE();
		return nullptr;
	}
}

#endif