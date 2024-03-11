#include <catch2/catch_test_macros.hpp>
#include <TDEngine2.h>


using namespace TDEngine2;


TEST_CASE("CFrameGraph Tests")
{
	struct TFrameGraphTexture
	{
		struct TDesc
		{
			U32 mWidth = 128;
			U32 mHeight = 128;
		};

		TDE2_API void Acquire() {}
		TDE2_API void Release() {}

		TDE2_API void BeforeReadOp() {}
		TDE2_API void BeforeWriteOp() {}
	};


	E_RESULT_CODE result = RC_OK;

	TPtr<CFrameGraph> pFrameGraph = TPtr<CFrameGraph>(CreateFrameGraph(result));

	SECTION("TestExecuteAndCompile_PassSinglePassWithSingleOutput_BothSetupAndExecuteShouldBeCreated")
	{
		struct TEmptyPassData 
		{
			TFrameGraphResourceHandle mOutput;
		};

		bool isSetupInvoked = false;
		bool isExecuteInvoked = false;

		E_RESULT_CODE result = RC_OK;

		auto pFileStream = TPtr<TDEngine2::IStream>(CreateFileOutputStream("framegraph.dot", result));
		ITextFileWriter* pDumpWriter = dynamic_cast<ITextFileWriter*>(CreateTextFileWriter(nullptr, pFileStream, result));

		pFrameGraph->AddPass<TEmptyPassData>("Empty", [&](CFrameGraphBuilder& builder, TEmptyPassData& data) 
		{
			data.mOutput = builder.Create<TFrameGraphTexture>("OutputTarget", {});
			data.mOutput = builder.Write(data.mOutput);

			REQUIRE(data.mOutput != TFrameGraphResourceHandle::Invalid);

			isSetupInvoked = true;

			builder.MarkAsPersistent(); // Mark the pass as persistent one to prevent its culling
		}, [&](const TEmptyPassData& data) 
		{
			TFrameGraphTexture& outputTex = pFrameGraph->GetResource<TFrameGraphTexture>(data.mOutput);

			isExecuteInvoked = true;
		});

		REQUIRE(RC_OK == pFrameGraph->Compile());
		REQUIRE(RC_OK == pFrameGraph->Execute());

		pFrameGraph->Dump(pDumpWriter);
		pDumpWriter->Close();

		REQUIRE(isSetupInvoked);
		REQUIRE(isExecuteInvoked);
	}

	SECTION("TestExecuteAndCompile_PassUnreferencedPass_PassShouldBeCulled")
	{
		struct TEmptyPassData
		{
		};

		bool isExecuteInvoked = false;

		pFrameGraph->AddPass<TEmptyPassData>("Culled", [&](CFrameGraphBuilder& builder, TEmptyPassData& data)
		{
		}, [&](const TEmptyPassData&)
		{
			isExecuteInvoked = true;
		});

		REQUIRE(RC_OK == pFrameGraph->Compile());
		REQUIRE(RC_OK == pFrameGraph->Execute());

		REQUIRE(!isExecuteInvoked);
	}
}