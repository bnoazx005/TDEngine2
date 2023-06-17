#include "../../include/platform/CYAMLFile.h"
#include "../../deps/yaml/Yaml.cpp"
#include "../../include/platform/MountableStorages.h"
#include "../../include/platform/IOStreams.h"
#include "../../include/utils/CFileLogger.h"
#include <deferOperation.hpp>
#include <stringUtils.hpp>


namespace TDEngine2
{
	CYAMLFileWriter::CYAMLFileWriter():
		CBaseFile()
	{
	}

	E_RESULT_CODE CYAMLFileWriter::Serialize(Yaml::Node& object)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		return _internalSerialize(object);
	}

	E_RESULT_CODE CYAMLFileWriter::Open(IMountableStorage* pStorage, TPtr<IStream> pStream)
	{
		E_RESULT_CODE result = CBaseFile::Open(pStorage, pStream);
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
		mArrayContext.pop();

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
		if (value.empty())
		{
			return RC_OK;
		}

		std::lock_guard<std::mutex> lock(mMutex);
		return _setContent<std::string>(key, value);
	}

	E_RESULT_CODE CYAMLFileWriter::_internalSerialize(Yaml::Node& object)
	{
		if (!mpStreamImpl->IsValid())
		{
			return RC_FAIL;
		}

		try
		{
			std::string formattedOutput;

			Yaml::Serialize(object, formattedOutput);

			mpStreamImpl->SetPosition(std::ios::beg);
			DynamicPtrCast<IOutputStream>(mpStreamImpl)->Write(formattedOutput.c_str(), formattedOutput.length());
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
			defer([this] { delete mpRootNode; });

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


	IFile* CreateYAMLFileWriter(IMountableStorage* pStorage, TPtr<IStream> pStream, E_RESULT_CODE& result)
	{
		CYAMLFileWriter* pFileInstance = new (std::nothrow) CYAMLFileWriter();

		if (!pFileInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pFileInstance->Open(pStorage, pStream);

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

	E_RESULT_CODE CYAMLFileReader::Open(IMountableStorage* pStorage, TPtr<IStream> pStream)
	{
		E_RESULT_CODE result = CBaseFile::Open(pStorage, pStream);
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
		return (mCurrElementIndex + 1 <= _getCurrScope()->Size());
	}

	U8 CYAMLFileReader::GetUInt8(const std::string& key, U8 defaultValue)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return static_cast<U8>(_getContentAsOrDefault<U16>(key, defaultValue));
	}

	U16 CYAMLFileReader::GetUInt16(const std::string& key, U16 defaultValue)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _getContentAsOrDefault<U16>(key, defaultValue);
	}

	U32 CYAMLFileReader::GetUInt32(const std::string& key, U32 defaultValue)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _getContentAsOrDefault<U32>(key, defaultValue);
	}

	U64 CYAMLFileReader::GetUInt64(const std::string& key, U64 defaultValue)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _getContentAsOrDefault<U64>(key, defaultValue);
	}

	I8 CYAMLFileReader::GetInt8(const std::string& key, I8 defaultValue)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _getContentAsOrDefault<I8>(key, defaultValue);
	}

	I16 CYAMLFileReader::GetInt16(const std::string& key, I16 defaultValue)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _getContentAsOrDefault<I16>(key, defaultValue);
	}

	I32 CYAMLFileReader::GetInt32(const std::string& key, I32 defaultValue)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _getContentAsOrDefault<I32>(key, defaultValue);
	}

	I64 CYAMLFileReader::GetInt64(const std::string& key, I64 defaultValue)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _getContentAsOrDefault<I64>(key, defaultValue);
	}

	F32 CYAMLFileReader::GetFloat(const std::string& key, F32 defaultValue)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _getContentAsOrDefault<F32>(key, defaultValue);
	}

	F64 CYAMLFileReader::GetDouble(const std::string& key, F64 defaultValue)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _getContentAsOrDefault<F64>(key, defaultValue);
	}

	bool CYAMLFileReader::GetBool(const std::string& key, bool defaultValue)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _getContentAsOrDefault<bool>(key, defaultValue);
	}

	std::string CYAMLFileReader::GetString(const std::string& key, const std::string& defaultValue)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _getContentAsOrDefault<std::string>(key, defaultValue);
	}

	std::string CYAMLFileReader::GetCurrKey() const
	{
		std::lock_guard<std::mutex> lock(mMutex);

		auto&& pCurrNode = _getCurrScope();
		auto&& iter = pCurrNode->Begin();

		for (U32 i = 0; i < (std::min<U32>)(mCurrElementIndex, static_cast<U32>(pCurrNode->Size())); ++i, iter++) {}

		return (*iter).first;
	}
	
	E_RESULT_CODE CYAMLFileReader::_internalDeserialize(Yaml::Node& outputObject)
	{
		if (!mpStreamImpl->IsValid())
		{
			return RC_FAIL;
		}

		// \note read YAML file to its end
		// \parse it using mini-yaml library
		try
		{
			Yaml::Parse(outputObject, DynamicPtrCast<IInputStream>(mpStreamImpl)->ReadToEnd());
		}
		catch (Yaml::Exception e)
		{
			LOG_ERROR(Wrench::StringUtils::Format("[YamlFileReader] Error's happened during deserialization process, error_message: {0}", e.Message()));
			TDE2_ASSERT(false);

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


	IFile* CreateYAMLFileReader(IMountableStorage* pStorage, TPtr<IStream> pStream, E_RESULT_CODE& result)
	{
		CYAMLFileReader* pFileInstance = new (std::nothrow) CYAMLFileReader();

		if (!pFileInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pFileInstance->Open(pStorage, pStream);

		if (result != RC_OK)
		{
			delete pFileInstance;

			pFileInstance = nullptr;
		}

		return dynamic_cast<IFile*>(pFileInstance);
	}
}