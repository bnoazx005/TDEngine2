#include <catch2/catch.hpp>
#include <TDEngine2.h>
#include <stringUtils.hpp>
#include <vector>
#include <string>
#include <tuple>


using namespace TDEngine2;


TEST_CASE("File System Tests")
{
	E_RESULT_CODE result = RC_OK;

	IFileSystem* pFileSystem = nullptr;

#if defined (TDE2_USE_WIN32PLATFORM)
	pFileSystem = CreateWin32FileSystem(result);
#elif defined (TDE2_USE_UNIXPLATFORM)
	pFileSystem = CreateUnixFileSystem(result);
#else
#endif

	REQUIRE(pFileSystem);
	REQUIRE(result == RC_OK);

	REQUIRE(RC_OK == pFileSystem->RegisterFileFactory<IBinaryFileReader>({ CreateBinaryFileReader, E_FILE_FACTORY_TYPE::READER }));
	REQUIRE(RC_OK == pFileSystem->RegisterFileFactory<IBinaryFileWriter>({ CreateBinaryFileWriter, E_FILE_FACTORY_TYPE::WRITER }));

	SECTION("TestMountPhysicalPath_PassRealPhysicalPathsAndVirtualOnes_CorrectlyResolvesPaths")
	{
		REQUIRE(RC_OK == pFileSystem->MountPhysicalPath("./Resources/", "Resources/"));
		REQUIRE(RC_OK == pFileSystem->MountPhysicalPath("./tmp/", "/tmp/"));
	}

	SECTION("TestOpenFile_PassNativeFiles_CorrectlyOpensItAndReturnsHandleIfTheyExist")
	{
		REQUIRE(RC_OK == pFileSystem->MountPhysicalPath(".", "")); // Mount native FS
		REQUIRE(RC_OK == pFileSystem->MountPhysicalPath("../../", "Sources/")); // Remap native path with alias
		
		auto tryOpenFile = [pFileSystem](const std::string& path)
		{
			if (auto fileResult = pFileSystem->Open<IBinaryFileReader>(path))
			{
				REQUIRE(fileResult.Get() != TFileEntryId::Invalid);
			}
			else
			{
				REQUIRE(false);
			}
		};

		tryOpenFile("TDEngine2.dll");
		tryOpenFile("Sources/main.cpp");

		REQUIRE(pFileSystem->Open<IBinaryFileReader>("test.bin").HasError()); // this one doesn't exist
	}

#if 0
	SECTION("TestMount_PassValidPathsAndAliased_ReturnsOk")
	{
		// first string is a path, the second is an alias
		std::vector<std::tuple<std::string, std::string>> inputs
		{
			{ "./test", "test" }, 
			{ "./test", "/test" },
			{ "./test", "/test/" },
			{ "./test", "/test/test2/" },
			{ "./test", "test/test2/" },
			{ "./test", "test/test2" },
		};

		for (const auto& currInput: inputs)
		{
			REQUIRE(pFileSystem->MountPhysicalPath(std::get<0>(currInput), std::get<1>(currInput)) == RC_OK);
			REQUIRE(pFileSystem->Unmount(std::get<1>(currInput)) == RC_OK);
		}
	}

	SECTION("TestMount_PassInvalidPathsOrAliased_Fails")
	{
		REQUIRE(true);
	}
#endif

	SECTION("TestResolveVirtualPath_PassValidPaths_ReturnsResolvedIfVirtualFileSystemPathsAreUsed")
	{
		auto currDirectoryStr = Wrench::StringUtils::Format("{0}{1}", pFileSystem->GetCurrDirectory(), pFileSystem->GetPathSeparatorChar());

		std::string testPathExpected   = Wrench::StringUtils::Format(".{0}test{0}", pFileSystem->GetPathSeparatorChar());
		std::string shaderPathExpected = Wrench::StringUtils::Format(".{0}Resources{0}Shaders{0}", pFileSystem->GetPathSeparatorChar());

		// first string is an input, the second is expected result, the third is a flag which tells whether it's a directory path or not
		std::vector<std::tuple<std::string, std::string, bool>> paths
		{
			{ "", currDirectoryStr, true },
			{ "test/abcde", testPathExpected + Wrench::StringUtils::Format("abcde{0}", pFileSystem->GetPathSeparatorChar()), true },
			{ "test", testPathExpected, true },
			//{ "./test", testPathExpected, true },
			{ "test", testPathExpected, true },
			{ "test.png", "test.png", false },
			{ "Shaders", shaderPathExpected, true },
			{ "Shaders/", shaderPathExpected, true },
		};

		pFileSystem->MountPhysicalPath("./test", "test");
		pFileSystem->MountPhysicalPath("./Resources/Shaders/", "Shaders");

		for (const auto& currPath : paths)
		{
			REQUIRE(pFileSystem->ResolveVirtualPath(std::get<0>(currPath), std::get<2>(currPath)) == std::get<1>(currPath));
		}
	}

	dynamic_cast<IEngineSubsystem*>(pFileSystem)->Free();
}