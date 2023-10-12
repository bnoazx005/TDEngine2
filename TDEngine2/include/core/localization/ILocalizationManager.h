/*!
	\file ILocalizationManager.h
	\date 18.12.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "../../utils/Types.h"
#include "../../utils/Utils.h"
#include "../../core/IBaseObject.h"
#include "../../core/IEngineSubsystem.h"
#include "../../core/CProjectSettings.h"
#include <string>
#include <vector>


namespace TDEngine2
{
	TDE2_DECLARE_HANDLE_TYPE(TLocaleId)


	class IFileSystem;
	class IResourceManager;

	
	TDE2_DECLARE_SCOPED_PTR(IFileSystem)
	TDE2_DECLARE_SCOPED_PTR(IResourceManager)


	/*!
		interface ILocalizationManager

		\brief The interface represents a functionality that any localization manager in the engine
		should provide
	*/

	class ILocalizationManager : public virtual IBaseObject, public IEngineSubsystem
	{
		public:
			/*!
				\brief The method initializes an inner state of a manager

				\param[in, out] pFileSystem A pointer to IFileSystem implementation
				\param[in, out] pResourceManager A pointer to IResourceManager implementation
				\param[in] settings A structure with bunch of settings related with the localization manager

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(TPtr<IFileSystem> pFileSystem, TPtr<IResourceManager> pResourceManager, const CProjectSettings::TLocalizationSettings& settings) = 0;

			/*!
				\brief The method loads given locale's data into the manager

				\param[in] locale A numeric handle of the locale, could be given from GetLocaleHash method

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE LoadLocale(TLocaleId locale) = 0;

			/*!
				\brief The method loads given locale's data into the manager

				\param[in] localeId A string identifier of a locale. All possible values should be enumerated within localization manager's configuration file

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE LoadLocale(const std::string& localeId) = 0;

			/*!
				\brief The method returns a value of a key

				\param[in] keyId An identifier 

				\return A string with value of the given key identifier
			*/

			TDE2_API virtual std::string GetText(const std::string& keyId) const = 0;

			/*!
				\brief The method transforms input string of the following structure " aaa{key}bbbbb..."
				So let assume that {key} equals to "one" so the result of the method will be "aaaonebbbb..."
				The variables should always enclosed using braces { and }. Nested braces are not supported for now

				\param[in] rawText An input string that can contain some text with variables which are keys from locale packages

				\return The string with replaced appearances of variables {variableX} onto their values
			*/

			TDE2_API virtual std::string GetFormattedText(const std::string& rawText) const = 0;

			/*!
				\return The method returns a hash value that corresponds to given string identifier of a locale
			*/

			TDE2_API virtual TLocaleId GetLocaleHash(const std::string& localeId) const = 0;

			/*!
				\return The method returns an array with identifiers of locales that are available to be loaded
			*/

			TDE2_API virtual std::vector<std::string> GetAvailableLanguages() const = 0;

			TDE2_API static E_ENGINE_SUBSYSTEM_TYPE GetTypeID() { return E_ENGINE_SUBSYSTEM_TYPE::EST_LOCALIZATION_MANAGER; }

		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ILocalizationManager)
	};
}