#include "../../include/autotests/CTestContext.h"
#include "../../include/autotests/CBaseTestFixture.h"
#include "../../include/autotests/ITestResultsReporter.h"
#include "../../include/core/CProxyInputContext.h"
#include "../../include/core/IWindowSystem.h"
#include "../../include/core/IEngineCore.h"
#include "../../include/core/IFileSystem.h"
#include "../../include/core/IGraphicsContext.h"
#include "../../include/core/IFile.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"
#include <chrono>
#include <ctime>
#include <cmath>
#include <numeric>


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	TDE2_DEFINE_SCOPED_PTR(ITestResultsReporter)


	static std::unique_ptr<TProxyInputContextDesc> pProxyInputContextDesc{ new TProxyInputContextDesc() };


	TTestFixtureAutoRegister::TTestFixtureAutoRegister(std::function<void()> action)
	{
		if (action) 
		{
			action(); 
		}
	}


	CTestContext::CTestContext():
		CBaseObject()
	{
	}

	E_RESULT_CODE CTestContext::Init(const TTestContextConfig& config)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!config.mpEngineCore)
		{
			return RC_INVALID_ARGS;
		}

		mpEngineCore = config.mpEngineCore;

		E_RESULT_CODE result = RC_OK;

		auto pInputContext = TPtr<IEngineSubsystem>(CreateProxyInputContext(pProxyInputContextDesc.get(), mpEngineCore->GetSubsystem<IWindowSystem>(), result));
		if (RC_OK != result || !pInputContext)
		{
			return result;
		}

		if (RC_OK != (result = mpEngineCore->RegisterSubsystem(pInputContext)))
		{
			return result;
		}

		mpProxyInputContext = dynamic_cast<IDesktopInputContext*>(pInputContext.Get());
		
		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CTestContext::AddTestFixture(TPtr<ITestFixture> pFixture)
	{
		if (!pFixture)
		{
			return RC_INVALID_ARGS;
		}

		mTestFixtures.push_back(pFixture);

		return RC_OK;
	}

	void CTestContext::AddTestResult(const std::string& testFixtureName, const std::string& testCaseName, const TTestResultEntity& result)
	{
		++mTotalTestsCount;

		mTestResults[testFixtureName].insert({ testCaseName, result });

		if (result.mHasPassed)
		{
			++mPassedTestsCount;
		}
		else
		{
			++mFailedTestsCount;
		}
	}

	E_RESULT_CODE CTestContext::WriteTestResults(TPtr<ITestResultsReporter> pReporter)
	{
		if (!pReporter)
		{
			return RC_INVALID_ARGS;
		}

		pReporter->WriteTestsStatistics({ mPassedTestsCount, mFailedTestsCount });

		for (auto&& currTestFixtureEntry : mTestResults)
		{
			pReporter->EnterTestFixtureSection(currTestFixtureEntry.first);

			for (auto&& currTestResult : currTestFixtureEntry.second)
			{
				pReporter->WriteTestResult(currTestResult.first, currTestResult.second);
			}

			pReporter->ExitTestFixtureSection();
		}
		
		return RC_OK;
	}

	E_RESULT_CODE CTestContext::Assert(const std::string& message, bool actual, bool expected, const std::string& filename, U32 line)
	{
		if (actual != expected)
		{
			throw CAssertException(message, filename, line);
		}

		return RC_OK;
	}

	E_RESULT_CODE CTestContext::RunAllTests()
	{
		if (mIsRunning)
		{
			return RC_FAIL;
		}

		mIsRunning = true;

		mTotalTestsCount = 0;
		mPassedTestsCount = 0;
		mFailedTestsCount = 0;

		return RC_OK;
	}

	void CTestContext::Update(F32 dt)
	{
		if (!mIsRunning)
		{
			return;
		}

		TPtr<ITestFixture> pCurrFixture = mTestFixtures.front();

		if (!pCurrFixture->IsFinished())
		{
			pCurrFixture->Update(dt);
			return;
		}

		mTestFixtures.erase(mTestFixtures.cbegin());

		mIsRunning = !mTestFixtures.empty();
	}

	void CTestContext::SetMousePosition(const TVector3& position)
	{
		pProxyInputContextDesc->mMousePosition = position;
	}

	void CTestContext::NotifyOnKeyPressEvent(E_KEYCODES keyCode)
	{
		pProxyInputContextDesc->mFrameKeysInputBuffer.insert(keyCode);
	}
	
	void CTestContext::NotifyOnMouseButtonPressEvent(U8 buttonId)
	{
		if (buttonId >= pProxyInputContextDesc->mMouseButtonsCount)
		{
			TDE2_ASSERT(false);
			return;
		}

		pProxyInputContextDesc->mFrameMouseButtonsInputBuffer.insert(buttonId);
	}

	E_RESULT_CODE CTestContext::TakeScreenshot(const std::string& filename)
	{
		auto pFileSystem = mpEngineCore->GetSubsystem<IFileSystem>();

		auto screenshotFileWriterResult = pFileSystem->Open<IImageFileWriter>(filename, true);
		if (screenshotFileWriterResult.HasError())
		{
			return screenshotFileWriterResult.GetError();
		}

		if (IImageFileWriter* pImageWriter = pFileSystem->Get<IImageFileWriter>(screenshotFileWriterResult.Get()))
		{
			auto pGraphicsContext = mpEngineCore->GetSubsystem<IGraphicsContext>();
			auto pWindowSystem = mpEngineCore->GetSubsystem<IWindowSystem>();

			E_RESULT_CODE result = pImageWriter->Write(pWindowSystem->GetWidth(), pWindowSystem->GetHeight(), 4, pGraphicsContext->GetBackBufferData());
			result = result | pImageWriter->Close();

			return result;
		}

		return RC_OK;
	}

	E_RESULT_CODE CTestContext::TakeScreenshot(const std::string& testFixture, const std::string& testCase)
	{
		auto pFileSystem = mpEngineCore->GetSubsystem<IFileSystem>();
		if (!pFileSystem)
		{
			return RC_FAIL;
		}

		static const std::string filenamePattern { "{0}_{1}{2}{3}.png" };

		/// \note Extract current time
		const auto& timePoint = std::chrono::system_clock::now();
		const std::time_t currTime = std::chrono::system_clock::to_time_t(timePoint);
		const auto& localTime = std::localtime(&currTime);
		
		return TakeScreenshot(
			pFileSystem->CombinePath(mArtifactsOutputDirectoryPath, 
			pFileSystem->CombinePath(testFixture, Wrench::StringUtils::Format(filenamePattern, testCase, localTime->tm_hour, localTime->tm_min, localTime->tm_sec))));
	}

	constexpr I32 PerceptualHashImageSize = 8;


	CTestContext::TImagePerceptualHash CTestContext::ComputePerceptualHashForCurrentFrame()
	{
		auto pGraphicsContext = mpEngineCore->GetSubsystem<IGraphicsContext>();
		auto pWindowSystem = mpEngineCore->GetSubsystem<IWindowSystem>();
		
		auto&& frameBufferPixelData = pGraphicsContext->GetBackBufferData();

		std::vector<U8> outputBufferData;
		outputBufferData.resize(PerceptualHashImageSize * PerceptualHashImageSize * sizeof(U32));

		// Downscale image to 8x8
		stbir_resize_uint8(frameBufferPixelData.data(), pWindowSystem->GetWidth(), pWindowSystem->GetHeight(), pWindowSystem->GetWidth() * sizeof(U32),
			outputBufferData.data(), PerceptualHashImageSize, PerceptualHashImageSize, PerceptualHashImageSize * sizeof(U32), 4);

		// Convert to grayscale 
		std::vector<U8> grayscaleImageData;

		for (USIZE i = 0; i < outputBufferData.size(); i += sizeof(U32)) // don't account alpha channel
		{
			const U32 value = outputBufferData[i] / 3 + outputBufferData[i + 1] / 2 + outputBufferData[i + 2] / 10;
			grayscaleImageData.push_back(value < 255 ? value : 255);
		}

		// Find average for all pixels
		const U8 averagePixelValue = std::accumulate(grayscaleImageData.cbegin(), grayscaleImageData.cend(), 0) / static_cast<U8>(grayscaleImageData.size());

		// Form 64 bits number where a bit's set up in 1 if current pixel's value is greater than the average and 0 otherwise
		TImagePerceptualHash hash;

		for (USIZE i = 0; i < grayscaleImageData.size(); i++)
		{
			hash[i] = grayscaleImageData[i] > averagePixelValue ? 1 : 0;
		}

		return hash;
	}

	bool CTestContext::IsCurrentFrameHasSamePerceptualHash(U64 expectedHash, U8 hammingDistanceThreshold)
	{
		const TImagePerceptualHash currentHashBits = ComputePerceptualHashForCurrentFrame();
		const TImagePerceptualHash expectedHashBits = expectedHash;

		return (currentHashBits ^ expectedHashBits).count() < hammingDistanceThreshold;
	}

	TColor32F CTestContext::GetFrameBufferPixel(U32 x, U32 y) const
	{
		auto pGraphicsContext = mpEngineCore->GetSubsystem<IGraphicsContext>();
		auto pWindowSystem = mpEngineCore->GetSubsystem<IWindowSystem>();

		if (x >= pWindowSystem->GetWidth() || y >= pWindowSystem->GetHeight())
		{
			return TColorUtils::mBlack;
		}

		auto&& frameBufferPixelData = pGraphicsContext->GetBackBufferData();

		const USIZE index = (pWindowSystem->GetHeight() - y) * (pWindowSystem->GetWidth() * sizeof(U32)) + x * sizeof(U32);
		TDE2_ASSERT(index + 3 < frameBufferPixelData.size());

		if (index + 3 >= frameBufferPixelData.size())
		{
			return TColorUtils::mBlack;
		}

		return TColor32F(frameBufferPixelData[index] / 255.0f, frameBufferPixelData[index + 1] / 255.0f, frameBufferPixelData[index + 2] / 255.0f, 1.0f);
	}

	bool CTestContext::IsFinished() const
	{
		return !mIsRunning && mTestFixtures.empty();
	}

	E_RESULT_CODE CTestContext::SetArtifactsOutputDirectory(const std::string& path)
	{
		if (auto pFileSystem = mpEngineCore->GetSubsystem<IFileSystem>())
		{
			if (!pFileSystem->IsPathValid(path))
			{
				return RC_INVALID_ARGS;
			}

			mArtifactsOutputDirectoryPath = path;

			return RC_OK;
		}

		return RC_FAIL;
	}

	const std::string& CTestContext::GetArtifactsOutputDirectory() const
	{
		return mArtifactsOutputDirectoryPath;
	}

	TPtr<CTestContext> CTestContext::Get()
	{
		static TPtr<CTestContext> pInstance = TPtr<CTestContext>(new CTestContext);
		return pInstance;
	}

	IEngineCore* CTestContext::GetEngineCore() const
	{
		return mpEngineCore;
	}
}

#endif