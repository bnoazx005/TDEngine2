/*!
	\file CYAMLFileWriter.h
	\date 09.09.2019
	\authors Kasimov Ildar
*/

#pragma once


#include <platform/CBaseFile.h>
#include "IYAMLFile.h"


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CYAMLFileWriter's type

		\return A pointer to CYAMLFileWriter's implementation
	*/

	TDE2_API IFile* CreateYAMLFileWriter(IFileSystem* pFileSystem, const std::string& filename, E_RESULT_CODE& result);


	/*!
		class CYAMLFileWriter

		\brief The class is an implementation of IYAMLFileWriter
	*/

	class CYAMLFileWriter : public IYAMLFileWriter, public CBaseFile
	{
		public:
			friend TDE2_API IFile* CreateYAMLFileWriter(IFileSystem* pFileSystem, const std::string& filename, E_RESULT_CODE& result);
		public:
			typedef void(*TSuccessWriteCallback)();

			typedef void(*TErrorWriteCallback)(E_RESULT_CODE);
		public:
			TDE2_REGISTER_TYPE(CYAMLFileWriter)

			TDE2_API E_RESULT_CODE Serialize(Yaml::Node& object) override;
		protected:
			TDE2_API E_RESULT_CODE _onFree() override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CYAMLFileWriter)
	};


	/*!
		\brief A factory function for creation objects of CYAMLFileReader's type

		\return A pointer to CYAMLFileReader's implementation
	*/

	TDE2_API IFile* CreateYAMLFileReader(IFileSystem* pFileSystem, const std::string& filename, E_RESULT_CODE& result);


	/*!
		class CYAMLFileReader

		\brief The class is an implementation of IYAMLFileReader
	*/

	class CYAMLFileReader : public IYAMLFileReader, public CBaseFile
	{
		public:
			friend TDE2_API IFile* CreateYAMLFileReader(IFileSystem* pFileSystem, const std::string& filename, E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_TYPE(CYAMLFileReader)

			TDE2_API E_RESULT_CODE Deserialize(Yaml::Node& outputObject) override;
		protected:
			TDE2_API E_RESULT_CODE _onFree() override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CYAMLFileReader)
	};
}