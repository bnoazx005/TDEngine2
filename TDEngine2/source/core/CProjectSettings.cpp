#include "../../include/core/CProjectSettings.h"
#include "../../include/core/IFile.h"
#include "../../include/metadata.h"


namespace TDEngine2
{
	struct TProjectSettingsArchiveKeys
	{
		static const std::string mCommonSettingsGroupId;
		static const std::string mGraphicsSettingsGroupId;
		static const std::string mAudioSettingsGroupId;
		static const std::string mLocalizationSettingsGroupId;
		static const std::string mWorldSettingsGroupId;

		struct TCommonSettingsKeys
		{
			static const std::string mApplicationIdKey;
			static const std::string mMaxThreadsCountKey;
			static const std::string mFlagsKey;
			static const std::string mAdditionalMountedDirectoriesKey;
			static const std::string mUserPluginsArrayKey;

			struct TFilepathAliasArchiveKeys
			{
				static const std::string mPathKey;
				static const std::string mAliasKey;
			};

			struct TPluginArchiveKeys
			{
				static const std::string mId;
			};
		};

		struct TGraphicsSettingsKeys
		{
			static const std::string mGraphicsTypeKey;
			static const std::string mRendererSettingsGroupKey;
			static const std::string mDefaultSkyboxMaterialKey;

			struct TRendererSettingsKeys
			{
				static const std::string mShadowMapSizesKey;
				static const std::string mIsShadowMapEnabledKey;
			};
		};

		struct TAudioSettingsKeys
		{
			static const std::string mAudioTypeKey;
		};

		struct TLocalizationSettingsKeys
		{
			static const std::string mCurrActiveLocaleKey;
			static const std::string mRegisteredLocalesKey;
			static const std::string mLocaleInfoGroupId;
			static const std::string mLocaleIdKey;
			static const std::string mLocalePackagePathKey;
		};

		struct TWorldSettingsKeys
		{
			static const std::string mBoundsUpdateIntervalKey;
		};
	};

	const std::string TProjectSettingsArchiveKeys::mCommonSettingsGroupId = "common_settings";
	const std::string TProjectSettingsArchiveKeys::mGraphicsSettingsGroupId = "graphics_settings";
	const std::string TProjectSettingsArchiveKeys::mAudioSettingsGroupId = "audio_settings";
	const std::string TProjectSettingsArchiveKeys::mLocalizationSettingsGroupId = "localization_settings";
	const std::string TProjectSettingsArchiveKeys::mWorldSettingsGroupId = "world_settings";

	const std::string TProjectSettingsArchiveKeys::TCommonSettingsKeys::mApplicationIdKey = "application_id";
	const std::string TProjectSettingsArchiveKeys::TCommonSettingsKeys::mMaxThreadsCountKey = "max_worker_threads_count";
	const std::string TProjectSettingsArchiveKeys::TCommonSettingsKeys::mFlagsKey = "flags";
	const std::string TProjectSettingsArchiveKeys::TCommonSettingsKeys::mAdditionalMountedDirectoriesKey = "additional_mounted_dirs";
	const std::string TProjectSettingsArchiveKeys::TCommonSettingsKeys::mUserPluginsArrayKey = "user_plugins_list";
	const std::string TProjectSettingsArchiveKeys::TCommonSettingsKeys::TFilepathAliasArchiveKeys::mAliasKey = "alias";
	const std::string TProjectSettingsArchiveKeys::TCommonSettingsKeys::TFilepathAliasArchiveKeys::mPathKey = "path";
	const std::string TProjectSettingsArchiveKeys::TCommonSettingsKeys::TPluginArchiveKeys::mId = "id";

	const std::string TProjectSettingsArchiveKeys::TGraphicsSettingsKeys::mGraphicsTypeKey = "gapi_type";
	const std::string TProjectSettingsArchiveKeys::TGraphicsSettingsKeys::mRendererSettingsGroupKey = "renderer_settings";
	const std::string TProjectSettingsArchiveKeys::TGraphicsSettingsKeys::mDefaultSkyboxMaterialKey = "default_skybox_mat_id";
	const std::string TProjectSettingsArchiveKeys::TGraphicsSettingsKeys::TRendererSettingsKeys::mShadowMapSizesKey = "shadow_maps_size";
	const std::string TProjectSettingsArchiveKeys::TGraphicsSettingsKeys::TRendererSettingsKeys::mIsShadowMapEnabledKey = "shadow_maps_enabled";

	const std::string TProjectSettingsArchiveKeys::TAudioSettingsKeys::mAudioTypeKey = "api_type";

	const std::string TProjectSettingsArchiveKeys::TLocalizationSettingsKeys::mCurrActiveLocaleKey = "current_locale";
	const std::string TProjectSettingsArchiveKeys::TLocalizationSettingsKeys::mRegisteredLocalesKey = "registered_locales";
	const std::string TProjectSettingsArchiveKeys::TLocalizationSettingsKeys::mLocaleInfoGroupId = "locale_info";
	const std::string TProjectSettingsArchiveKeys::TLocalizationSettingsKeys::mLocaleIdKey = "id";
	const std::string TProjectSettingsArchiveKeys::TLocalizationSettingsKeys::mLocalePackagePathKey = "package_path";

	const std::string TProjectSettingsArchiveKeys::TWorldSettingsKeys::mBoundsUpdateIntervalKey = "object_bounds_interval";


	CProjectSettings::CProjectSettings():
		CBaseObject()
	{
	}


	static E_RESULT_CODE LoadCommonSettings(IArchiveReader* pFileReader, CProjectSettings& projectSettings)
	{
		E_RESULT_CODE result = RC_OK;

		auto& commonSettings = projectSettings.mCommonSettings;

		result = result | pFileReader->BeginGroup(TProjectSettingsArchiveKeys::mCommonSettingsGroupId);
		{
			commonSettings.mApplicationName = pFileReader->GetString(TProjectSettingsArchiveKeys::TCommonSettingsKeys::mApplicationIdKey);
			commonSettings.mMaxNumOfWorkerThreads = pFileReader->GetUInt32(TProjectSettingsArchiveKeys::TCommonSettingsKeys::mMaxThreadsCountKey);
			commonSettings.mFlags = pFileReader->GetUInt32(TProjectSettingsArchiveKeys::TCommonSettingsKeys::mFlagsKey);

			/// \note Read user's defined directories
			result = result | pFileReader->BeginGroup(TProjectSettingsArchiveKeys::TCommonSettingsKeys::mAdditionalMountedDirectoriesKey);
			{
				while (pFileReader->HasNextItem())
				{
					pFileReader->BeginGroup(Wrench::StringUtils::GetEmptyStr());
					
					CProjectSettings::TCommonSettings::TFilesystemPathAlias aliasInfo;

					aliasInfo.mAlias = pFileReader->GetString(TProjectSettingsArchiveKeys::TCommonSettingsKeys::TFilepathAliasArchiveKeys::mAliasKey);
					aliasInfo.mPath  = pFileReader->GetString(TProjectSettingsArchiveKeys::TCommonSettingsKeys::TFilepathAliasArchiveKeys::mPathKey);

					commonSettings.mAdditionalMountedDirectories.emplace_back(aliasInfo);
					
					pFileReader->EndGroup();
				}
			}
			result = result | pFileReader->EndGroup();

			/// \note Read user plugin's identifiers
			result = result | pFileReader->BeginGroup(TProjectSettingsArchiveKeys::TCommonSettingsKeys::mUserPluginsArrayKey);
			{
				while (pFileReader->HasNextItem())
				{
					pFileReader->BeginGroup(Wrench::StringUtils::GetEmptyStr());

					commonSettings.mUserPluginsToLoad.push_back(pFileReader->GetString(TProjectSettingsArchiveKeys::TCommonSettingsKeys::TPluginArchiveKeys::mId));

					pFileReader->EndGroup();
				}
			}
			result = result | pFileReader->EndGroup();
		}
		result = result | pFileReader->EndGroup();

		return result;
	}


	static E_RESULT_CODE LoadGraphicsSettings(IArchiveReader* pFileReader, CProjectSettings& projectSettings)
	{
		E_RESULT_CODE result = RC_OK;

		auto& graphicsSettings = projectSettings.mGraphicsSettings;

		result = result | pFileReader->BeginGroup(TProjectSettingsArchiveKeys::mGraphicsSettingsGroupId);
		{
			graphicsSettings.mGraphicsContextType = Meta::EnumTrait<E_GRAPHICS_CONTEXT_GAPI_TYPE>::FromString(pFileReader->GetString(TProjectSettingsArchiveKeys::TGraphicsSettingsKeys::mGraphicsTypeKey));

			const auto& skyboxMaterialId = pFileReader->GetString(TProjectSettingsArchiveKeys::TGraphicsSettingsKeys::mDefaultSkyboxMaterialKey);
			if (!skyboxMaterialId.empty())
			{
				graphicsSettings.mDefaultSkyboxMaterial = skyboxMaterialId;
			}

			/// \note Shadow maps settings
			result = result | pFileReader->BeginGroup(TProjectSettingsArchiveKeys::TGraphicsSettingsKeys::mRendererSettingsGroupKey);
			{
				graphicsSettings.mRendererSettings.mShadowMapSizes = pFileReader->GetUInt32(TProjectSettingsArchiveKeys::TGraphicsSettingsKeys::TRendererSettingsKeys::mShadowMapSizesKey);
				graphicsSettings.mRendererSettings.mIsShadowMappingEnabled = pFileReader->GetBool(TProjectSettingsArchiveKeys::TGraphicsSettingsKeys::TRendererSettingsKeys::mIsShadowMapEnabledKey);
			}
			result = result | pFileReader->EndGroup();
		}
		result = result | pFileReader->EndGroup();

		return result;
	}


	static E_RESULT_CODE LoadLocaleSettings(IArchiveReader* pFileReader, CProjectSettings& projectSettings)
	{
		E_RESULT_CODE result = RC_OK;

		auto& localizationSettings = projectSettings.mLocalizationSettings;

		result = result | pFileReader->BeginGroup(TProjectSettingsArchiveKeys::mLocalizationSettingsGroupId);
		{
			pFileReader->BeginGroup(TProjectSettingsArchiveKeys::TLocalizationSettingsKeys::mRegisteredLocalesKey);
			{
				CProjectSettings::TLocalizationSettings::TLocaleInfo info;

				auto& registeredLocales = localizationSettings.mRegisteredLocales;

				while (pFileReader->HasNextItem())
				{
					pFileReader->BeginGroup(Wrench::StringUtils::GetEmptyStr());
					{
						pFileReader->BeginGroup(TProjectSettingsArchiveKeys::TLocalizationSettingsKeys::mLocaleInfoGroupId);

						info.mName        = pFileReader->GetString(TProjectSettingsArchiveKeys::TLocalizationSettingsKeys::mLocaleIdKey);
						info.mPackagePath = pFileReader->GetString(TProjectSettingsArchiveKeys::TLocalizationSettingsKeys::mLocalePackagePathKey);
						info.mId          = static_cast<TLocaleId>(registeredLocales.size());

						registeredLocales.emplace_back(info);

						pFileReader->EndGroup();
					}
					pFileReader->EndGroup();
				}
			}
			pFileReader->EndGroup();

			localizationSettings.mCurrActiveLocaleId = pFileReader->GetString(TProjectSettingsArchiveKeys::TLocalizationSettingsKeys::mCurrActiveLocaleKey);
		}
		result = result | pFileReader->EndGroup();

		return result;
	}


	static E_RESULT_CODE LoadAudioSettings(IArchiveReader* pFileReader, CProjectSettings& projectSettings)
	{
		E_RESULT_CODE result = RC_OK;

		auto& audioSettings = projectSettings.mAudioSettings;

		result = result | pFileReader->BeginGroup(TProjectSettingsArchiveKeys::mAudioSettingsGroupId);
		{
			audioSettings.mAudioContextType = Meta::EnumTrait<E_AUDIO_CONTEXT_API_TYPE>::FromString(pFileReader->GetString(TProjectSettingsArchiveKeys::TAudioSettingsKeys::mAudioTypeKey));
		}
		result = result | pFileReader->EndGroup();

		return result;
	}


	E_RESULT_CODE CProjectSettings::Init(IArchiveReader* pFileReader)
	{
		if (!pFileReader)
		{
			return RC_INVALID_ARGS;
		}

		E_RESULT_CODE result = RC_OK;

		result = result | LoadCommonSettings(pFileReader, *this);	
		result = result | LoadGraphicsSettings(pFileReader, *this);	
		result = result | LoadAudioSettings(pFileReader, *this);	
		result = result | LoadLocaleSettings(pFileReader, *this);
		
		return RC_OK;
	}

	TDE2_API TPtr<CProjectSettings> CProjectSettings::Get()
	{
		static TPtr<CProjectSettings> pInstance = TPtr<CProjectSettings>(new (std::nothrow) CProjectSettings());
		return pInstance;
	}
}