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


	static E_RESULT_CODE WriteStringInternal(IOutputStream* pStream, const std::string& value)
	{
		if (!pStream)
		{
			return RC_FAIL;
		}

		const USIZE length = SwapBytes(value.length());
		const C8* pStrBuffer = value.c_str();

		E_RESULT_CODE result = pStream->Write(static_cast<const void*>(&length), sizeof(length));
		result = result | pStream->Write(static_cast<const void*>(pStrBuffer), sizeof(C8) * length);

		return result;
	}


	static inline E_RESULT_CODE WriteTagValueImpl(IOutputStream* pStream, U32 tag)
	{
		tag = SwapBytes(tag);

		return pStream->Write(&tag, sizeof(tag));
	}


	template <typename T>
	static E_RESULT_CODE WriteValueImpl(IOutputStream* pStream, const std::string& key, const T& value)
	{
		E_RESULT_CODE result = WriteStringInternal(pStream, key);

		// \note Write tag of the type's value
		const U32 tag = SwapBytes(static_cast<U32>(TArchiveValue::GetValueTag<T>()));

		result = result | pStream->Write(&tag, sizeof(tag));
		result = result | pStream->Write(static_cast<const void*>(&value), sizeof(value));

		return result;
	}


	template <>
	E_RESULT_CODE WriteValueImpl<std::string>(IOutputStream* pStream, const std::string& key, const std::string& value)
	{
		E_RESULT_CODE result = WriteStringInternal(pStream, key);

		// \note Write tag of the type's value
		const U32 tag = SwapBytes(static_cast<U32>(TDE2_TYPE_ID(std::string)));

		result = result | pStream->Write(&tag, sizeof(tag));
		result = result | WriteStringInternal(pStream, value);

		return result;
	}


	CBinaryArchiveWriter::CBinaryArchiveWriter() :
		CBaseFile()
	{
	}

	E_RESULT_CODE CBinaryArchiveWriter::Open(IMountableStorage* pStorage, TPtr<IStream> pStream)
	{
		E_RESULT_CODE result = CBaseFile::Open(pStorage, pStream);
		if (result != RC_OK)
		{
			return result;
		}

		std::lock_guard<std::mutex> lock(mMutex);

		mpCachedOutputStream = dynamic_cast<IOutputStream*>(mpStreamImpl.Get());
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

		result = result | WriteStringInternal(mpCachedOutputStream, key);
		result = result | WriteTagValueImpl(mpCachedOutputStream, TArchiveValue::mBeginBlockTag);

		return result;
	}

	E_RESULT_CODE CBinaryArchiveWriter::EndGroup()
	{
		std::lock_guard<std::mutex> lock(mMutex);

		WriteTagValueImpl(mpCachedOutputStream, TArchiveValue::mEndBlockTag);

		return RC_OK;
	}

	E_RESULT_CODE CBinaryArchiveWriter::SetUInt8(const std::string& key, U8 value)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return WriteValueImpl<U8>(mpCachedOutputStream, key, value);
	}

	E_RESULT_CODE CBinaryArchiveWriter::SetUInt16(const std::string& key, U16 value)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return WriteValueImpl<U16>(mpCachedOutputStream, key, SwapBytes(value));
	}

	E_RESULT_CODE CBinaryArchiveWriter::SetUInt32(const std::string& key, U32 value)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return WriteValueImpl<U32>(mpCachedOutputStream, key, SwapBytes(value));
	}

	E_RESULT_CODE CBinaryArchiveWriter::SetUInt64(const std::string& key, U64 value)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return WriteValueImpl<U64>(mpCachedOutputStream, key, SwapBytes(value));
	}

	E_RESULT_CODE CBinaryArchiveWriter::SetInt8(const std::string& key, I8 value)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return WriteValueImpl<I8>(mpCachedOutputStream, key, value);
	}

	E_RESULT_CODE CBinaryArchiveWriter::SetInt16(const std::string& key, I16 value)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return WriteValueImpl<I16>(mpCachedOutputStream, key, SwapBytes(value));
	}

	E_RESULT_CODE CBinaryArchiveWriter::SetInt32(const std::string& key, I32 value)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return WriteValueImpl<I32>(mpCachedOutputStream, key, SwapBytes(value));
	}

	E_RESULT_CODE CBinaryArchiveWriter::SetInt64(const std::string& key, I64 value)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return WriteValueImpl<I64>(mpCachedOutputStream, key, SwapBytes(value));
	}

	E_RESULT_CODE CBinaryArchiveWriter::SetFloat(const std::string& key, F32 value)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return WriteValueImpl<F32>(mpCachedOutputStream, key, SwapBytes(value));
	}

	E_RESULT_CODE CBinaryArchiveWriter::SetDouble(const std::string& key, F64 value)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return WriteValueImpl<F64>(mpCachedOutputStream, key, SwapBytes(value));
	}

	E_RESULT_CODE CBinaryArchiveWriter::SetBool(const std::string& key, bool value)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return WriteValueImpl<bool>(mpCachedOutputStream, key, SwapBytes(value));
	}

	E_RESULT_CODE CBinaryArchiveWriter::SetString(const std::string& key, const std::string& value)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return WriteValueImpl<std::string>(mpCachedOutputStream, key, value);
	}

	E_RESULT_CODE CBinaryArchiveWriter::_onFree()
	{
		return RC_OK;
	}


	IFile* CreateBinaryArchiveWriter(IMountableStorage* pStorage, TPtr<IStream> pStream, E_RESULT_CODE& result)
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

	template <> TDE2_API I8 CBinaryArchiveReader::_tryToGetValue<I8>(const TArchiveValue::TValue& value, I8 defaultValue) { return !value.Is<I64>() ? defaultValue : static_cast<I8>(value.As<I64>()); }
	template <> TDE2_API I16 CBinaryArchiveReader::_tryToGetValue<I16>(const TArchiveValue::TValue& value, I16 defaultValue) { return !value.Is<I64>() ? defaultValue : static_cast<I16>(value.As<I64>()); }
	template <> TDE2_API I32 CBinaryArchiveReader::_tryToGetValue<I32>(const TArchiveValue::TValue& value, I32 defaultValue) { return !value.Is<I64>() ? defaultValue : static_cast<I32>(value.As<I64>()); }

	template <> TDE2_API U8 CBinaryArchiveReader::_tryToGetValue<U8>(const TArchiveValue::TValue& value, U8 defaultValue) { return !value.Is<I64>() ? defaultValue : static_cast<U8>(value.As<I64>()); }
	template <> TDE2_API U16 CBinaryArchiveReader::_tryToGetValue<U16>(const TArchiveValue::TValue& value, U16 defaultValue) { return !value.Is<I64>() ? defaultValue : static_cast<U16>(value.As<I64>()); }
	template <> TDE2_API U32 CBinaryArchiveReader::_tryToGetValue<U32>(const TArchiveValue::TValue& value, U32 defaultValue) { return !value.Is<I64>() ? defaultValue : static_cast<U32>(value.As<I64>()); }
	template <> TDE2_API U64 CBinaryArchiveReader::_tryToGetValue<U64>(const TArchiveValue::TValue& value, U64 defaultValue) { return !value.Is<I64>() ? defaultValue : static_cast<U64>(value.As<I64>()); }
	
	template <> TDE2_API F32 CBinaryArchiveReader::_tryToGetValue<F32>(const TArchiveValue::TValue& value, F32 defaultValue) { return !value.Is<F64>() ? defaultValue : static_cast<F32>(value.As<F64>()); }
	

	CBinaryArchiveReader::CBinaryArchiveReader() :
		CBaseFile()
	{
	}

	E_RESULT_CODE CBinaryArchiveReader::Open(IMountableStorage* pStorage, TPtr<IStream> pStream)
	{
		E_RESULT_CODE result = CBaseFile::Open(pStorage, pStream);
		if (result != RC_OK)
		{
			return result;
		}

		std::lock_guard<std::mutex> lock(mMutex);

		mpCachedInputStream = dynamic_cast<IInputStream*>(mpStreamImpl.Get());
		if (!mpCachedInputStream)
		{
			return RC_FAIL;
		}

		return _deserializeInternal();
	}

	E_RESULT_CODE CBinaryArchiveReader::BeginGroup(const std::string& key)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		// \note try to find node with given identifier
		auto&& groups = mpCurrNode ? mpCurrNode->mpChildren : mTopLevelDecls;

		auto iter = key.empty() ? (groups.cbegin() + mCurrElementIndex) : std::find_if(groups.cbegin(), groups.cend(), [key](auto&& group) { return group->mName == key; });
		if (iter == groups.cend())
		{
			return RC_FAIL;
		}

		mpHierarchyContext.emplace(mCurrElementIndex, mpCurrNode); // \note save current pointer
		mCurrElementIndex = 0;

		// \note move down into hierarchy
		mpCurrNode = iter->get();

		return RC_OK;
	}

	E_RESULT_CODE CBinaryArchiveReader::EndGroup()
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (!mpCurrNode || mpHierarchyContext.empty())
		{
			return RC_FAIL;
		}

		std::tie(mCurrElementIndex, mpCurrNode) = mpHierarchyContext.top();
		mpHierarchyContext.pop();

		++mCurrElementIndex;

		return RC_OK;
	}

	bool CBinaryArchiveReader::HasNextItem() const
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return (mCurrElementIndex + 1) <= static_cast<U32>(mpCurrNode ? mpCurrNode->mpChildren.size() : 0);
	}

	U8 CBinaryArchiveReader::GetUInt8(const std::string& key, U8 defaultValue)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _getContentAsOrDefault<U8>(key, defaultValue);
	}

	U16 CBinaryArchiveReader::GetUInt16(const std::string& key, U16 defaultValue)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _getContentAsOrDefault<U16>(key, defaultValue);
	}

	U32 CBinaryArchiveReader::GetUInt32(const std::string& key, U32 defaultValue)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _getContentAsOrDefault<U32>(key, defaultValue);
	}

	U64 CBinaryArchiveReader::GetUInt64(const std::string& key, U64 defaultValue)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _getContentAsOrDefault<U64>(key, defaultValue);
	}

	I8 CBinaryArchiveReader::GetInt8(const std::string& key, I8 defaultValue)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _getContentAsOrDefault<I8>(key, defaultValue);
	}

	I16 CBinaryArchiveReader::GetInt16(const std::string& key, I16 defaultValue)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _getContentAsOrDefault<I16>(key, defaultValue);
	}

	I32 CBinaryArchiveReader::GetInt32(const std::string& key, I32 defaultValue)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _getContentAsOrDefault<I32>(key, defaultValue);
	}

	I64 CBinaryArchiveReader::GetInt64(const std::string& key, I64 defaultValue)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _getContentAsOrDefault<I64>(key, defaultValue);
	}

	F32 CBinaryArchiveReader::GetFloat(const std::string& key, F32 defaultValue)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _getContentAsOrDefault<F32>(key, defaultValue);
	}

	F64 CBinaryArchiveReader::GetDouble(const std::string& key, F64 defaultValue)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _getContentAsOrDefault<F64>(key, defaultValue);
	}

	bool CBinaryArchiveReader::GetBool(const std::string& key, bool defaultValue)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _getContentAsOrDefault<bool>(key, defaultValue);
	}

	std::string CBinaryArchiveReader::GetString(const std::string& key, const std::string& defaultValue)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _getContentAsOrDefault<std::string>(key, defaultValue);
	}

	std::string CBinaryArchiveReader::GetCurrKey() const
	{
		std::lock_guard<std::mutex> lock(mMutex);

		return mpCurrNode ? mpCurrNode->mName : Wrench::StringUtils::GetEmptyStr();
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

			mpCachedInputStream->SetPosition(std::max<TSizeType>(0, mpCachedInputStream->GetPosition() - sizeof(U32)));

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

		length = SwapBytes(length);

		keyValue.resize(length);
		_readValue(&keyValue[0], sizeof(C8) * length);

		return keyValue;
	}
	
	std::unique_ptr<TArchiveValue> CBinaryArchiveReader::_readIntValue(const std::string& key, TSizeType size)
	{
		I64 value = 0x0;
		_readValue(&value, size);

		value = SwapBytes(value);

		auto pValue = std::make_unique<TArchiveValue>();
		pValue->mName = key;
		pValue->mValue = value;

		return pValue;
	}
	
	std::unique_ptr<TArchiveValue> CBinaryArchiveReader::_readFloatValue(const std::string& key, TSizeType size)
	{
		F64 value = 0.0f;
		_readValue(&value, size);

		value = SwapBytes(value);

		auto pValue = std::make_unique<TArchiveValue>();
		pValue->mName = key;
		pValue->mValue = value;

		return pValue;
	}
	
	std::unique_ptr<TArchiveValue> CBinaryArchiveReader::_readBoolValue(const std::string& key)
	{
		bool value = false;
		_readValue(&value, sizeof(bool));

		value = SwapBytes(value);

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

	E_RESULT_CODE CBinaryArchiveReader::_readValue(void* pBuffer, TSizeType size)
	{
		return mpCachedInputStream->Read(pBuffer, size);
	}


	IFile* CreateBinaryArchiveReader(IMountableStorage* pStorage, TPtr<IStream> pStream, E_RESULT_CODE& result)
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