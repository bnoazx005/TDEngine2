#include "../../include/core/CProjectSettings.h"
#include "../../include/core/IFile.h"
#include "../../include/math/TVector3.h"
#include "../../include/metadata.h"
#include "stringUtils.hpp"


namespace TDEngine2
{
	struct TProjectSettingsArchiveKeys
	{
		static const std::string mCommonSettingsGroupId;
		static const std::string mGraphicsSettingsGroupId;
		static const std::string mAudioSettingsGroupId;
		static const std::string mLocalizationSettingsGroupId;
		static const std::string mWorldSettingsGroupId;
		static const std::string mSceneManagerSettingsGroupId;
		static const std::string mSplashScreenSettingsGroupId;
		static const std::string m3DPhysicsSettingsGroupId;
		static const std::string mQualityPresetsGroupId;

		struct TCommonSettingsKeys
		{
			static const std::string mApplicationIdKey;
			static const std::string mMaxThreadsCountKey;
			static const std::string mFlagsKey;
			static const std::string mAdditionalMountedDirectoriesKey;
			static const std::string mUserPluginsArrayKey;
			static const std::string mResourcesRuntimeManifestPathKeyId;
			static const std::string mBaseResourcesPathKeyId;
			static const std::string mDefaultPrefabsManifestPathKeyId;

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

		struct TSceneManagerSettingsKeys
		{
			static const std::string mMainLevelPathKey;
		};

		struct TSplashScreenSettingsKeys
		{
			static const std::string mIsEnabledKey;
			static const std::string mShowDurationKey;
		};

		struct T3DPhysicsSettingsKeys
		{
			static const std::string mGravityKey;
		};

		struct TQualityPresetKeys
		{
			static const std::string mShadowMapSizesKey;
			static const std::string mIsShadowMapEnabledKey;
		};
	};


	const std::string TProjectSettingsArchiveKeys::mCommonSettingsGroupId = "common_settings";
	const std::string TProjectSettingsArchiveKeys::mGraphicsSettingsGroupId = "graphics_settings";
	const std::string TProjectSettingsArchiveKeys::mAudioSettingsGroupId = "audio_settings";
	const std::string TProjectSettingsArchiveKeys::mLocalizationSettingsGroupId = "localization_settings";
	const std::string TProjectSettingsArchiveKeys::mWorldSettingsGroupId = "world_settings";
	const std::string TProjectSettingsArchiveKeys::mSceneManagerSettingsGroupId = "scenes_settings";
	const std::string TProjectSettingsArchiveKeys::mSplashScreenSettingsGroupId = "splash_screen_settings";
	const std::string TProjectSettingsArchiveKeys::m3DPhysicsSettingsGroupId = "3d_physics_settings";
	const std::string TProjectSettingsArchiveKeys::mQualityPresetsGroupId = "quality_presets";

	const std::string TProjectSettingsArchiveKeys::TCommonSettingsKeys::mApplicationIdKey = "application_id";
	const std::string TProjectSettingsArchiveKeys::TCommonSettingsKeys::mMaxThreadsCountKey = "max_worker_threads_count";
	const std::string TProjectSettingsArchiveKeys::TCommonSettingsKeys::mFlagsKey = "flags";
	const std::string TProjectSettingsArchiveKeys::TCommonSettingsKeys::mAdditionalMountedDirectoriesKey = "additional_mounted_dirs";
	const std::string TProjectSettingsArchiveKeys::TCommonSettingsKeys::mUserPluginsArrayKey = "user_plugins_list";
	const std::string TProjectSettingsArchiveKeys::TCommonSettingsKeys::mResourcesRuntimeManifestPathKeyId = "resources_runtime_manifest_path";
	const std::string TProjectSettingsArchiveKeys::TCommonSettingsKeys::mBaseResourcesPathKeyId = "base_resources_path";
	const std::string TProjectSettingsArchiveKeys::TCommonSettingsKeys::mDefaultPrefabsManifestPathKeyId = "default_prefabs_manifest_path";
	const std::string TProjectSettingsArchiveKeys::TCommonSettingsKeys::TFilepathAliasArchiveKeys::mAliasKey = "alias";
	const std::string TProjectSettingsArchiveKeys::TCommonSettingsKeys::TFilepathAliasArchiveKeys::mPathKey = "path";
	const std::string TProjectSettingsArchiveKeys::TCommonSettingsKeys::TPluginArchiveKeys::mId = "id";

	const std::string TProjectSettingsArchiveKeys::TGraphicsSettingsKeys::mGraphicsTypeKey = "renderer_plugin";
	const std::string TProjectSettingsArchiveKeys::TGraphicsSettingsKeys::mDefaultSkyboxMaterialKey = "default_skybox_mat_id";

	const std::string TProjectSettingsArchiveKeys::TAudioSettingsKeys::mAudioTypeKey = "audio_plugin";

	const std::string TProjectSettingsArchiveKeys::TLocalizationSettingsKeys::mCurrActiveLocaleKey = "current_locale";
	const std::string TProjectSettingsArchiveKeys::TLocalizationSettingsKeys::mRegisteredLocalesKey = "registered_locales";
	const std::string TProjectSettingsArchiveKeys::TLocalizationSettingsKeys::mLocaleInfoGroupId = "locale_info";
	const std::string TProjectSettingsArchiveKeys::TLocalizationSettingsKeys::mLocaleIdKey = "id";
	const std::string TProjectSettingsArchiveKeys::TLocalizationSettingsKeys::mLocalePackagePathKey = "package_path";

	const std::string TProjectSettingsArchiveKeys::TWorldSettingsKeys::mBoundsUpdateIntervalKey = "object_bounds_interval";

	const std::string TProjectSettingsArchiveKeys::TSceneManagerSettingsKeys::mMainLevelPathKey = "main_scene_path";

	const std::string TProjectSettingsArchiveKeys::TSplashScreenSettingsKeys::mIsEnabledKey = "enabled";
	const std::string TProjectSettingsArchiveKeys::TSplashScreenSettingsKeys::mShowDurationKey = "show_duration";

	const std::string TProjectSettingsArchiveKeys::T3DPhysicsSettingsKeys::mGravityKey = "gravity";

	const std::string TProjectSettingsArchiveKeys::TQualityPresetKeys::mShadowMapSizesKey = "shadow_maps_size";
	const std::string TProjectSettingsArchiveKeys::TQualityPresetKeys::mIsShadowMapEnabledKey = "shadow_maps_enabled";

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

			commonSettings.mPathToResourcesRuntimeManifest = pFileReader->GetString(
																	TProjectSettingsArchiveKeys::TCommonSettingsKeys::mResourcesRuntimeManifestPathKeyId, 
																	commonSettings.mPathToResourcesRuntimeManifest);

			commonSettings.mBaseResourcesPath = pFileReader->GetString(TProjectSettingsArchiveKeys::TCommonSettingsKeys::mBaseResourcesPathKeyId, 
																		commonSettings.mBaseResourcesPath);

			commonSettings.mPathToDefaultPrefabsManifest = pFileReader->GetString(TProjectSettingsArchiveKeys::TCommonSettingsKeys::mDefaultPrefabsManifestPathKeyId,
																					commonSettings.mPathToDefaultPrefabsManifest);

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
			graphicsSettings.mRendererPluginFilePath = pFileReader->GetString(TProjectSettingsArchiveKeys::TGraphicsSettingsKeys::mGraphicsTypeKey, graphicsSettings.mRendererPluginFilePath);

			const auto& skyboxMaterialId = pFileReader->GetString(TProjectSettingsArchiveKeys::TGraphicsSettingsKeys::mDefaultSkyboxMaterialKey);
			if (!skyboxMaterialId.empty())
			{
				graphicsSettings.mDefaultSkyboxMaterial = skyboxMaterialId;
			}
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
			audioSettings.mAudioPluginFilePath = pFileReader->GetString(TProjectSettingsArchiveKeys::TAudioSettingsKeys::mAudioTypeKey, audioSettings.mAudioPluginFilePath);
		}
		result = result | pFileReader->EndGroup();

		return result;
	}


	static E_RESULT_CODE LoadScenesSettings(IArchiveReader* pFileReader, CProjectSettings& projectSettings)
	{
		E_RESULT_CODE result = RC_OK;

		auto& scenesSettings = projectSettings.mScenesSettings;

		result = result | pFileReader->BeginGroup(TProjectSettingsArchiveKeys::mSceneManagerSettingsGroupId);
		{
			scenesSettings.mMainLevelScenePath = pFileReader->GetString(TProjectSettingsArchiveKeys::TSceneManagerSettingsKeys::mMainLevelPathKey, scenesSettings.mMainLevelScenePath);
		}
		result = result | pFileReader->EndGroup();

		return result;
	}


	static E_RESULT_CODE LoadSplashScreenSettings(IArchiveReader* pFileReader, CProjectSettings& projectSettings)
	{
		E_RESULT_CODE result = RC_OK;

		auto& settings = projectSettings.mSplashScreenSettings;

		result = result | pFileReader->BeginGroup(TProjectSettingsArchiveKeys::mSplashScreenSettingsGroupId);
		{
			settings.mIsEnabled = pFileReader->GetBool(TProjectSettingsArchiveKeys::TSplashScreenSettingsKeys::mIsEnabledKey, settings.mIsEnabled);
			settings.mShowDuration = pFileReader->GetFloat(TProjectSettingsArchiveKeys::TSplashScreenSettingsKeys::mShowDurationKey, settings.mShowDuration);
		}
		result = result | pFileReader->EndGroup();

		return result;
	}


	static E_RESULT_CODE Load3DPhysicsSettings(IArchiveReader* pFileReader, CProjectSettings& projectSettings)
	{
		E_RESULT_CODE result = RC_OK;

		auto& settings = projectSettings.m3DPhysicsSettings;

		result = result | pFileReader->BeginGroup(TProjectSettingsArchiveKeys::m3DPhysicsSettingsGroupId);
		{
			result = result | pFileReader->BeginGroup(TProjectSettingsArchiveKeys::T3DPhysicsSettingsKeys::mGravityKey);
			
			auto gravityResult = LoadVector3(pFileReader);
			if (gravityResult.HasError())
			{
				return gravityResult.GetError();
			}

			const TVector3& gravity = gravityResult.Get();
			if (Length(gravity) > FloatEpsilon)
			{
				settings.mGravity = gravity;
			}

			result = result | pFileReader->EndGroup();
		}
		result = result | pFileReader->EndGroup();

		return result;
	}


	static E_RESULT_CODE LoadQualityPresets(IArchiveReader* pFileReader, CProjectSettings& projectSettings)
	{
		E_RESULT_CODE result = RC_OK;

		auto& qualityPresets = projectSettings.mQualityPresets;

		auto&& enumFields = Meta::EnumTrait<E_QUALITY_PRESET_TYPE>::GetFields(); /// \fixme Reflection in production version should not be used at all

		result = result | pFileReader->BeginGroup(TProjectSettingsArchiveKeys::mQualityPresetsGroupId);
		{
			for (auto&& currEnumField : enumFields)
			{
				if (E_QUALITY_PRESET_TYPE::PRESETS_COUNT == currEnumField.value)
				{
					continue;
				}

				auto& currPreset = qualityPresets[static_cast<U32>(currEnumField.value)];

				result = result | pFileReader->BeginGroup(currEnumField.name);

				currPreset.mIsShadowMappingEnabled = pFileReader->GetBool(TProjectSettingsArchiveKeys::TQualityPresetKeys::mIsShadowMapEnabledKey, true);
				currPreset.mShadowMapSizes = pFileReader->GetUInt32(TProjectSettingsArchiveKeys::TQualityPresetKeys::mShadowMapSizesKey, 1024);

				result = result | pFileReader->EndGroup();
			}
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
		result = result | LoadScenesSettings(pFileReader, *this);
		result = result | LoadSplashScreenSettings(pFileReader, *this);
		result = result | Load3DPhysicsSettings(pFileReader, *this);
		result = result | LoadQualityPresets(pFileReader, *this);
		
		return RC_OK;
	}

	TDE2_API TPtr<CProjectSettings> CProjectSettings::Get()
	{
		static TPtr<CProjectSettings> pInstance = TPtr<CProjectSettings>(new (std::nothrow) CProjectSettings());
		return pInstance;
	}
}