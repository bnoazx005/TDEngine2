#include <catch2/catch.hpp>
#include <TDEngine2.h>
#include <vector>
#include <string>
#include <tuple>


using namespace TDEngine2;


TEST_CASE("CMemoryInputStream Tests")
{
	E_RESULT_CODE result = RC_OK;

	SECTION("TestRead_MakeFewReads_WhenReachesEndReturnsTrue")
	{
		std::vector<U8> data{ 0x42, 0x16, 0x2, 0x4 };

		IInputStream* pStream = dynamic_cast<IInputStream*>(CreateMemoryInputStream("", data, result));

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
		REQUIRE(RC_OK == pStream->Free());
	}

	SECTION("TestReadToEnd_MakeRead_WhenReachesEndReturnsTrue")
	{
		std::vector<U8> data{ 0x42, 0x16, 0x2, 0x4 };

		IInputStream* pStream = dynamic_cast<IInputStream*>(CreateMemoryInputStream("", data, result));

		REQUIRE((pStream && RC_OK == result));
		{
			REQUIRE(pStream->GetLength() == data.size());

			std::string actualData = pStream->ReadToEnd();

			for (int i = 0; i < actualData.size(); ++i)
			{
				REQUIRE(static_cast<U8>(actualData[i]) == data[i]);
			}

			REQUIRE(pStream->IsEndOfStream());
		}
		REQUIRE(RC_OK == pStream->Free());
	}
}