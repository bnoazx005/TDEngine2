#include "./../../include/platform/CYAMLFile.h"
#include "./../../deps/yaml/Yaml.cpp"


namespace TDEngine2
{
	CYAMLFileWriter::CYAMLFileWriter():
		CBaseFile()
	{
		mCreationFlags |= std::ios::trunc;
	}

	E_RESULT_CODE CYAMLFileWriter::Serialize(Yaml::Node& object)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (!mFile.is_open())
		{
			return RC_FAIL;
		}

		try
		{
			std::string formattedOutput;

			Yaml::Serialize(object, formattedOutput);
			
			mFile.seekg(std::ios::beg);
			mFile.write(formattedOutput.c_str(), formattedOutput.length());
		}
		catch (const Yaml::Exception e)
		{
			return RC_FAIL;
		}

		return RC_OK;
	}

	E_RESULT_CODE CYAMLFileWriter::_onFree()
	{
		return RC_OK;
	}


	IFile* CreateYAMLFileWriter(IFileSystem* pFileSystem, const std::string& filename, E_RESULT_CODE& result)
	{
		CYAMLFileWriter* pFileInstance = new (std::nothrow) CYAMLFileWriter();

		if (!pFileInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pFileInstance->Open(pFileSystem, filename);

		if (result != RC_OK)
		{
			delete pFileInstance;

			pFileInstance = nullptr;
		}

		return dynamic_cast<IFile*>(pFileInstance);
	}


	/*!
		\brief CYAMLFileReader's definition
	*/

	CYAMLFileReader::CYAMLFileReader() :
		CBaseFile()
	{
	}

	E_RESULT_CODE CYAMLFileReader::Deserialize(Yaml::Node& outputObject)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (!mFile.is_open())
		{
			return RC_FAIL;
		}

		// \note read YAML file to its end
		std::stringstream strBuffer;

		strBuffer << mFile.rdbuf();

		// \parse it using mini-yaml library
		try
		{
			Yaml::Parse(outputObject, strBuffer.str());
		}
		catch (Yaml::Exception e)
		{
			return RC_FAIL;
		}

		return RC_OK;
	}

	E_RESULT_CODE CYAMLFileReader::_onFree()
	{
		return RC_OK;
	}


	IFile* CreateYAMLFileReader(IFileSystem* pFileSystem, const std::string& filename, E_RESULT_CODE& result)
	{
		CYAMLFileReader* pFileInstance = new (std::nothrow) CYAMLFileReader();

		if (!pFileInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pFileInstance->Open(pFileSystem, filename);

		if (result != RC_OK)
		{
			delete pFileInstance;

			pFileInstance = nullptr;
		}

		return dynamic_cast<IFile*>(pFileInstance);
	}
}