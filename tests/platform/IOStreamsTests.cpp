#include <catch2/catch_test_macros.hpp>
#include <TDEngine2.h>
#include <vector>
#include <string>
#include <tuple>


using namespace TDEngine2;


TEST_CASE("CMemoryIOStream Tests")
{
	E_RESULT_CODE result = RC_OK;

	SECTION("TestRead_MakeFewReads_WhenReachesEndReturnsTrue")
	{
		std::vector<U8> data{ 0x42, 0x16, 0x2, 0x4 };

		TPtr<IInputStream> pStream = DynamicPtrCast<IInputStream>(TPtr<TDEngine2::IStream>(CreateMemoryIOStream(Wrench::StringUtils::GetEmptyStr(), data, result)));

		REQUIRE((pStream && RC_OK == result));
		{
			U8 actualValue = 0;

			for (auto currValue : data)
			{
				REQUIRE(RC_OK == pStream->Read(&actualValue, 1));
				REQUIRE(actualValue == currValue);
			}

			REQUIRE(pStream->IsEndOfStream());
		}
	}

	SECTION("TestReadToEnd_MakeRead_WhenReachesEndReturnsTrue")
	{
		std::vector<U8> data{ 0x42, 0x16, 0x2, 0x4 };

		TPtr<IInputStream> pStream = DynamicPtrCast<IInputStream>(TPtr<TDEngine2::IStream>(CreateMemoryIOStream(Wrench::StringUtils::GetEmptyStr(), data, result)));

		REQUIRE((pStream && RC_OK == result));
		{
			REQUIRE(pStream->GetLength() == data.size());

			std::string actualData = pStream->ReadToEnd();

			for (size_t i = 0; i < actualData.size(); ++i)
			{
				REQUIRE(static_cast<U8>(actualData[i]) == data[i]);
			}

			REQUIRE(pStream->IsEndOfStream());
		}
	}

	SECTION("TestRead_OverrunDataSize_ReturnsRC_FAIL")
	{
		std::vector<U8> data{ 0x42, 0x16, 0x2, 0x4 };

		TPtr<IInputStream> pStream = DynamicPtrCast<IInputStream>(TPtr<TDEngine2::IStream>(CreateMemoryIOStream(Wrench::StringUtils::GetEmptyStr(), data, result)));

		REQUIRE((pStream && RC_OK == result));
		{
			std::vector<U8> buffer;
			buffer.resize(10);

			REQUIRE(pStream->Read(static_cast<void*>(&buffer[0]), 10) == RC_OK);
			REQUIRE(pStream->IsEndOfStream());
		}
	}

	SECTION("TestWrite_WriteBytesArrayThenTryToReadBack_BytesShouldBeCorrectlyProcessed")
	{
		std::vector<U8> data{ 0x42, 0x16, 0x2, 0x4 };

		TPtr<IOutputStream> pOutStream = DynamicPtrCast<IOutputStream>(TPtr<TDEngine2::IStream>(CreateMemoryIOStream(Wrench::StringUtils::GetEmptyStr(), {}, result)));
		REQUIRE(pOutStream);

		REQUIRE(RC_OK == pOutStream->Write(&data.front(), sizeof(U8) * data.size()));

		TPtr<IInputStream> pInStream = DynamicPtrCast<IInputStream>(pOutStream);
		
		std::vector<U8> buffer;
		buffer.resize(data.size());

		pInStream->SetPosition(0);
		REQUIRE(RC_OK == pInStream->Read(&buffer.front(), buffer.size()));

		REQUIRE(buffer.size() == data.size());

		for (USIZE i = 0; i < buffer.size(); i++)
		{
			REQUIRE(buffer[i] == data[i]);
		}
	}
}