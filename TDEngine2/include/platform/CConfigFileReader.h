/*!
	\file CConfigFileReader.h
	\date 20.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "CBaseFile.h"
#include <fstream>
#include <unordered_map>


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CConfigFileReader's type.
		The class implements a lazy reader, it means that it hashes read values into
		an internal table to speed up a search.

		\return A pointer to CConfigFileReader's implementation
	*/

	TDE2_API IFile* CreateConfigFileReader(IFileSystem* pFileSystem, const std::string& filename, E_RESULT_CODE& result);


	/*!
		class CConfigFileReader

		\brief The class represents a base reader of config files
	*/

	class CConfigFileReader : public IConfigFileReader, public CBaseFile<CConfigFileReader>
	{
		public:
			friend TDE2_API IFile* CreateConfigFileReader(IFileSystem* pFileSystem, const std::string& filename, E_RESULT_CODE& result);
		protected:
			typedef std::unordered_map<std::string, std::unordered_map<std::string, std::string>> TConfigParamsMap;
		public:
			/*!
				\brief The method try to read integer value with specified name inside a group. Otherwise, it
				will return a default value

				\param[in] group A group's name

				\param[in] paramName A parameter's name

				\param[in] defaultValue A default value, which will be returned if the specified parameter doesn't exists

				\return The method try to read integer value with specified name inside a group. Otherwise, it
				will return a default value
			*/

			TDE2_API I32 GetInt(const std::string& group, const std::string& paramName, I32 defaultValue = 0) override;

			/*!
				\brief The method try to read integer value with specified name inside a group. Otherwise, it
				will return a default value

				\param[in] group A group's name

				\param[in] paramName A parameter's name

				\param[in] defaultValue A default value, which will be returned if the specified parameter doesn't exists

				\return The method try to read integer value with specified name inside a group. Otherwise, it
				will return a default value
			*/

			TDE2_API F32 GetFloat(const std::string& group, const std::string& paramName, F32 defaultValue = 0.0f) override;

			/*!
				\brief The method try to read integer value with specified name inside a group. Otherwise, it
				will return a default value

				\param[in] group A group's name

				\param[in] paramName A parameter's name

				\param[in] defaultValue A default value, which will be returned if the specified parameter doesn't exists

				\return The method try to read integer value with specified name inside a group. Otherwise, it
				will return a default value
			*/

			TDE2_API bool GetBool(const std::string& group, const std::string& paramName, bool defaultValue = false) override;

			/*!
				\brief The method try to read integer value with specified name inside a group. Otherwise, it
				will return a default value

				\param[in] group A group's name

				\param[in] paramName A parameter's name

				\param[in] defaultValue A default value, which will be returned if the specified parameter doesn't exists

				\return The method try to read integer value with specified name inside a group. Otherwise, it
				will return a default value
			*/

			TDE2_API std::string GetString(const std::string& group, const std::string& paramName, std::string defaultValue = "") override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CConfigFileReader)

			TDE2_API E_RESULT_CODE _parseFileUntilParam(const std::string& group, const std::string& paramName, TConfigParamsMap& paramsMap, std::string& value);

			TDE2_API E_RESULT_CODE _onFree() override;
		protected:
			TConfigParamsMap mParamsMap;
	};
}
