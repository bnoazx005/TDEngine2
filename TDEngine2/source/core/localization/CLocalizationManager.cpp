#include "../../include/core/localization/CLocalizationManager.h"
#include "../../include/core/localization/CLocalizationPackage.h"
#include "../../include/core/IFileSystem.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/platform/CYAMLFile.h"
#include "../../include/utils/CFileLogger.h"
#include <stringUtils.hpp>


namespace TDEngine2
{
	CLocalizationManager::CLocalizationManager() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CLocalizationManager::Init(IFileSystem* pFileSystem, IResourceManager* pResourceManager, const std::string& configPath)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pFileSystem || !pResourceManager || configPath.empty())
		{
			return RC_INVALID_ARGS;
		}

		std::lock_guard<std::mutex> lock(mMutex);

		mpFileSystem = pFileSystem;
		mpResourceManager = pResourceManager;
		mConfigFilePath = configPath;

		mCurrSelectedLocaleId = TLocaleId::Invalid;

		// \todo Load configuration of the manager
		E_RESULT_CODE result = _readConfiguration();
		if (RC_OK != result)
		{
			return result;
		}

		// \note We load default locale, but if it's failed nothing bad happens. We just return empty string for any given key until some locale will be reloaded
		_loadLocaleResources(mCurrSelectedLocaleId);

		mIsInitialized = true;

		return result;
	}

	E_RESULT_CODE CLocalizationManager::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		--mRefCounter;

		if (!mRefCounter)
		{
			mIsInitialized = false;
			delete this;
		}

		return RC_OK;
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

	TLocaleId CLocalizationManager::_getLocaleHashInternal(const std::string& localeId) const
	{
		auto iter = std::find_if(mRegisteredLocales.cbegin(), mRegisteredLocales.cend(), [&localeId](const TLocaleInfoEntity& entity) { return entity.mName == localeId; });
		return (iter == mRegisteredLocales.cend()) ? TLocaleId::Invalid : iter->mId;
	}

	E_RESULT_CODE CLocalizationManager::_readConfiguration()
	{
		auto openFileResult = mpFileSystem->Open<IYAMLFileReader>(mConfigFilePath);
		if (openFileResult.HasError())
		{
			return openFileResult.GetError();
		}

		if (auto pConfigFile = mpFileSystem->Get<IYAMLFileReader>(openFileResult.Get()))
		{
			pConfigFile->BeginGroup("registered_locales");
			{
				while (pConfigFile->HasNextItem())
				{
					pConfigFile->BeginGroup(Wrench::StringUtils::GetEmptyStr());
					{
						pConfigFile->BeginGroup("locale_info");
						_registerLocale(pConfigFile);
						pConfigFile->EndGroup();
					}
					pConfigFile->EndGroup();
				}				
			}
			pConfigFile->EndGroup();

			mCurrSelectedLocaleId = _getLocaleHashInternal(pConfigFile->GetString("current_locale"));
			TDE2_ASSERT(TLocaleId::Invalid != mCurrSelectedLocaleId);

			pConfigFile->Close();
		}

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
		mCurrLocalePackageId = mpResourceManager->Load<CLocalizationPackage>(iter->mPackagePath);

		// \note Wait while package is loaded and then retrieve information from it
		// \todo Make it asynchronous

		if (ILocalizationPackage* pPackageResource = mpResourceManager->GetResource<ILocalizationPackage>(mCurrLocalePackageId))
		{
			auto&& data = pPackageResource->GetDictionaryData();

			mCurrLocaleData.clear();
			mCurrLocaleData.insert(data.cbegin(), data.cend());
		}

		return RC_OK;
	}

	E_RESULT_CODE CLocalizationManager::_registerLocale(IYAMLFileReader* pConfigReader)
	{
		TLocaleInfoEntity localeInfo;

		localeInfo.mName = pConfigReader->GetString("id");
		localeInfo.mPackagePath = pConfigReader->GetString("package_path");
		localeInfo.mId = static_cast<TLocaleId>(mRegisteredLocales.size());

		mRegisteredLocales.emplace_back(localeInfo);

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


	TDE2_API ILocalizationManager* CreateLocalizationManager(IFileSystem* pFileSystem, IResourceManager* pResourceManager, const std::string& configPath, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ILocalizationManager, CLocalizationManager, result, pFileSystem, pResourceManager, configPath);
	}
}