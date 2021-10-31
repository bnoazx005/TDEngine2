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

		struct TCommonSettingsKeys
		{
			static const std::string mApplicationIdKey;
			static const std::string mMaxThreadsCountKey;
			static const std::string mMemoryAllocationSizeKey;
			static const std::string mFlagsKey;
		};

		struct TGraphicsSettingsKeys
		{
			static const std::string mGraphicsTypeKey;
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
	};

	const std::string TProjectSettingsArchiveKeys::mCommonSettingsGroupId = "common_settings";
	const std::string TProjectSettingsArchiveKeys::mGraphicsSettingsGroupId = "graphics_settings";
	const std::string TProjectSettingsArchiveKeys::mAudioSettingsGroupId = "audio_settings";
	const std::string TProjectSettingsArchiveKeys::mLocalizationSettingsGroupId = "localization_settings";

	const std::string TProjectSettingsArchiveKeys::TCommonSettingsKeys::mApplicationIdKey = "application_id";
	const std::string TProjectSettingsArchiveKeys::TCommonSettingsKeys::mMaxThreadsCountKey = "max_worker_threads_count";
	const std::string TProjectSettingsArchiveKeys::TCommonSettingsKeys::mMemoryAllocationSizeKey = "total_preallocated_memory_size";
	const std::string TProjectSettingsArchiveKeys::TCommonSettingsKeys::mFlagsKey = "flags";

	const std::string TProjectSettingsArchiveKeys::TGraphicsSettingsKeys::mGraphicsTypeKey = "gapi_type";

	const std::string TProjectSettingsArchiveKeys::TAudioSettingsKeys::mAudioTypeKey = "api_type";

	const std::string TProjectSettingsArchiveKeys::TLocalizationSettingsKeys::mCurrActiveLocaleKey = "current_locale";
	const std::string TProjectSettingsArchiveKeys::TLocalizationSettingsKeys::mRegisteredLocalesKey = "registered_locales";
	const std::string TProjectSettingsArchiveKeys::TLocalizationSettingsKeys::mLocaleInfoGroupId = "locale_info";
	const std::string TProjectSettingsArchiveKeys::TLocalizationSettingsKeys::mLocaleIdKey = "id";
	const std::string TProjectSettingsArchiveKeys::TLocalizationSettingsKeys::mLocalePackagePathKey = "package_path";


	CProjectSettings::CProjectSettings():
		CBaseObject()
	{
	}

	E_RESULT_CODE CProjectSettings::Init(IArchiveReader* pFileReader)
	{
		if (!pFileReader)
		{
			return RC_INVALID_ARGS;
		}

		E_RESULT_CODE result = RC_OK;

		/// \note Core settings
		result = result | pFileReader->BeginGroup(TProjectSettingsArchiveKeys::mCommonSettingsGroupId);
		{
			mCommonSettings.mApplicationName = pFileReader->GetString(TProjectSettingsArchiveKeys::TCommonSettingsKeys::mApplicationIdKey);
			mCommonSettings.mMaxNumOfWorkerThreads = pFileReader->GetUInt32(TProjectSettingsArchiveKeys::TCommonSettingsKeys::mMaxThreadsCountKey);
			mCommonSettings.mTotalPreallocatedMemorySize = pFileReader->GetUInt32(TProjectSettingsArchiveKeys::TCommonSettingsKeys::mMemoryAllocationSizeKey);
			mCommonSettings.mFlags = pFileReader->GetUInt32(TProjectSettingsArchiveKeys::TCommonSettingsKeys::mFlagsKey);
		}
		result = result | pFileReader->EndGroup();

		/// \note Graphics settings
		result = result | pFileReader->BeginGroup(TProjectSettingsArchiveKeys::mGraphicsSettingsGroupId);
		{
			mGraphicsSettings.mGraphicsContextType = Meta::EnumTrait<E_GRAPHICS_CONTEXT_GAPI_TYPE>::FromString(pFileReader->GetString(TProjectSettingsArchiveKeys::TGraphicsSettingsKeys::mGraphicsTypeKey));
		}
		result = result | pFileReader->EndGroup();

		/// \note Audio setiings
		result = result | pFileReader->BeginGroup(TProjectSettingsArchiveKeys::mAudioSettingsGroupId);
		{
			 mAudioSettings.mAudioContextType = Meta::EnumTrait<E_AUDIO_CONTEXT_API_TYPE>::FromString(pFileReader->GetString(TProjectSettingsArchiveKeys::TAudioSettingsKeys::mAudioTypeKey));
		}
		result = result | pFileReader->EndGroup();

		/// \note Localization settings
		result = result | pFileReader->BeginGroup(TProjectSettingsArchiveKeys::mLocalizationSettingsGroupId);
		{
			pFileReader->BeginGroup(TProjectSettingsArchiveKeys::TLocalizationSettingsKeys::mRegisteredLocalesKey);
			{
				TLocalizationSettings::TLocaleInfo info;

				auto& registeredLocales = mLocalizationSettings.mRegisteredLocales;

				while (pFileReader->HasNextItem())
				{
					pFileReader->BeginGroup(Wrench::StringUtils::GetEmptyStr());
					{
						pFileReader->BeginGroup(TProjectSettingsArchiveKeys::TLocalizationSettingsKeys::mLocaleInfoGroupId);
						
						info.mName = pFileReader->GetString(TProjectSettingsArchiveKeys::TLocalizationSettingsKeys::mLocaleIdKey);
						info.mPackagePath = pFileReader->GetString(TProjectSettingsArchiveKeys::TLocalizationSettingsKeys::mLocalePackagePathKey);
						info.mId = static_cast<TLocaleId>(registeredLocales.size());

						registeredLocales.emplace_back(info);

						pFileReader->EndGroup();
					}
					pFileReader->EndGroup();
				}
			}
			pFileReader->EndGroup();

			mLocalizationSettings.mCurrActiveLocaleId = pFileReader->GetString(TProjectSettingsArchiveKeys::TLocalizationSettingsKeys::mCurrActiveLocaleKey);
		}
		result = result | pFileReader->EndGroup();

		return RC_OK;
	}

	TDE2_API TPtr<CProjectSettings> CProjectSettings::Get()
	{
		static TPtr<CProjectSettings> pInstance = TPtr<CProjectSettings>(new (std::nothrow) CProjectSettings());
		return pInstance;
	}
}