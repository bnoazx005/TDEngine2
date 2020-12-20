/*!
	\file CLocalizationManager.h
	\date 18.12.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "ILocalizationManager.h"
#include "../core/CBaseObject.h"
#include <unordered_map>
#include <vector>
#include <mutex>


namespace TDEngine2
{
	class IYAMLFileReader;


	/*!
		\brief A factory function for creation objects of CLocalizationManager's type

		\param[in, out] pFileSystem A pointer to IFileSystem implementation
		\param[in, out] pResourceManager A pointer to IResourceManager implementation
		\param[in] configPath A string with a path to manager's configuration
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CLocalizationManager's implementation
	*/

	TDE2_API ILocalizationManager* CreateLocalizationManager(IFileSystem* pFileSystem, IResourceManager* pResourceManager, const std::string& configPath, E_RESULT_CODE& result);


	class CLocalizationManager : public CBaseObject, public ILocalizationManager
	{
		public:
			friend TDE2_API ILocalizationManager* CreateLocalizationManager(IFileSystem*, IResourceManager*, const std::string&, E_RESULT_CODE&);

		public:
			struct TLocaleInfoEntity
			{
				std::string mName;
				std::string mPackagePath;
				TLocaleId mId;
			};

			typedef std::vector<TLocaleInfoEntity> TLocalesRegistry;
			typedef std::unordered_map<std::string, std::string> TLocaleData;

		public:
			/*!
				\brief The method initializes an inner state of a manager

				\param[in, out] pFileSystem A pointer to IFileSystem implementation
				\param[in, out] pResourceManager A pointer to IResourceManager implementation
				\param[in] configPath A string with a path to manager's configuration

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IFileSystem* pFileSystem, IResourceManager* pResourceManager, const std::string& configPath) override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method loads given locale's data into the manager

				\param[in] locale A numeric handle of the locale, could be given from GetLocaleHash method

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE LoadLocale(TLocaleId locale) override;

			/*!
				\brief The method loads given locale's data into the manager

				\param[in] localeId A string identifier of a locale. All possible values should be enumerated within localization manager's configuration file

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE LoadLocale(const std::string& localeId) override;

			/*!
				\brief The method returns a value of a key

				\param[in] keyId An identifier

				\return A string with value of the given key identifier
			*/

			TDE2_API std::string GetText(const std::string& keyId) const override;

			/*!
				\brief The method transforms input string of the following structure " aaa{key}bbbbb..."
				So let assume that {key} equals to "one" so the result of the method will be "aaaonebbbb..."
				The variables should always enclosed using braces { and }. Nested braces are not supported for now

				\param[in] rawText An input string that can contain some text with variables which are keys from locale packages

				\return The string with replaced appearances of variables {variableX} onto their values
			*/

			TDE2_API std::string GetFormattedText(const std::string& rawText) const override;

			/*!
				\brief The method returns a type of the subsystem

				\return A type, which is represented with E_ENGINE_SUBSYSTEM_TYPE's value
			*/

			TDE2_API E_ENGINE_SUBSYSTEM_TYPE GetType() const override;

			TDE2_API static E_ENGINE_SUBSYSTEM_TYPE GetTypeID() { return E_ENGINE_SUBSYSTEM_TYPE::EST_LOCALIZATION_MANAGER; }

			/*!
				\return The method returns a hash value that corresponds to given string identifier of a locale
			*/

			TDE2_API TLocaleId GetLocaleHash(const std::string& localeId) const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CLocalizationManager)

			TDE2_API E_RESULT_CODE _readConfiguration();
			TDE2_API E_RESULT_CODE _registerLocale(IYAMLFileReader* pConfigReader);

			TDE2_API E_RESULT_CODE _loadLocaleResources(TLocaleId locale);

			TDE2_API std::string _getTextInternal(const std::string& key) const;

			TDE2_API TLocaleId _getLocaleHashInternal(const std::string& localeId) const;
		protected:
			mutable std::mutex mMutex;

			IFileSystem* mpFileSystem;

			IResourceManager* mpResourceManager;

			std::string mConfigFilePath;

			TLocaleId mCurrSelectedLocaleId;

			TLocalesRegistry mRegisteredLocales;

			TLocaleData mCurrLocaleData;
	};
}