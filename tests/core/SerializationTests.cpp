#include <catch2/catch_test_macros.hpp>
#include <TDEngine2.h>
#include <tuple>


using namespace TDEngine2;


template <typename T>
void TestDeserializationForType(const std::function<void(IYAMLFileWriter*)>& valueProvider)
{
	E_RESULT_CODE result = RC_OK;

	auto pMemoryMappedStream = TPtr<TDEngine2::IStream>(CreateMemoryIOStream(Wrench::StringUtils::GetEmptyStr(), {}, result));
	REQUIRE(pMemoryMappedStream);

	IYAMLFileWriter* pFileWriter = dynamic_cast<IYAMLFileWriter*>(CreateYAMLFileWriter(nullptr, pMemoryMappedStream, result));
	REQUIRE(pFileWriter);

	pFileWriter->SetUInt32("type_id", static_cast<U32>(::TDEngine2::GetTypeId<T>::mValue));
	valueProvider(pFileWriter);

	pFileWriter->Close();

	/// \note Read back the structure of the archive
	IYAMLFileReader* pFileReader = dynamic_cast<IYAMLFileReader*>(CreateYAMLFileReader(nullptr, pMemoryMappedStream, result));
	REQUIRE(pFileReader);

	auto wrappedValueResult = DeserializeValue(pFileReader);
	REQUIRE(wrappedValueResult.IsOk());

	auto value = wrappedValueResult.Get();
}


TEST_CASE("CValueWrapper Tests")
{
	SECTION("TestValueWrapperConstruction_PassBuiltinTypes_CorrectlyInitializesObjects")
	{
		auto objectsAndTypes =
		{
			std::tuple { TDE2_TYPE_ID(I32), CValueWrapper(-42) },
			std::tuple { TDE2_TYPE_ID(U32), CValueWrapper(0x42u) },
			std::tuple { TDE2_TYPE_ID(std::string), CValueWrapper(std::string("Hello, World")) },
			std::tuple { TDE2_TYPE_ID(F32), CValueWrapper(4.2f) },
			std::tuple { TDE2_TYPE_ID(bool), CValueWrapper(false) },
			std::tuple { TDE2_TYPE_ID(TVector2), CValueWrapper(TVector2(1.0f)) },
			std::tuple { TDE2_TYPE_ID(TVector3), CValueWrapper(TVector3(1.0f)) },
			std::tuple { TDE2_TYPE_ID(TColor32F), CValueWrapper(TColor32F(1.0f, 0.0f, 0.0f, 1.0f)) },
		};

		TypeId currTypeId;
		CValueWrapper value, v2;

		for (auto&& currTestCase : objectsAndTypes)
		{
			std::tie(currTypeId, value) = currTestCase;
			REQUIRE(currTypeId == value.GetTypeId());
		}
	}

	SECTION("TestDeserializeValue_TryToDeserializeBuiltinTypes_CorrectlyReturnsCValueWrapperObject")
	{
		TestDeserializationForType<std::string>([](IYAMLFileWriter* pFileWriter) { pFileWriter->SetString("value", "Test string"); });
		TestDeserializationForType<U32>([](IYAMLFileWriter* pFileWriter) { pFileWriter->SetUInt32("value", 42); });
		TestDeserializationForType<I32>([](IYAMLFileWriter* pFileWriter) { pFileWriter->SetInt32("value", -42); });
		TestDeserializationForType<F32>([](IYAMLFileWriter* pFileWriter) { pFileWriter->SetFloat("value", 0.42f); });
		TestDeserializationForType<F64>([](IYAMLFileWriter* pFileWriter) { pFileWriter->SetDouble("value", 0.42); });
		TestDeserializationForType<bool>([](IYAMLFileWriter* pFileWriter) { pFileWriter->SetBool("value", true); });
		TestDeserializationForType<TVector2>([](IYAMLFileWriter* pFileWriter) { SaveVector2(pFileWriter, TVector2(1.0f)); });
		TestDeserializationForType<TVector3>([](IYAMLFileWriter* pFileWriter) { SaveVector3(pFileWriter, TVector3(1.0f)); });
		TestDeserializationForType<TVector4>([](IYAMLFileWriter* pFileWriter) { SaveVector4(pFileWriter, TVector4(1.0f)); });
		TestDeserializationForType<TColor32F>([](IYAMLFileWriter* pFileWriter) { SaveColor32F(pFileWriter, TColorUtils::mBlue); });
	}

	SECTION("TestCastTo_PassCorrectValuesTryToCastToTheirOriginalTypes_AllValuesShouldBeCorrectlyRetrived")
	{
		REQUIRE(*CValueWrapper(-42).CastTo<I32>() == -42);
		REQUIRE(*CValueWrapper(0x42u).CastTo<U32>() == 0x42);
		REQUIRE(*CValueWrapper(std::string("Hello, World")).CastTo<std::string>() == "Hello, World");
		REQUIRE(*CValueWrapper(4.2f).CastTo<F32>() == 4.2f);
		REQUIRE(*CValueWrapper(false).CastTo<bool>() == false);
		REQUIRE(*CValueWrapper(TVector2(1.0f)).CastTo<TVector2>() == TVector2(1.0f));
		REQUIRE(*CValueWrapper(TVector3(1.0f)).CastTo<TVector3>() == TVector3(1.0f));
	}
}