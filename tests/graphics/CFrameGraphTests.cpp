#include <catch2/catch_test_macros.hpp>
#include <TDEngine2.h>


using namespace TDEngine2;


TEST_CASE("CFrameGraph Tests")
{
	struct TFrameGraphTexture
	{
		TDE2_API TFrameGraphTexture() = default;
		TDE2_API TFrameGraphTexture(TFrameGraphTexture&&) = default;
		TDE2_API explicit TFrameGraphTexture(TTextureHandleId handle):
			mTextureHandle(handle)
		{
		}

		struct TDesc
		{
			U32 mWidth = 128;
			U32 mHeight = 128;
		};

		TDE2_API E_RESULT_CODE Acquire(IGraphicsObjectManager* pGraphicsObjectManager, const TDesc& desc) { return RC_OK; }
		TDE2_API E_RESULT_CODE Release(IGraphicsObjectManager* pGraphicsObjectManager) { return RC_OK; }

		TDE2_API void BeforeReadOp() {}
		TDE2_API void BeforeWriteOp() {}

		TTextureHandleId mTextureHandle = TTextureHandleId::Invalid;
	};


	E_RESULT_CODE result = RC_OK;

	TPtr<CFrameGraph> pFrameGraph = TPtr<CFrameGraph>(CreateFrameGraph(nullptr, result));

	SECTION("TestExecuteAndCompile_PassSinglePassWithSingleOutput_BothSetupAndExecuteShouldBeCreated")
	{
		struct TEmptyPassData 
		{
			TFrameGraphResourceHandle mOutput;
		};

		bool isSetupInvoked = false;
		bool isExecuteInvoked = false;

		E_RESULT_CODE result = RC_OK;
		
		auto pFileStream = TPtr<TDEngine2::IStream>(CreateFileOutputStream("framegraph_empty.dot", result));
		ITextFileWriter* pDumpWriter = dynamic_cast<ITextFileWriter*>(CreateTextFileWriter(nullptr, pFileStream, result));

		pFrameGraph->AddPass<TEmptyPassData>("Empty", [&](CFrameGraphBuilder& builder, TEmptyPassData& data) 
		{
			data.mOutput = builder.Create<TFrameGraphTexture>("OutputTarget", {});
			data.mOutput = builder.Write(data.mOutput);

			REQUIRE(data.mOutput != TFrameGraphResourceHandle::Invalid);

			isSetupInvoked = true;

			builder.MarkAsPersistent(); // Mark the pass as persistent one to prevent its culling
		}, [&](const TEmptyPassData& data, const TFramePassExecutionContext&, const std::string& renderPassName)
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
		}, [&](const TEmptyPassData&, const TFramePassExecutionContext&, const std::string& renderPassName)
		{
			isExecuteInvoked = true;
		});

		REQUIRE(RC_OK == pFrameGraph->Compile());
		REQUIRE(RC_OK == pFrameGraph->Execute());

		REQUIRE(!isExecuteInvoked);
	}

	SECTION("TestImportResource_AddImportedResourceAndTryToGetItWithinPass_TheResourceShouldBeCorrectlyRetrieved")
	{
		struct TEmptyPassData
		{
			TFrameGraphResourceHandle mOutput;
		};

		bool isSetupInvoked = false;
		bool isExecuteInvoked = false;

		E_RESULT_CODE result = RC_OK;

		auto pFileStream = TPtr<TDEngine2::IStream>(CreateFileOutputStream("framegraph_import.dot", result));
		ITextFileWriter* pDumpWriter = dynamic_cast<ITextFileWriter*>(CreateTextFileWriter(nullptr, pFileStream, result));
		
		constexpr TTextureHandleId expectedBackBufferTextureHandle = TTextureHandleId(42);

		TFrameGraphResourceHandle backBufferHandle = pFrameGraph->ImportResource<TFrameGraphTexture>("BackBuffer", { 1024, 768 }, TFrameGraphTexture { expectedBackBufferTextureHandle });

		pFrameGraph->AddPass<TEmptyPassData>("BackbufferDrawPass", [&](CFrameGraphBuilder& builder, TEmptyPassData& data)
		{
			data.mOutput = builder.Write(backBufferHandle);

			REQUIRE(data.mOutput != TFrameGraphResourceHandle::Invalid);

			isSetupInvoked = true;
		}, [&](const TEmptyPassData& data, const TFramePassExecutionContext& executionContext, const std::string& renderPassName)
		{
			TFrameGraphTexture& outputTex = executionContext.mpOwnerGraph->GetResource<TFrameGraphTexture>(data.mOutput);
			REQUIRE(expectedBackBufferTextureHandle == outputTex.mTextureHandle);

			isExecuteInvoked = true;
		});

		REQUIRE(RC_OK == pFrameGraph->Compile());
		REQUIRE(RC_OK == pFrameGraph->Execute());

		pFrameGraph->Dump(pDumpWriter);
		pDumpWriter->Close();

		REQUIRE(isSetupInvoked);
		REQUIRE(isExecuteInvoked);
	}


	SECTION("TestExecute_SameTargetProcessedThroughPasses_TheResourceVersionedCorrectly")
	{
		struct TEmptyPassData
		{
			TFrameGraphResourceHandle mOutput;
		};

		bool isSetupInvoked[2] = { false };

		E_RESULT_CODE result = RC_OK;

		auto pFileStream = TPtr<TDEngine2::IStream>(CreateFileOutputStream("framegraph_renamed.dot", result));
		ITextFileWriter* pDumpWriter = dynamic_cast<ITextFileWriter*>(CreateTextFileWriter(nullptr, pFileStream, result));

		auto&& pass1Data = pFrameGraph->AddPass<TEmptyPassData>("Pass1", [&](CFrameGraphBuilder& builder, TEmptyPassData& data)
		{
			data.mOutput = builder.Create<TFrameGraphTexture>("OutputTarget", {});
			data.mOutput = builder.Write(data.mOutput);

			REQUIRE(data.mOutput != TFrameGraphResourceHandle::Invalid);

			isSetupInvoked[0] = true;
		}, [&](const TEmptyPassData& data, const TFramePassExecutionContext& executionContext, const std::string& renderPassName)
		{
		});

		pFrameGraph->AddPass<TEmptyPassData>("Pass2", [&](CFrameGraphBuilder& builder, TEmptyPassData& data)
		{
			data.mOutput = builder.Write(builder.Read(pass1Data.mOutput));

			REQUIRE(data.mOutput != TFrameGraphResourceHandle::Invalid);

			isSetupInvoked[1] = true;
		}, [&](const TEmptyPassData& data, const TFramePassExecutionContext& executionContext, const std::string& renderPassName)
		{
		});

		REQUIRE(RC_OK == pFrameGraph->Compile());
		REQUIRE(RC_OK == pFrameGraph->Execute());

		pFrameGraph->Dump(pDumpWriter);
		pDumpWriter->Close();

		REQUIRE((isSetupInvoked[0] && isSetupInvoked[1]));
	}

	SECTION("TestGetResourceDesc_CreateTextureAndTryToGetItsDesc_TheMethodReturnsCorrectInformationAboutTexture")
	{
		const TFrameGraphTexture::TDesc expectedDesc { 1024, 768 };

		const TFrameGraphResourceHandle texHandle = pFrameGraph->ImportResource<TFrameGraphTexture>("TestTexture", expectedDesc, {});
		REQUIRE(texHandle != TFrameGraphResourceHandle::Invalid);

		const auto& actualTexDesc = pFrameGraph->GetResourceDesc<TFrameGraphTexture>(texHandle);
		REQUIRE((expectedDesc.mWidth == actualTexDesc.mWidth && expectedDesc.mHeight == actualTexDesc.mHeight));
	}
}