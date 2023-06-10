#include <catch2/catch_test_macros.hpp>
#include <TDEngine2.h>
#include <vector>
#include <string>
#include <tuple>


using namespace TDEngine2;


TEST_CASE("CPrefabChangesList Tests")
{
	E_RESULT_CODE result = RC_OK;

	SECTION("TestLoad_PassCorrectSequenceThenTryToApplyToEntity_AllSpecifiedChangesAreAppliedToEntity")
	{
		auto pMemoryMappedStream = TPtr<TDEngine2::IStream>(CreateMemoryIOStream(Wrench::StringUtils::GetEmptyStr(), {}, result));
		REQUIRE(pMemoryMappedStream);

		IYAMLFileWriter* pFileWriter = dynamic_cast<IYAMLFileWriter*>(CreateYAMLFileWriter(nullptr, pMemoryMappedStream, result));
		REQUIRE(pFileWriter);

		pFileWriter->BeginGroup("changes", true);
		{
			pFileWriter->BeginGroup(Wrench::StringUtils::GetEmptyStr());
			pFileWriter->BeginGroup("element");

			CEntityRef targetRef(nullptr, TEntityId(0));
			SaveEntityRef(pFileWriter, "target_id", targetRef);

			pFileWriter->SetString("property_binding", "transform.position");
			{
				pFileWriter->BeginGroup("value");
				pFileWriter->SetUInt32("type_id", static_cast<U32>(TDE2_TYPE_ID(TVector3)));
				SaveVector3(pFileWriter, TVector3(1.0f, 2.0f, 3.0f));
				pFileWriter->EndGroup();
			}
			pFileWriter->EndGroup();
			pFileWriter->EndGroup();
		}
		pFileWriter->EndGroup();

		pFileWriter->Close();

		/// \note Read back the structure of the archive
		IYAMLFileReader* pFileReader = dynamic_cast<IYAMLFileReader*>(CreateYAMLFileReader(nullptr, pMemoryMappedStream, result));
		REQUIRE(pFileReader);

		auto pPrefabChangesList = TPtr<CPrefabChangesList>(CreatePrefabChangesList(result));
		REQUIRE(pPrefabChangesList);

		REQUIRE(RC_OK == pPrefabChangesList->Load(pFileReader));

		/// \todo Complete the test case
	}
}