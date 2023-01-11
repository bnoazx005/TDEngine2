#include <catch2/catch_test_macros.hpp>
#include <TDEngine2.h>
#include <stringUtils.hpp>
#include <vector>
#include <string>
#include <tuple>


using namespace TDEngine2;


TEST_CASE("File System Tests")
{
	E_RESULT_CODE result = RC_OK;

	TPtr<IFileSystem> pFileSystem = TPtr<IFileSystem>
		(
#if defined (TDE2_USE_WINPLATFORM)
		CreateWin32FileSystem(result)
#elif defined (TDE2_USE_UNIXPLATFORM)
		CreateUnixFileSystem(result)
#else
#endif
		);

	REQUIRE(pFileSystem);
	REQUIRE(result == RC_OK);

	REQUIRE(RC_OK == pFileSystem->RegisterFileFactory<IBinaryFileReader>({ CreateBinaryFileReader, E_FILE_FACTORY_TYPE::READER }));
	REQUIRE(RC_OK == pFileSystem->RegisterFileFactory<IBinaryFileWriter>({ CreateBinaryFileWriter, E_FILE_FACTORY_TYPE::WRITER }));

	SECTION("TestMountPhysicalPath_PassRealPhysicalPathsAndVirtualOnes_CorrectlyResolvesPaths")
	{
		REQUIRE(RC_OK == pFileSystem->MountPhysicalPath("./Resources/", "Resources/"));
		REQUIRE(RC_OK == pFileSystem->MountPhysicalPath("./tmp/", "/tmp/"));
	}

#if 0
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
}


TEST_CASE("CYAMLFileOperations Tests")
{
	E_RESULT_CODE result = RC_OK;

	auto pMemoryMappedStream = TPtr<TDEngine2::IStream>(CreateMemoryIOStream(Wrench::StringUtils::GetEmptyStr(), {}, result));
	REQUIRE(pMemoryMappedStream);

	SECTION("TestWriteReadYAML_WriteSomeYAMLIntoMemoryRegionThenTryToReadBack_AllTheDataShouldBeCorrectlySerializedDeserialized")
	{
		struct TObject
		{
			I32 mValue;
			I32 mArray[2];
			std::string mStr;
		} expectedObject{ 42, { 0, 1}, "Hello, World!" };

		/// \note Try to write
		{
			IYAMLFileWriter* pYAMLFileWriter = dynamic_cast<IYAMLFileWriter*>(CreateYAMLFileWriter(nullptr, pMemoryMappedStream, result));
			REQUIRE(pYAMLFileWriter);

			/*!
				YAML file will be something like that

				test: 
					value: 42
					array:
						- element: 0
						- element: 1
					str: "Hello, World!"
			*/

			pYAMLFileWriter->BeginGroup("test");
			{
				pYAMLFileWriter->SetInt32("value", expectedObject.mValue);
				{
					pYAMLFileWriter->BeginGroup("array", true);
					{
						for (USIZE i = 0; i < sizeof(expectedObject.mArray) / sizeof(expectedObject.mArray[0]); i++)
						{
							pYAMLFileWriter->BeginGroup(Wrench::StringUtils::GetEmptyStr(), false); /// \note Each array's element should be wrapped with empty string group
							{
								pYAMLFileWriter->SetInt32("element", expectedObject.mArray[i]);
							}
							pYAMLFileWriter->EndGroup();
						}
					}
					pYAMLFileWriter->EndGroup();
				}
				pYAMLFileWriter->SetString("str", expectedObject.mStr);
			}
			pYAMLFileWriter->EndGroup();

			REQUIRE(RC_OK == pYAMLFileWriter->Close());
		}

		TObject actualObject;

		/// \note Try to read
		{
			IYAMLFileReader* pYAMLFileReader = dynamic_cast<IYAMLFileReader*>(CreateYAMLFileReader(nullptr, pMemoryMappedStream, result));
			REQUIRE(pYAMLFileReader);

			pYAMLFileReader->BeginGroup("test");
			{
				actualObject.mValue = pYAMLFileReader->GetInt32("value");
				{
					pYAMLFileReader->BeginGroup("array");
					{
						USIZE index = 0;
						while (pYAMLFileReader->HasNextItem())
						{
							pYAMLFileReader->BeginGroup(Wrench::StringUtils::GetEmptyStr()); 
							{
								REQUIRE(index < (sizeof(actualObject.mArray) / sizeof(actualObject.mArray[0])));
								actualObject.mArray[index++] = pYAMLFileReader->GetInt32("element");
							}
							pYAMLFileReader->EndGroup();
						}
					}
					pYAMLFileReader->EndGroup();
				}
				actualObject.mStr = pYAMLFileReader->GetString("str");
			}
			pYAMLFileReader->EndGroup();

			REQUIRE(RC_OK == pYAMLFileReader->Close());
		}

		REQUIRE(expectedObject.mValue == actualObject.mValue);

		for (USIZE i = 0; i < sizeof(expectedObject.mArray) / sizeof(expectedObject.mArray[0]); i++)
		{
			REQUIRE(expectedObject.mArray[i] == actualObject.mArray[i]);
		}

		REQUIRE(expectedObject.mStr == actualObject.mStr);
	}
}