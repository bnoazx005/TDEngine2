#include "../../include/core/localization/CLocalizationManager.h"
#include "../../include/core/localization/CLocalizationPackage.h"
#include "../../include/core/IFileSystem.h"
#include "../../include/core/IJobManager.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/core/CGameUserSettings.h"
#include "../../include/platform/CYAMLFile.h"
#include "../../include/utils/CFileLogger.h"
#include "../../include/editor/CPerfProfiler.h"
#include <stringUtils.hpp>
#include <algorithm>


namespace TDEngine2
{
	CLocalizationManager::CLocalizationManager() :
		CBaseObject(),
		mCurrLocalePackageId(TResourceId::Invalid)
	{
	}

	E_RESULT_CODE CLocalizationManager::Init(TPtr<IFileSystem> pFileSystem, TPtr<IResourceManager> pResourceManager, const CProjectSettings::TLocalizationSettings& settings)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pFileSystem || !pResourceManager)
		{
			return RC_INVALID_ARGS;
		}

		std::lock_guard<std::mutex> lock(mMutex);

		mpFileSystem = pFileSystem;
		mpResourceManager = pResourceManager;

		mCurrSelectedLocaleId = TLocaleId::Invalid;

		// \todo Load configuration of the manager
		E_RESULT_CODE result = _readConfiguration(settings);
		if (RC_OK != result)
		{
			return result;
		}

		// \note We load default locale, but if it's failed nothing bad happens. We just return empty string for any given key until some locale will be reloaded
		if (TLocaleId::Invalid != mCurrSelectedLocaleId)
		{
			_loadLocaleResources(mCurrSelectedLocaleId);
		}

		mIsInitialized = true;

		return result;
	}

	E_RESULT_CODE CLocalizationManager::LoadLocale(TLocaleId locale)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _loadLocaleResources(locale);
	}

	E_RESULT_CODE CLocalizationManager::LoadLocale(const std::string& localeId)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return RC_OK;
	}

	std::string CLocalizationManager::GetText(const std::string& keyId) const
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _getTextInternal(keyId);
	}
		
	std::string CLocalizationManager::GetFormattedText(const std::string& rawText) const
	{
		std::lock_guard<std::mutex> lock(mMutex);

		std::string sourceString = rawText;
		std::string formattedString;

		std::string::size_type pos = 0;
		std::string::size_type endPos = 0;

		while ((pos = sourceString.find_first_of('{')) != std::string::npos)
		{
			formattedString.append(sourceString.substr(0, pos));

			endPos = sourceString.find_first_of('}', pos);

			if (endPos != std::string::npos)
			{
				formattedString.append(_getTextInternal(sourceString.substr(pos + 1, endPos - pos - 1)));
			}

			sourceString = sourceString.substr(((endPos != std::string::npos) ? endPos : pos) + 1);
		}

		return formattedString;
	}

	E_ENGINE_SUBSYSTEM_TYPE CLocalizationManager::GetType() const
	{
		return E_ENGINE_SUBSYSTEM_TYPE::EST_LOCALIZATION_MANAGER;
	}

	TLocaleId CLocalizationManager::GetLocaleHash(const std::string& localeId) const
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _getLocaleHashInternal(localeId);
	}

	std::vector<std::string> CLocalizationManager::GetAvailableLanguages() const
	{
		std::lock_guard<std::mutex> lock(mMutex);

		std::vector<std::string> availableLocales;
		std::transform(mRegisteredLocales.cbegin(), mRegisteredLocales.cend(), std::back_inserter(availableLocales), [](const TLocaleInfoEntity& info)
		{
			return info.mName;
		});

		return availableLocales;
	}

	TLocaleId CLocalizationManager::_getLocaleHashInternal(const std::string& localeId) const
	{
		auto iter = std::find_if(mRegisteredLocales.cbegin(), mRegisteredLocales.cend(), [&localeId](const TLocaleInfoEntity& entity) { return entity.mName == localeId; });
		return (iter == mRegisteredLocales.cend()) ? TLocaleId::Invalid : iter->mId;
	}

	E_RESULT_CODE CLocalizationManager::_readConfiguration(const CProjectSettings::TLocalizationSettings& settings)
	{
		for (auto&& currLocaleInfo : settings.mRegisteredLocales)
		{
			mRegisteredLocales.push_back({ currLocaleInfo.mName, currLocaleInfo.mPackagePath, currLocaleInfo.mId });
		}

		mCurrSelectedLocaleId = _getLocaleHashInternal(CGameUserSettings::Get()->mpCurrLanguageCVar->Get());

		return RC_OK;
	}

	E_RESULT_CODE CLocalizationManager::_loadLocaleResources(TLocaleId locale)
	{
		if (!mpResourceManager)
		{
			return RC_FAIL;
		}

		// \note Retrieve a path of the package
		auto iter = std::find_if(mRegisteredLocales.cbegin(), mRegisteredLocales.cend(), [locale](const TLocaleInfoEntity& entity) { return entity.mId == locale; });
		if (iter == mRegisteredLocales.cend())
		{
			LOG_ERROR(Wrench::StringUtils::Format("[Localization Manager] Couldn't load resources of the given locale, locale_handle: {0}", static_cast<U32>(locale)));
			TDE2_ASSERT(false);

			return RC_FAIL;
		}

		// \note Load it using the resource manager
		mCurrLocalePackageId = mpResourceManager->Load<ILocalizationPackage>(iter->mPackagePath);

		// \note Wait while package is loaded and then retrieve information from it
		if (IJobManager* pJobManager = mpFileSystem->GetJobManager())
		{
			pJobManager->SubmitJob(nullptr, [this](auto&&) {
				TDE2_PROFILER_SCOPE("LoadLocalizationPackageJob");

				if (TPtr<IResource> pResource = mpResourceManager->GetResource<IResource>(mCurrLocalePackageId))
				{
					if (E_RESOURCE_STATE_TYPE::RST_PENDING == pResource->GetState())
					{
						return;
					}

					while (pResource->GetState() != E_RESOURCE_STATE_TYPE::RST_LOADED) {}

					{
						std::lock_guard<std::mutex> lock(mMutex);
						if (TPtr<ILocalizationPackage> pPackageResource = DynamicPtrCast<ILocalizationPackage>(pResource))
						{
							auto&& data = pPackageResource->GetDictionaryData();

							mCurrLocaleData.clear();
							mCurrLocaleData.insert(data.cbegin(), data.cend());
						}
					}
				}				
			});			
		}
		
		return RC_OK;
	}

	std::string CLocalizationManager::_getTextInternal(const std::string& key) const
	{
		auto iter = mCurrLocaleData.find(key);
		if (iter == mCurrLocaleData.cend())
		{
			LOG_ERROR(Wrench::StringUtils::Format("[Localization Manager] The key {0} wasn't found", key));
			TDE2_ASSERT(false);

			return Wrench::StringUtils::GetEmptyStr();
		}

		return iter->second;
	}


	TDE2_API ILocalizationManager* CreateLocalizationManager(TPtr<IFileSystem> pFileSystem, TPtr<IResourceManager> pResourceManager, const CProjectSettings::TLocalizationSettings& settings, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ILocalizationManager, CLocalizationManager, result, pFileSystem, pResourceManager, settings);
	}
}