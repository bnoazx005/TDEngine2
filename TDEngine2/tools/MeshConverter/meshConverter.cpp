#include "meshConverter.h"
#include "deps/argparse/argparse.h"
#include <experimental/filesystem>
#include <unordered_set>
#include <iostream>
#include <fstream>


namespace fs = std::experimental::filesystem;

namespace TDEngine2
{

	constexpr const char* Usage[] =
	{
		"tde2_mesh_converter <input> .. <input> [options]",
		"where <input> - single file path or directory",
		0
	};


	TResult<TUtilityOptions> ParseOptions(int argc, const char** argv) TDE2_NOEXCEPT
	{
		int showVersion = 0;

		// flags
		int suppressLogOutput = 0;
		int debugMode = 0;
		int skipNormals = 0;
		int skipTangents = 0;
		int skipJoints = 0;

		const char* pOutputDirectory = nullptr;
		const char* pOutputFilename = nullptr;

		struct argparse_option options[] = {
			OPT_HELP(),
			OPT_GROUP("Options"),
			OPT_BOOLEAN('V', "version", &showVersion, "Print version info and exit"),
			OPT_STRING(0, "outdir", &pOutputDirectory, "Write output into specified <dirname>"),
			OPT_STRING('o', "output", &pOutputFilename, "Output file's name <filename>"),
			OPT_BOOLEAN(0, "quiet", &suppressLogOutput, "Enables suppresion of program's output"),
			OPT_BOOLEAN('D', "debug", &debugMode, "Enables debug output of the utility"),
			OPT_BOOLEAN(0, "skip_normals", &skipNormals, "If defined object\'s normals will be skipped"),
			OPT_BOOLEAN(0, "skip_tangents", &skipTangents, "If defined object\'s tangents will be skipped"),
			OPT_BOOLEAN(0, "skip_joints", &skipJoints, "If defined object\'s joints information will be skipped"),
			OPT_END(),
		};

		struct argparse argparse;
		argparse_init(&argparse, options, Usage, 0);
		argparse_describe(&argparse, "\nThe utility is an mesh converter tool that processes FBX, OBJ files into internal engine format", "\n");
		argc = argparse_parse(&argparse, argc, argv);

		if (showVersion)
		{
			std::cout << "tde2_mesh_converter, version " << ToolVersion.mMajor << "." << ToolVersion.mMinor << std::endl;
			exit(0);
		}

		TUtilityOptions utilityOptions;

		// \note parse input files before any option, because argparse library will remove all argv's values after it processes that
		if (argc >= 1)
		{
			auto& sources = utilityOptions.mInputFiles;
			sources.clear();

			for (int i = 0; i < argc; ++i)
			{
				sources.push_back(argparse.out[i]);
			}
		}

		if (utilityOptions.mInputFiles.empty())
		{
			std::cerr << "Error: no input found\n";
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		if (pOutputDirectory)
		{
			utilityOptions.mOutputDirname = fs::path(pOutputDirectory).string();
		}

		if (pOutputFilename)
		{
			utilityOptions.mOutputFilename = pOutputFilename;
		}

		utilityOptions.mShouldSkipNormals  = static_cast<bool>(skipNormals);
		utilityOptions.mShouldSkipTangents = static_cast<bool>(skipTangents);
		utilityOptions.mShouldSkipJoints   = static_cast<bool>(skipJoints);

		return Wrench::TOkValue<TUtilityOptions>(utilityOptions);
	}


	std::vector<std::string> BuildFilesList(const std::vector<std::string>& directories) TDE2_NOEXCEPT
	{
		if (directories.empty())
		{
			return {};
		}

		static const std::array<std::string, 4> extensions{ ".obj", ".OBJ", ".fbx", ".FBX" };

		auto&& hasValidExtension = [=](const std::string& ext)
		{
			for (auto&& currExtension : extensions)
			{
				if (currExtension == ext)
				{
					return true;
				}
			}

			return false;
		};

		std::unordered_set<std::string> processedPaths; // contains absolute paths that already have been processed 

		std::vector<std::string> filesPaths;

		for (auto&& currSource : directories)
		{
			// files
			if (!fs::is_directory(currSource))
			{
				auto&& path = fs::path{ currSource };

				auto&& absPathStr = fs::canonical(currSource).string();

				if (processedPaths.find(absPathStr) == processedPaths.cend() && hasValidExtension(path.extension().string()))
				{
					filesPaths.emplace_back(currSource);
					processedPaths.emplace(absPathStr);
				}

				continue;
			}

			// directories
			for (auto&& directory : fs::recursive_directory_iterator{ currSource })
			{
				auto&& path = directory.path();

				auto&& absPathStr = fs::canonical(path).string();

				if (processedPaths.find(absPathStr) == processedPaths.cend() && hasValidExtension(path.extension().string()))
				{
					filesPaths.emplace_back(path.string());
					processedPaths.emplace(absPathStr);
				}
			}
		}

		return filesPaths;
	}


	static E_RESULT_CODE WritePackageHeader(std::ofstream& packageFile, const TPackageFileHeader& headerData) TDE2_NOEXCEPT
	{
		packageFile.write(reinterpret_cast<const char*>(&headerData), sizeof(headerData));

		return RC_OK;
	}


	static E_RESULT_CODE WriteFilesTableInfo(std::ofstream& packageFile, std::vector<TPackageFileEntryInfo>&& filesTable) TDE2_NOEXCEPT
	{
		for (auto&& currFileEntry : filesTable)
		{
			uint64_t filenameLength = currFileEntry.mFilename.length();
			packageFile.write(reinterpret_cast<char*>(&filenameLength), sizeof(uint64_t));
			packageFile.write(currFileEntry.mFilename.data(), filenameLength);

			packageFile.write(reinterpret_cast<char*>(&currFileEntry.mDataBlockOffset), sizeof(currFileEntry.mDataBlockOffset));
			packageFile.write(reinterpret_cast<char*>(&currFileEntry.mDataBlockSize), sizeof(currFileEntry.mDataBlockSize));
		}

		return RC_OK;
	}


	static E_RESULT_CODE ProcessSingleMeshFile(const std::string& filePath, const TUtilityOptions& options) TDE2_NOEXCEPT
	{
		return RC_OK;
	}


	E_RESULT_CODE ProcessMeshFiles(std::vector<std::string>&& files, const TUtilityOptions& options) TDE2_NOEXCEPT
	{
		if (files.empty())
		{
			return RC_INVALID_ARGS;
		}

		if (!fs::exists(options.mOutputDirname))
		{
			fs::create_directory(options.mOutputDirname);
		}

		E_RESULT_CODE result = RC_OK;

		for (auto&& currFilePath : files)
		{
			result = result | ProcessSingleMeshFile(currFilePath, options);
		}

		return result;
	}
}