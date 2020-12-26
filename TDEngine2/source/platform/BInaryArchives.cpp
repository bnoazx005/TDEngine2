#include "../../include/platform/BinaryArchives.h"
#include "../../include/platform/MountableStorages.h"
#include "../../include/platform/IOStreams.h"
#include "../../include/utils/CFileLogger.h"
#include <stringUtils.hpp>


namespace TDEngine2
{
	template<> constexpr TypeId TArchiveValue::GetValueTag<I8>() { return TDE2_TYPE_ID(I8); }
	template<> constexpr TypeId TArchiveValue::GetValueTag<I16>() { return TDE2_TYPE_ID(I16); }
	template<> constexpr TypeId TArchiveValue::GetValueTag<I32>() { return TDE2_TYPE_ID(I32); }
	template<> constexpr TypeId TArchiveValue::GetValueTag<I64>() { return TDE2_TYPE_ID(I64); }

	template<> constexpr TypeId TArchiveValue::GetValueTag<U8>() { return TDE2_TYPE_ID(U8); }
	template<> constexpr TypeId TArchiveValue::GetValueTag<U16>() { return TDE2_TYPE_ID(U16); }
	template<> constexpr TypeId TArchiveValue::GetValueTag<U32>() { return TDE2_TYPE_ID(U32); }
	template<> constexpr TypeId TArchiveValue::GetValueTag<U64>() { return TDE2_TYPE_ID(U64); }

	template<> constexpr TypeId TArchiveValue::GetValueTag<F32>() { return TDE2_TYPE_ID(F32); }
	template<> constexpr TypeId TArchiveValue::GetValueTag<F64>() { return TDE2_TYPE_ID(F64); }

	template<> constexpr TypeId TArchiveValue::GetValueTag<bool>() { return TDE2_TYPE_ID(bool); }
	template<> constexpr TypeId TArchiveValue::GetValueTag<std::string>() { return TDE2_TYPE_ID(std::string); }


	template <>
	E_RESULT_CODE CBinaryArchiveWriter::_writeValue<std::string>(const std::string& key, const std::string& value)
	{
		E_RESULT_CODE result = _writeStringInternal(key);

		// \note Write tag of the type's value
		const U32 tag = static_cast<U32>(TDE2_TYPE_ID(std::string));
		result = result | mpCachedOutputStream->Write(&tag, sizeof(tag));

		result = result | _writeStringInternal(value);

		return result;
	}

	CBinaryArchiveWriter::CBinaryArchiveWriter() :
		CBaseFile()
	{
	}

	E_RESULT_CODE CBinaryArchiveWriter::Open(IMountableStorage* pStorage, IStream* pStream)
	{
		E_RESULT_CODE result = CBaseFile::Open(pStorage, pStream);
		if (result != RC_OK)
		{
			return result;
		}

		std::lock_guard<std::mutex> lock(mMutex);

		mpCachedOutputStream = dynamic_cast<IOutputStream*>(mpStreamImpl);
		if (!mpCachedOutputStream)
		{
			return RC_FAIL;
		}

		return RC_OK;
	}

	E_RESULT_CODE CBinaryArchiveWriter::BeginGroup(const std::string& key, bool isArray)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		E_RESULT_CODE result = RC_OK;

		result = result | _writeStringInternal(key);
		result = result | _writeTagValue(TArchiveValue::mBeginBlockTag);

		return result;
	}

	E_RESULT_CODE CBinaryArchiveWriter::EndGroup()
	{
		std::lock_guard<std::mutex> lock(mMutex);

		_writeTagValue(TArchiveValue::mEndBlockTag);

		return RC_OK;
	}

	E_RESULT_CODE CBinaryArchiveWriter::SetUInt8(const std::string& key, U8 value)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _writeValue<U8>(key, value);
	}

	E_RESULT_CODE CBinaryArchiveWriter::SetUInt16(const std::string& key, U16 value)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _writeValue<U16>(key, value);
	}

	E_RESULT_CODE CBinaryArchiveWriter::SetUInt32(const std::string& key, U32 value)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _writeValue<U32>(key, value);
	}

	E_RESULT_CODE CBinaryArchiveWriter::SetUInt64(const std::string& key, U64 value)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _writeValue<U64>(key, value);
	}

	E_RESULT_CODE CBinaryArchiveWriter::SetInt8(const std::string& key, I8 value)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _writeValue<I8>(key, value);
	}

	E_RESULT_CODE CBinaryArchiveWriter::SetInt16(const std::string& key, I16 value)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _writeValue<I16>(key, value);
	}

	E_RESULT_CODE CBinaryArchiveWriter::SetInt32(const std::string& key, I32 value)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _writeValue<I32>(key, value);
	}

	E_RESULT_CODE CBinaryArchiveWriter::SetInt64(const std::string& key, I64 value)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _writeValue<I64>(key, value);
	}

	E_RESULT_CODE CBinaryArchiveWriter::SetFloat(const std::string& key, F32 value)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _writeValue<F32>(key, value);
	}

	E_RESULT_CODE CBinaryArchiveWriter::SetDouble(const std::string& key, F64 value)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _writeValue<F64>(key, value);
	}

	E_RESULT_CODE CBinaryArchiveWriter::SetBool(const std::string& key, bool value)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _writeValue<bool>(key, value);
	}

	E_RESULT_CODE CBinaryArchiveWriter::SetString(const std::string& key, const std::string& value)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _writeValue<std::string>(key, value);
	}

	E_RESULT_CODE CBinaryArchiveWriter::_onFree()
	{

		return RC_OK;
	}

	E_RESULT_CODE CBinaryArchiveWriter::_writeStringInternal(const std::string& value)
	{
		if (!mpCachedOutputStream)
		{
			return RC_FAIL;
		}

		const size_t length = value.length();
		const char* pStrBuffer = value.c_str();

		E_RESULT_CODE result = mpCachedOutputStream->Write(static_cast<const void*>(&length), sizeof(size_t));
		result = result | mpCachedOutputStream->Write(static_cast<const void*>(pStrBuffer), sizeof(C8) * length);

		return result;
	}

	TDE2_API E_RESULT_CODE CBinaryArchiveWriter::_writeTagValue(U32 tag)
	{
		return mpCachedOutputStream->Write(&tag, sizeof(tag));
	}

	E_RESULT_CODE CBinaryArchiveWriter::_writeInternal(const void* pValue, U32 size)
	{
		return mpCachedOutputStream->Write(pValue, size);
	}


	IFile* CreateBinaryArchiveWriter(IMountableStorage* pStorage, IStream* pStream, E_RESULT_CODE& result)
	{
		CBinaryArchiveWriter* pFileInstance = new (std::nothrow) CBinaryArchiveWriter();

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
		\brief CBinaryArchiveReader's definition
	*/

	CBinaryArchiveReader::CBinaryArchiveReader() :
		CBaseFile()
	{
	}

	E_RESULT_CODE CBinaryArchiveReader::Open(IMountableStorage* pStorage, IStream* pStream)
	{
		E_RESULT_CODE result = CBaseFile::Open(pStorage, pStream);
		if (result != RC_OK)
		{
			return result;
		}

		std::lock_guard<std::mutex> lock(mMutex);

		mpCachedInputStream = dynamic_cast<IInputStream*>(mpStreamImpl);
		if (!mpCachedInputStream)
		{
			return RC_FAIL;
		}

		return _deserializeInternal();
	}

	E_RESULT_CODE CBinaryArchiveReader::BeginGroup(const std::string& key)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		return RC_OK;
	}

	E_RESULT_CODE CBinaryArchiveReader::EndGroup()
	{
		std::lock_guard<std::mutex> lock(mMutex);

		return RC_OK;
	}

	bool CBinaryArchiveReader::HasNextItem() const
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return false;
	}

	U8 CBinaryArchiveReader::GetUInt8(const std::string& key)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		//return _getContentAs<U8>(key);
		return 0;
	}

	U16 CBinaryArchiveReader::GetUInt16(const std::string& key)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		//return _getContentAs<U16>(key);
		return 0;
	}

	U32 CBinaryArchiveReader::GetUInt32(const std::string& key)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		//return _getContentAs<U32>(key);
		return 0;
	}

	U64 CBinaryArchiveReader::GetUInt64(const std::string& key)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		//return _getContentAs<U64>(key);
		return 0;
	}

	I8 CBinaryArchiveReader::GetInt8(const std::string& key)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		//return _getContentAs<I8>(key);
		return 0;
	}

	I16 CBinaryArchiveReader::GetInt16(const std::string& key)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		//return _getContentAs<I16>(key);
		return 0;
	}

	I32 CBinaryArchiveReader::GetInt32(const std::string& key)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		//return _getContentAs<I32>(key);
		return 0;
	}

	I64 CBinaryArchiveReader::GetInt64(const std::string& key)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		//return _getContentAs<I64>(key);
		return 0;
	}

	F32 CBinaryArchiveReader::GetFloat(const std::string& key)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		//	return _getContentAs<F32>(key);
		return 0.0f;
	}

	F64 CBinaryArchiveReader::GetDouble(const std::string& key)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		//return _getContentAs<F64>(key);
		return 0.0;
	}

	bool CBinaryArchiveReader::GetBool(const std::string& key)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		//	return _getContentAs<bool>(key);
		return false;
	}

	std::string CBinaryArchiveReader::GetString(const std::string& key)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		//return _getContentAs<std::string>(key);
		return "";
	}

	std::string CBinaryArchiveReader::GetCurrKey() const
	{
		std::lock_guard<std::mutex> lock(mMutex);

		return "";
	}

	E_RESULT_CODE CBinaryArchiveReader::_onFree()
	{
		return RC_OK;
	}

	E_RESULT_CODE CBinaryArchiveReader::_deserializeInternal()
	{
		mCurrElementIndex = 0;

		mpCurrNode = nullptr;

		mTopLevelDecls = _readChunk();

		return RC_OK;
	}

	TArchiveValue::TArchiveValuesArray CBinaryArchiveReader::_readChunk()
	{
		std::unique_ptr<TArchiveValue> pCurrNode = nullptr;

		TArchiveValue::TArchiveValuesArray chunks;

		while (true)
		{
			if (_readChunkTag() == TArchiveValue::mEndBlockTag)
			{
				break;
			}

			mpCachedInputStream->SetPosition(std::max<U32>(0, mpCachedInputStream->GetPosition() - sizeof(U32)));

			pCurrNode = _readChunkData();
			if (!pCurrNode)
			{
				break;
			}

			chunks.emplace_back(std::move(pCurrNode));
		}

		return chunks;
	}

	const TArchiveValue* CBinaryArchiveReader::_getContent(const std::string& key)
	{
		const auto& children = mpCurrNode->mpChildren;

		if (key.empty())
		{
			return children[mCurrElementIndex++].get();
		}

		auto iter = std::find_if(children.cbegin(), children.cend(), [key](auto&& t) { return t->mName == key; });
		if (iter == children.cend())
		{
			return nullptr;
		}

		return iter->get();
	}

	std::unique_ptr<TArchiveValue> CBinaryArchiveReader::_readChunkData()
	{
		const std::string key = _readStringChunk();

		const U32 tag = _readChunkTag();
		if (!tag)
		{
			return nullptr;
		}

		std::unique_ptr<TArchiveValue> pBlockValue = nullptr;

		// \note Choose next step based on tag's value
		// \todo Refactor this block later

		switch (static_cast<TypeId>(tag))
		{
			case TArchiveValue::GetValueTag<I8>() :
				return _readIntValue(key, sizeof(I8));

			case TArchiveValue::GetValueTag<I16>() :
				return _readIntValue(key, sizeof(I16));

			case TArchiveValue::GetValueTag<I32>() :
				return _readIntValue(key, sizeof(I32));

			case TArchiveValue::GetValueTag<I64>() :
				return _readIntValue(key, sizeof(I64));

			case TArchiveValue::GetValueTag<U8>() :
				return _readIntValue(key, sizeof(U8));

			case TArchiveValue::GetValueTag<U16>() :
				return _readIntValue(key, sizeof(U16));

			case TArchiveValue::GetValueTag<U32>() :
				return _readIntValue(key, sizeof(U32));

			case TArchiveValue::GetValueTag<U64>() :
				return _readIntValue(key, sizeof(U64));

			case TArchiveValue::GetValueTag<F32>() :
				return _readFloatValue(key, sizeof(F32));

			case TArchiveValue::GetValueTag<F64>() :
				return _readFloatValue(key, sizeof(F64));

			case TArchiveValue::GetValueTag<bool>() :
				return _readBoolValue(key);

			case TDE2_TYPE_ID(std::string) :
				return _readStringValue(key);

			case TypeId(TArchiveValue::mBeginBlockTag) :
				pBlockValue = std::make_unique<TArchiveValue>();
				pBlockValue->mName = key;
				pBlockValue->mpChildren = _readChunk();

				return pBlockValue;
		}

		return nullptr;
	}

	U32 CBinaryArchiveReader::_readChunkTag()
	{
		U32 tagValue = 0x0;
		mpCachedInputStream->Read(&tagValue, sizeof(U32));
		return tagValue;
	}

	std::string CBinaryArchiveReader::_readStringChunk()
	{
		size_t length = 0;
		std::string keyValue = "";

		_readValue(&length, sizeof(size_t));

		keyValue.resize(length);
		_readValue(&keyValue[0], sizeof(C8) * length);

		return keyValue;
	}
	
	std::unique_ptr<TArchiveValue> CBinaryArchiveReader::_readIntValue(const std::string& key, U32 size)
	{
		I64 value = 0x0;
		_readValue(&value, size);

		auto pValue = std::make_unique<TArchiveValue>();
		pValue->mName = key;
		pValue->mValue = value;

		return pValue;
	}
	
	std::unique_ptr<TArchiveValue> CBinaryArchiveReader::_readFloatValue(const std::string& key, U32 size)
	{
		F64 value = 0.0f;
		_readValue(&value, size);

		auto pValue = std::make_unique<TArchiveValue>();
		pValue->mName = key;
		pValue->mValue = value;

		return pValue;
	}
	
	std::unique_ptr<TArchiveValue> CBinaryArchiveReader::_readBoolValue(const std::string& key)
	{
		bool value = false;
		_readValue(&value, sizeof(bool));

		auto pValue = std::make_unique<TArchiveValue>();
		pValue->mName = key;
		pValue->mValue = value;

		return pValue;
	}
	
	std::unique_ptr<TArchiveValue> CBinaryArchiveReader::_readStringValue(const std::string& key)
	{
		std::string value = _readStringChunk();

		auto pValue = std::make_unique<TArchiveValue>();
		pValue->mName = key;
		pValue->mValue = value;

		return pValue;
	}

	E_RESULT_CODE CBinaryArchiveReader::_readValue(void* pBuffer, U32 size)
	{
		return mpCachedInputStream->Read(pBuffer, size);
	}


	IFile* CreateBinaryArchiveReader(IMountableStorage* pStorage, IStream* pStream, E_RESULT_CODE& result)
	{
		CBinaryArchiveReader* pFileInstance = new (std::nothrow) CBinaryArchiveReader();

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