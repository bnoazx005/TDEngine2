#include "../../include/core/Serialization.h"
#include "../../include/math/TVector4.h"
#include "../../include/utils/Color.h"


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
	template <> TDE2_API E_RESULT_CODE Serialize<std::string>(IArchiveWriter* pWriter, std::string value) { return TDE2_SERIALIZE_IMPL(std::string, SetString, value); }

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
  

	/*!
		\brief CValueWrapper's definition
	*/

	CValueWrapper::CValueWrapper(const CValueWrapper& object):
		mpImpl(object.mpImpl->Clone())
	{
	}

	CValueWrapper::CValueWrapper(CValueWrapper&& object) noexcept:
		mpImpl(std::move(object.mpImpl))
	{
		object.mpImpl = nullptr;
	}

	E_RESULT_CODE CValueWrapper::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_INVALID_ARGS;
		}

		return mpImpl ? mpImpl->Load(pReader) : RC_FAIL;
	}

	E_RESULT_CODE CValueWrapper::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_INVALID_ARGS;
		}

		return mpImpl ? mpImpl->Save(pWriter) : RC_FAIL;
	}

	E_RESULT_CODE CValueWrapper::Apply(TPtr<IPropertyWrapper> pPropertyWrapper)
	{
		if (!pPropertyWrapper)
		{
			return RC_INVALID_ARGS;
		}

		return mpImpl ? mpImpl->Apply(pPropertyWrapper) : RC_FAIL;
	}

	E_RESULT_CODE CValueWrapper::Set(TPtr<IPropertyWrapper> pPropertyWrapper)
	{
		if (!pPropertyWrapper)
		{
			return RC_INVALID_ARGS;
		}

		if (!mpImpl || pPropertyWrapper->GetValueType() != mpImpl->GetTypeId())
		{
			mpImpl = pPropertyWrapper->ToValueWrapper();
			return RC_OK;
		}

		return mpImpl ? mpImpl->Set(pPropertyWrapper) : RC_FAIL;
	}

	CValueWrapper& CValueWrapper::operator= (CValueWrapper object)
	{
		this->mpImpl = std::move(object.mpImpl);
		return *this;
	}

	TypeId CValueWrapper::GetTypeId() const
	{
		return mpImpl ? mpImpl->GetTypeId() : TypeId::Invalid;
	}


	TResult<CValueWrapper> DeserializeValue(IArchiveReader* pReader)
	{
		auto typeId = static_cast<TypeId>(pReader->GetUInt32("type_id"));
		if (TypeId::Invalid == typeId)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		/// \todo \fixme Replace with common engine registry of types maybe it should be implemented in Meta.h
		static const std::unordered_map<TypeId, std::function<CValueWrapper()>> TypedDeserializers
		{
			{ TDE2_TYPE_ID(I32), []() { return CValueWrapper{ 0 }; }},
			{ TDE2_TYPE_ID(U32), []() { return CValueWrapper{ 0u }; }},

			{ TDE2_TYPE_ID(F32), []() { return CValueWrapper{ 0.0f }; }},
			{ TDE2_TYPE_ID(F64), []() { return CValueWrapper{ 0.0 }; }},

			{ TDE2_TYPE_ID(bool), []() { return CValueWrapper{ false }; }},

			{ GetTypeId<std::string>::mValue, []() { return CValueWrapper{ std::string() }; }},

			{ TDE2_TYPE_ID(TVector2), []() { return CValueWrapper{ TVector2() }; }},
			{ TDE2_TYPE_ID(TVector3), []() { return CValueWrapper{ TVector3() }; }},
			{ TDE2_TYPE_ID(TVector4), []() { return CValueWrapper{ TVector4() }; }},

			{ TDE2_TYPE_ID(TColor32F), []() { return CValueWrapper{ TColor32F() }; }},
		};

		auto it = TypedDeserializers.find(typeId);
		if (it == TypedDeserializers.cend())
		{
			TDE2_ASSERT(false);
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		auto wrappedObject = (it->second)();

		wrappedObject.Load(pReader);

		return Wrench::TOkValue<CValueWrapper>(wrappedObject);
	}
}