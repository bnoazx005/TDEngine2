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

		return _internalSerialize(object);
	}

	E_RESULT_CODE CYAMLFileWriter::Open(IFileSystem* pFileSystem, const std::string& filename)
	{
		E_RESULT_CODE result = CBaseFile::Open(pFileSystem, filename);
		if (result != RC_OK)
		{
			return result;
		}

		std::lock_guard<std::mutex> lock(mMutex);

		mpRootNode = new (std::nothrow) Yaml::Node;
		if (!mpRootNode)
		{
			return RC_OUT_OF_MEMORY;
		}

		mpContext.emplace(mpRootNode);

		return RC_OK;
	}

	E_RESULT_CODE CYAMLFileWriter::BeginGroup(const std::string& key, bool isArray)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		auto&& currNode = *_getCurrScope();

		mpContext.emplace(key.empty() ? &currNode.PushBack() : &currNode[key]);
		mpScopesIndexers.push(mCurrElementIndex);
		mArrayContext.push(mIsArrayScope);

		mIsArrayScope = isArray;
		mCurrElementIndex = 0;

		return RC_OK;
	}

	E_RESULT_CODE CYAMLFileWriter::EndGroup()
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (mpContext.size() < 2)
		{
			return RC_FAIL;
		}

		mCurrElementIndex = mpScopesIndexers.top() + 1;
		mIsArrayScope = mArrayContext.top();

		mpScopesIndexers.pop();
		mpContext.pop();

		return RC_OK;
	}

	template <>
	E_RESULT_CODE CYAMLFileWriter::_setContent<std::string>(const std::string& key, const std::string& value)
	{
		if (mIsArrayScope)
		{
			(*_getCurrScope())[mCurrElementIndex++] = value;
		}

		(*_getCurrScope())[key] = value;
		return RC_OK;
	}

	E_RESULT_CODE CYAMLFileWriter::SetUInt8(const std::string& key, U8 value) 
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _setContent<U8>(key, value);
	}

	E_RESULT_CODE CYAMLFileWriter::SetUInt16(const std::string& key, U16 value) 
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _setContent<U16>(key, value);
	}

	E_RESULT_CODE CYAMLFileWriter::SetUInt32(const std::string& key, U32 value) 
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _setContent<U32>(key, value);
	}

	E_RESULT_CODE CYAMLFileWriter::SetUInt64(const std::string& key, U64 value) 
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _setContent<U64>(key, value);
	}

	E_RESULT_CODE CYAMLFileWriter::SetInt8(const std::string& key, I8 value) 
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _setContent<I8>(key, value);
	}

	E_RESULT_CODE CYAMLFileWriter::SetInt16(const std::string& key, I16 value) 
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _setContent<I16>(key, value);
	}

	E_RESULT_CODE CYAMLFileWriter::SetInt32(const std::string& key, I32 value) 
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _setContent<I32>(key, value);
	}

	E_RESULT_CODE CYAMLFileWriter::SetInt64(const std::string& key, I64 value) 
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _setContent<I64>(key, value);
	}

	E_RESULT_CODE CYAMLFileWriter::SetFloat(const std::string& key, F32 value) 
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _setContent<F32>(key, value);
	}

	E_RESULT_CODE CYAMLFileWriter::SetDouble(const std::string& key, F64 value) 
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _setContent<F64>(key, value);
	}

	E_RESULT_CODE CYAMLFileWriter::SetBool(const std::string& key, bool value) 
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _setContent<bool>(key, value);
	}

	E_RESULT_CODE CYAMLFileWriter::SetString(const std::string& key, const std::string& value)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _setContent<std::string>(key, value);
	}

	E_RESULT_CODE CYAMLFileWriter::_internalSerialize(Yaml::Node& object)
	{
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
		if (mpRootNode)
		{
			CDeferOperation releaseMemory([this] { delete mpRootNode; });

			E_RESULT_CODE result = _internalSerialize(*mpRootNode);
			if (result != RC_OK)
			{
				return result;
			}
		}

		return RC_OK;
	}

	Yaml::Node* CYAMLFileWriter::_getCurrScope() const
	{
		TDE2_ASSERT(!mpContext.empty());
		return mpContext.top();
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

		return _internalDeserialize(outputObject);
	}

	E_RESULT_CODE CYAMLFileReader::Open(IFileSystem* pFileSystem, const std::string& filename)
	{
		E_RESULT_CODE result = CBaseFile::Open(pFileSystem, filename);
		if (result != RC_OK)
		{
			return result;
		}

		std::lock_guard<std::mutex> lock(mMutex);

		mpRootNode = new (std::nothrow) Yaml::Node;
		if (!mpRootNode)
		{
			return RC_OUT_OF_MEMORY;
		}

		mpContext.emplace(mpRootNode);

		return _internalDeserialize(*mpRootNode);
	}

	E_RESULT_CODE CYAMLFileReader::BeginGroup(const std::string& key)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		auto&& currNode = *_getCurrScope();

		mpContext.emplace(key.empty() ? &currNode[mCurrElementIndex] : &currNode[key]);
		mpScopesIndexers.push(mCurrElementIndex);
		mCurrElementIndex = 0;

		return RC_OK;
	}

	E_RESULT_CODE CYAMLFileReader::EndGroup()
	{
		std::lock_guard<std::mutex> lock(mMutex);

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
		std::lock_guard<std::mutex> lock(mMutex);
		return (mCurrElementIndex + 1 < _getCurrScope()->Size());
	}

	U8 CYAMLFileReader::GetUInt8(const std::string& key)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _getContentAs<U8>(key);
	}

	U16 CYAMLFileReader::GetUInt16(const std::string& key)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _getContentAs<U16>(key);
	}

	U32 CYAMLFileReader::GetUInt32(const std::string& key)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _getContentAs<U32>(key);
	}

	U64 CYAMLFileReader::GetUInt64(const std::string& key)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _getContentAs<U64>(key);
	}

	I8 CYAMLFileReader::GetInt8(const std::string& key)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _getContentAs<I8>(key);
	}

	I16 CYAMLFileReader::GetInt16(const std::string& key)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _getContentAs<I16>(key);
	}

	I32 CYAMLFileReader::GetInt32(const std::string& key)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _getContentAs<I32>(key);
	}

	I64 CYAMLFileReader::GetInt64(const std::string& key)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _getContentAs<I64>(key);
	}

	F32 CYAMLFileReader::GetFloat(const std::string& key)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _getContentAs<F32>(key);
	}

	F64 CYAMLFileReader::GetDouble(const std::string& key)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _getContentAs<F64>(key);
	}

	bool CYAMLFileReader::GetBool(const std::string& key)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _getContentAs<bool>(key);
	}

	std::string CYAMLFileReader::GetString(const std::string& key)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _getContentAs<std::string>(key);
	}

	std::string CYAMLFileReader::GetCurrKey() const
	{
		std::lock_guard<std::mutex> lock(mMutex);

		auto&& pCurrNode = _getCurrScope();
		auto&& iter = pCurrNode->Begin();

		for (U32 i = 0; i < (std::min)(mCurrElementIndex, pCurrNode->Size()); ++i, iter++) {}

		return (*iter).first;
	}
	
	E_RESULT_CODE CYAMLFileReader::_internalDeserialize(Yaml::Node& outputObject)
	{
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