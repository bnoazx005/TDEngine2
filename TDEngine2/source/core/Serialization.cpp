#include "../../include/core/Serialization.h"


namespace TDEngine2
{
#define TDE2_SERIALIZE_IMPL(Type, SetterMethod, value) \
	pWriter->SetUInt32("type_id", static_cast<U32>(GetTypeId<Type>::mValue)) | pWriter->SetterMethod("value", value)
	

	/*!
		\brief Built in types serialization helpers
	*/

	template <> TDE2_API E_RESULT_CODE Serialize<I8>(IArchiveWriter* pWriter, I8&& value) { return TDE2_SERIALIZE_IMPL(I8, SetInt8, value); }
	template <> TDE2_API E_RESULT_CODE Serialize<I16>(IArchiveWriter* pWriter, I16&& value) { return TDE2_SERIALIZE_IMPL(I16, SetInt16, value); }
	template <> TDE2_API E_RESULT_CODE Serialize<I32>(IArchiveWriter* pWriter, I32&& value) { return TDE2_SERIALIZE_IMPL(I32, SetInt32, value); }
	template <> TDE2_API E_RESULT_CODE Serialize<I64>(IArchiveWriter* pWriter, I64&& value) { return TDE2_SERIALIZE_IMPL(I64, SetInt64, value); }

	template <> TDE2_API E_RESULT_CODE Serialize<U8>(IArchiveWriter* pWriter, U8&& value) { return TDE2_SERIALIZE_IMPL(U8, SetUInt8, value); }
	template <> TDE2_API E_RESULT_CODE Serialize<U16>(IArchiveWriter* pWriter, U16&& value) { return TDE2_SERIALIZE_IMPL(U16, SetUInt16, value); }
	template <> TDE2_API E_RESULT_CODE Serialize<U32>(IArchiveWriter* pWriter, U32&& value) { return TDE2_SERIALIZE_IMPL(U32, SetUInt32, value); }
	template <> TDE2_API E_RESULT_CODE Serialize<U64>(IArchiveWriter* pWriter, U64&& value) { return TDE2_SERIALIZE_IMPL(U64, SetUInt64, value); }

	template <> TDE2_API E_RESULT_CODE Serialize<F32>(IArchiveWriter* pWriter, F32&& value) { return TDE2_SERIALIZE_IMPL(F32, SetFloat, value); }
	template <> TDE2_API E_RESULT_CODE Serialize<F64>(IArchiveWriter* pWriter, F64&& value) { return TDE2_SERIALIZE_IMPL(F64, SetDouble, value); }

	template <> TDE2_API E_RESULT_CODE Serialize<bool>(IArchiveWriter* pWriter, bool&& value) { return TDE2_SERIALIZE_IMPL(bool, SetBool, value); }
	template <> TDE2_API E_RESULT_CODE Serialize<std::string>(IArchiveWriter* pWriter, std::string&& value) { return TDE2_SERIALIZE_IMPL(bool, SetString, value); }
}