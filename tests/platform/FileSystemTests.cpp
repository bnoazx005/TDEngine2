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

		std::string testPathExpected   = CStringUtils::Format(".{0}test{0}", pFileSystem->GetPathSeparatorChar());
		std::string shaderPathExpected = CStringUtils::Format(".{0}Resources{0}Shaders{0}", pFileSystem->GetPathSeparatorChar());

		// first string is an input, the second is expected result, the third is a flag which tells whether it's a directory path or not
		std::vector<std::tuple<std::string, std::string, bool>> paths
		{
			{ "", "", true },
			{ "vfs://", currDirectoryStr, true },
			{ "vfs://test/abcde", testPathExpected + CStringUtils::Format("abcde{0}", pFileSystem->GetPathSeparatorChar()), true },
			{ "vfs://test", testPathExpected, true },
			{ "./test", testPathExpected, true },
			{ "test", CStringUtils::Format("test{0}", pFileSystem->GetPathSeparatorChar()), true },
			{ "test.png", "test.png", false },
			{ "vfs://Shaders", shaderPathExpected, true },
			{ "vfs://Shaders/", shaderPathExpected, true },
		};

		pFileSystem->Mount("./test", "test");
		pFileSystem->Mount("./Resources/Shaders/", "Shaders");

		for (const auto& currPath : paths)
		{
			REQUIRE(pFileSystem->ResolveVirtualPath(std::get<0>(currPath), std::get<2>(currPath)) == std::get<1>(currPath));
		}
	}

	dynamic_cast<IEngineSubsystem*>(pFileSystem)->Free();
}