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

	E_RESULT_CODE CYAMLFileWriter::Open(IFileSystem* pFileSystem, const std::string& filename)
	{
		E_RESULT_CODE result = CBaseFile::Open(pFileSystem, filename);
		if (result != RC_OK)
		{
			return result;
		}

		mpRootNode = new (std::nothrow) Yaml::Node;
		if (!mpRootNode)
		{
			return RC_OUT_OF_MEMORY;
		}

		mpContext.emplace(mpRootNode);

		return RC_OK;
	}

	E_RESULT_CODE CYAMLFileWriter::BeginGroup(const std::string& key)
	{
		TDE2_UNIMPLEMENTED();
		return RC_NOT_IMPLEMENTED_YET;
	}

	E_RESULT_CODE CYAMLFileWriter::EndGroup()
	{
		TDE2_UNIMPLEMENTED();
		return RC_NOT_IMPLEMENTED_YET;
	}

	E_RESULT_CODE CYAMLFileWriter::BeginArray(const std::string& key) 
	{
		return RC_NOT_IMPLEMENTED_YET;
	}

	E_RESULT_CODE CYAMLFileWriter::EndArray() 
	{
		return RC_NOT_IMPLEMENTED_YET;
	}

	E_RESULT_CODE CYAMLFileWriter::SetUInt8(const std::string& key, U8 value) 
	{
		return RC_NOT_IMPLEMENTED_YET;
	}

	E_RESULT_CODE CYAMLFileWriter::SetUInt16(const std::string& key, U16 value) 
	{
		return RC_NOT_IMPLEMENTED_YET;
	}

	E_RESULT_CODE CYAMLFileWriter::SetUInt32(const std::string& key, U32 value) 
	{
		return RC_NOT_IMPLEMENTED_YET;
	}

	E_RESULT_CODE CYAMLFileWriter::SetUInt64(const std::string& key, U64 value) 
	{
		return RC_NOT_IMPLEMENTED_YET;
	}

	E_RESULT_CODE CYAMLFileWriter::SetInt8(const std::string& key, I8 value) 
	{
		return RC_NOT_IMPLEMENTED_YET;
	}

	E_RESULT_CODE CYAMLFileWriter::SetInt16(const std::string& key, I16 value) 
	{
		return RC_NOT_IMPLEMENTED_YET;
	}

	E_RESULT_CODE CYAMLFileWriter::SetInt32(const std::string& key, I32 value) 
	{
		return RC_NOT_IMPLEMENTED_YET;
	}

	E_RESULT_CODE CYAMLFileWriter::SetInt64(const std::string& key, I64 value) 
	{
		return RC_NOT_IMPLEMENTED_YET;
	}

	E_RESULT_CODE CYAMLFileWriter::SetFloat(const std::string& key, F32 value) 
	{
		return RC_NOT_IMPLEMENTED_YET;
	}

	E_RESULT_CODE CYAMLFileWriter::SetDouble(const std::string& key, F64 value) 
	{
		return RC_NOT_IMPLEMENTED_YET;
	}

	E_RESULT_CODE CYAMLFileWriter::SetBool(const std::string& key, bool value) 
	{
		return RC_NOT_IMPLEMENTED_YET;
	}

	E_RESULT_CODE CYAMLFileWriter::SetString(const std::string& key, const std::string& value)
	{
		return RC_NOT_IMPLEMENTED_YET;
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

	E_RESULT_CODE CYAMLFileReader::Open(IFileSystem* pFileSystem, const std::string& filename)
	{
		E_RESULT_CODE result = CBaseFile::Open(pFileSystem, filename);
		if (result != RC_OK)
		{
			return result;
		}

		mpRootNode = new (std::nothrow) Yaml::Node;
		if (!mpRootNode)
		{
			return RC_OUT_OF_MEMORY;
		}

		mpContext.emplace(mpRootNode);

		return Deserialize(*mpRootNode);
	}

	E_RESULT_CODE CYAMLFileReader::BeginGroup(const std::string& key)
	{
		auto&& currNode = *_getCurrScope();

		mpContext.emplace(key.empty() ? &currNode[mCurrElementIndex] : &currNode[key]);
		mpScopesIndexers.push(mCurrElementIndex);
		mCurrElementIndex = 0;

		return RC_OK;
	}

	E_RESULT_CODE CYAMLFileReader::EndGroup()
	{
		if (mpContext.size() < 2)
		{
			return RC_FAIL;
		}

		mCurrElementIndex = mpScopesIndexers.top() + 1;
		
		mpScopesIndexers.pop();
		mpContext.pop();

		return RC_OK;
	}

	bool CYAMLFileReader::HasNextItem() const
	{
		return (mCurrElementIndex + 1 < _getCurrScope()->Size());
	}

	U8 CYAMLFileReader::GetUInt8(const std::string& key)
	{
		return _getContentAs<U8>(key);
	}

	U16 CYAMLFileReader::GetUInt16(const std::string& key)
	{
		return _getContentAs<U16>(key);
	}

	U32 CYAMLFileReader::GetUInt32(const std::string& key)
	{
		return _getContentAs<U32>(key);
	}

	U64 CYAMLFileReader::GetUInt64(const std::string& key)
	{
		return _getContentAs<U64>(key);
	}

	I8 CYAMLFileReader::GetInt8(const std::string& key)
	{
		return _getContentAs<I8>(key);
	}

	I16 CYAMLFileReader::GetInt16(const std::string& key)
	{
		return _getContentAs<I16>(key);
	}

	I32 CYAMLFileReader::GetInt32(const std::string& key)
	{
		return _getContentAs<I32>(key);
	}

	I64 CYAMLFileReader::GetInt64(const std::string& key)
	{
		return _getContentAs<I64>(key);
	}

	F32 CYAMLFileReader::GetFloat(const std::string& key)
	{
		return _getContentAs<F32>(key);
	}

	F64 CYAMLFileReader::GetDouble(const std::string& key)
	{
		return _getContentAs<F64>(key);
	}

	bool CYAMLFileReader::GetBool(const std::string& key)
	{
		return _getContentAs<bool>(key);
	}

	std::string CYAMLFileReader::GetString(const std::string& key)
	{
		return _getContentAs<std::string>(key);
	}

	std::string CYAMLFileReader::GetCurrKey() const
	{
		auto&& pCurrNode = _getCurrScope();
		auto&& iter = pCurrNode->Begin();

		for (U32 i = 0; i < (std::min)(mCurrElementIndex, pCurrNode->Size()); ++i, iter++) {}

		return (*iter).first;
	}

	Yaml::Node* CYAMLFileReader::_getCurrScope() const
	{
		TDE2_ASSERT(!mpContext.empty());
		return mpContext.top();
	}

	E_RESULT_CODE CYAMLFileReader::_onFree()
	{
		if (mpRootNode)
		{
			delete mpRootNode;
		}

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