#include "../../include/core/Serialization.h"


namespace TDEngine2
{
#define TDE2_SERIALIZE_IMPL(Type, SetterMethod, value) \
	pWriter->SetUInt32("type_id", static_cast<U32>(GetTypeId<Type>::mValue)) | pWriter->SetterMethod("value", value)
	

	/*!
		\brief Built in types serialization helpers
	*/

	template <> TDE2_API E_RESULT_CODE Serialize<I8>(IArchiveWriter* pWriter, I8 value) { return TDE2_SERIALIZE_IMPL(I8, SetInt8, value); }
	template <> TDE2_API E_RESULT_CODE Serialize<I16>(IArchiveWriter* pWriter, I16 value) { return TDE2_SERIALIZE_IMPL(I16, SetInt16, value); }
	template <> TDE2_API E_RESULT_CODE Serialize<I32>(IArchiveWriter* pWriter, I32 value) { return TDE2_SERIALIZE_IMPL(I32, SetInt32, value); }
	template <> TDE2_API E_RESULT_CODE Serialize<I64>(IArchiveWriter* pWriter, I64 value) { return TDE2_SERIALIZE_IMPL(I64, SetInt64, value); }

	template <> TDE2_API E_RESULT_CODE Serialize<U8>(IArchiveWriter* pWriter, U8 value) { return TDE2_SERIALIZE_IMPL(U8, SetUInt8, value); }
	template <> TDE2_API E_RESULT_CODE Serialize<U16>(IArchiveWriter* pWriter, U16 value) { return TDE2_SERIALIZE_IMPL(U16, SetUInt16, value); }
	template <> TDE2_API E_RESULT_CODE Serialize<U32>(IArchiveWriter* pWriter, U32 value) { return TDE2_SERIALIZE_IMPL(U32, SetUInt32, value); }
	template <> TDE2_API E_RESULT_CODE Serialize<U64>(IArchiveWriter* pWriter, U64 value) { return TDE2_SERIALIZE_IMPL(U64, SetUInt64, value); }

	template <> TDE2_API E_RESULT_CODE Serialize<F32>(IArchiveWriter* pWriter, F32 value) { return TDE2_SERIALIZE_IMPL(F32, SetFloat, value); }
	template <> TDE2_API E_RESULT_CODE Serialize<F64>(IArchiveWriter* pWriter, F64 value) { return TDE2_SERIALIZE_IMPL(F64, SetDouble, value); }

	template <> TDE2_API E_RESULT_CODE Serialize<bool>(IArchiveWriter* pWriter, bool value) { return TDE2_SERIALIZE_IMPL(bool, SetBool, value); }
	template <> TDE2_API E_RESULT_CODE Serialize<std::string>(IArchiveWriter* pWriter, std::string value) { return TDE2_SERIALIZE_IMPL(bool, SetString, value); }

	/*!
		\brief Built in types deserialization helpers
	*/

	template <> TDE2_API TResult<I8> Deserialize<I8>(IArchiveReader* pReader) { return Wrench::TOkValue<I8>(pReader->GetInt8("value")); }
	template <> TDE2_API TResult<I16> Deserialize<I16>(IArchiveReader* pReader) { return Wrench::TOkValue<I16>(pReader->GetInt16("value")); }
	template <> TDE2_API TResult<I32> Deserialize<I32>(IArchiveReader* pReader) { return Wrench::TOkValue<I32>(pReader->GetInt32("value")); }
	template <> TDE2_API TResult<I64> Deserialize<I64>(IArchiveReader* pReader) { return Wrench::TOkValue<I64>(pReader->GetInt64("value")); }

	template <> TDE2_API TResult<U8> Deserialize<U8>(IArchiveReader* pReader) { return Wrench::TOkValue<U8>(pReader->GetUInt8("value")); }
	template <> TDE2_API TResult<U16> Deserialize<U16>(IArchiveReader* pReader) { return Wrench::TOkValue<U16>(pReader->GetUInt16("value")); }
	template <> TDE2_API TResult<U32> Deserialize<U32>(IArchiveReader* pReader) { return Wrench::TOkValue<U32>(pReader->GetUInt32("value")); }
	template <> TDE2_API TResult<U64> Deserialize<U64>(IArchiveReader* pReader) { return Wrench::TOkValue<U64>(pReader->GetUInt64("value")); }

	template <> TDE2_API TResult<F32> Deserialize<F32>(IArchiveReader* pReader) { return Wrench::TOkValue<F32>(pReader->GetFloat("value")); }
	template <> TDE2_API TResult<F64> Deserialize<F64>(IArchiveReader* pReader) { return Wrench::TOkValue<F64>(pReader->GetDouble("value")); }

	template <> TDE2_API TResult<bool> Deserialize<bool>(IArchiveReader* pReader) { return Wrench::TOkValue<bool>(pReader->GetBool("value")); }
	template <> TDE2_API TResult<std::string> Deserialize<std::string>(IArchiveReader* pReader) { return Wrench::TOkValue<std::string>(pReader->GetString("value")); }
  
}