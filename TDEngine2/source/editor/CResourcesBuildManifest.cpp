#include "../../include/editor/CResourcesBuildManifest.h"
#include <functional>
#include "stringUtils.hpp"


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


	static std::unique_ptr<TResourceBuildInfo> Deserialize(IArchiveReader* pReader)
	{
		static const std::unordered_map<TypeId, std::function<std::unique_ptr<TResourceBuildInfo>()>> factories
		{
			{ TDE2_TYPE_ID(TMeshResourceBuildInfo), []() { return std::make_unique<TMeshResourceBuildInfo>(); } },
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
		if (!pResourceInfo)
		{
			return RC_INVALID_ARGS;
		}

		mpResourcesBuildConfigs.push_back(std::move(pResourceInfo));

		return RC_OK;
	}


	TDE2_API CResourcesBuildManifest* CreateResourcesBuildManifest(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(CResourcesBuildManifest, CResourcesBuildManifest, result);
	}
}

#endif