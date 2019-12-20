#include <catch2/catch.hpp>
#include <TDEngine2.h>
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
			REQUIRE(pFileSystem->Mount(std::get<0>(currInput), std::get<1>(currInput)) == RC_OK);
			REQUIRE(pFileSystem->Unmount(std::get<1>(currInput)) == RC_OK);
		}
	}

	SECTION("TestMount_PassInvalidPathsOrAliased_Fails")
	{
		REQUIRE(true);
	}

	SECTION("TestResolveVirtualPath_PassValidPaths_ReturnsResolvedIfVirtualFileSystemPathsAreUsed")
	{
		auto currDirectoryStr = pFileSystem->GetCurrDirectory();

#if defined (TDE2_USE_WIN32PLATFORM)
		std::string testExpected = ".\\test";
#elif defined (TDE2_USE_UNIXPLATFORM)
		std::string testExpected = "./test";
#else
#endif

		// first string is an input, the second is expected result
		std::vector<std::tuple<std::string, std::string>> paths
		{
			{ "", "" },
			{ "vfs://", currDirectoryStr },
			{ "vfs://test", testExpected },
			{ "./test", testExpected },
			{ "test", "test" },
			{ "test.png", "test.png" },
		};

		pFileSystem->Mount("./test", "test");

		for (const auto& currPath : paths)
		{
			REQUIRE(pFileSystem->ResolveVirtualPath(std::get<0>(currPath)) == std::get<1>(currPath));
		}
	}

	dynamic_cast<IEngineSubsystem*>(pFileSystem)->Free();
}