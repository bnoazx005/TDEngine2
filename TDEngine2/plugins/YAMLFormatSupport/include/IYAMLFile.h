/*!
	\file IYAMLFile.h
	\date 09.09.2019
	\authors Kasimov Ildar
*/

#pragma once


#include <core/IFile.h>
#include <utils/CResult.h>


namespace Yaml
{
	class Node;
}


namespace TDEngine2
{
	/*!
		interface IYAMLFileWriter

		\brief The interface represents a functionality of YAML file writer
	*/

	class IYAMLFileWriter : public IFileWriter
	{
		public:
			typedef void(*TSuccessWriteCallback)();

			typedef void(*TErrorWriteCallback)(E_RESULT_CODE);
		public:
			virtual TDE2_API E_RESULT_CODE Serialize(Yaml::Node& object) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IYAMLFileWriter)
	};


	/*!
		interface IYAMLFileReader

		\brief The interface describes a functionality of YAML file reader
	*/

	class IYAMLFileReader : public IFileReader
	{
		public:
			virtual TDE2_API E_RESULT_CODE Deserialize(Yaml::Node& outputObject) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IYAMLFileReader)
	};
}