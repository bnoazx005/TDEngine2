/*!
	\file CConfigFileWriter.h
	\date 23.09.2023
	\authors Kasimov Ildar
*/

#pragma once


#include "CBaseFile.h"
#include <fstream>
#include <unordered_map>
#include <string>


namespace TDEngine2
{
	class IOutputStream;


	TDE2_API IFile* CreateConfigFileWriter(IMountableStorage* pStorage, TPtr<IStream> pStream, E_RESULT_CODE& result);


	/*!
		class CConfigFileWriter

		\brief The class represents a basic writer of config files
	*/

	class CConfigFileWriter : public IConfigFileWriter, public CBaseFile
	{
		public:
			friend TDE2_API IFile* CreateConfigFileWriter(IMountableStorage*, TPtr<IStream>, E_RESULT_CODE&);
		protected:
			typedef std::unordered_map<std::string, std::unordered_map<std::string, std::string>> TConfigParamsMap;
		public:
			TDE2_REGISTER_TYPE(CConfigFileWriter)

			TDE2_API E_RESULT_CODE SetInt(const std::string& group, const std::string& paramName, I32 value = 0) override;
			TDE2_API E_RESULT_CODE SetFloat(const std::string& group, const std::string& paramName, F32 value = 0.0f) override;
			TDE2_API E_RESULT_CODE SetBool(const std::string& group, const std::string& paramName, bool value = false) override;
			TDE2_API E_RESULT_CODE SetString(const std::string& group, const std::string& paramName, const std::string& value = "") override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CConfigFileWriter)

			TDE2_API E_RESULT_CODE _onFree() override;

			TDE2_API E_RESULT_CODE _writeValueToMap(const std::string& group, const std::string& paramName, const std::string& value);

			TDE2_API IOutputStream* _getOutputStream();
		protected:
			TConfigParamsMap mParamsMap;
			IOutputStream*   mpCachedOutputStream;
	};
}
