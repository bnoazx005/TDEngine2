#include "resourcePacker.h"
#include "deps/argparse/argparse.h"
#include "../../deps/zlib/zlib.h"
#include <unordered_set>
#include <iostream>
#include <fstream>

#if _HAS_CXX17
	#include <filesystem>
	namespace fs = std::filesystem;
#else
	#include <experimental/filesystem>
	namespace fs = std::experimental::filesystem;
#endif



namespace TDEngine2
{

	constexpr const char* Usage[] =
	{
		"tde2_resource_packer <input> .. <input> [options]",
		"where <input> - single file path or directory",
		0
	};


	Result<TUtilityOptions> ParseOptions(int argc, const char** argv) TDE2_NOEXCEPT
	{
		int showVersion = 0;

		// flags
		int taggedOnly = 0;
		int suppressLogOutput = 0;
		int forceMode = 0;
		int emitFlags = 0;
		int useCompression = 0;

		const char* pOutputDirectory = nullptr;
		const char* pOutputFilename = nullptr;
		const char* pExcludedPathsStr = nullptr;
		const char* pExcludedTypenamesStr = nullptr;

		const char* pCacheOutputDirectory = nullptr;

		struct argparse_option options[] = {
			OPT_HELP(),
			OPT_GROUP("Basic options"),
			OPT_BOOLEAN('V', "version", &showVersion, "Print version info and exit"),
			OPT_STRING(0, "outdir", &pOutputDirectory, "Write output into specified <dirname>"),
			OPT_STRING('o', "outfile", &pOutputFilename, "Output file's name <filename>"),
			OPT_BOOLEAN(0, "quiet", &suppressLogOutput, "Enables suppresion of program's output"),
			OPT_BOOLEAN(0, "compress", &useCompression, "Enables compression of resources"),
			OPT_END(),
		};

		struct argparse argparse;
		argparse_init(&argparse, options, Usage, 0);
		argparse_describe(&argparse, "\nThe utility is an archiver of resources which is a part of TDE2 game engine that gathers them together in single peace (package)", "\n");
		argc = argparse_parse(&argparse, argc, argv);

		if (showVersion)
		{
			std::cout << "tde2_resource_packer, version " << ToolVersion.mMajor << "." << ToolVersion.mMinor << std::endl;
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
			return Wrench::TErrValue<E_ERROR_CODE>(E_ERROR_CODE::FAIL);
		}

		if (pOutputDirectory)
		{
			utilityOptions.mOutputDirname = fs::path(pOutputDirectory).string();
		}

		if (pOutputFilename)
		{
			utilityOptions.mOutputFilename = pOutputFilename;
		}

		utilityOptions.mCompressFiles = static_cast<bool>(useCompression);

		return Wrench::TOkValue<TUtilityOptions>(utilityOptions);
	}


	std::vector<std::string> BuildFilesList(const std::vector<std::string>& directories) TDE2_NOEXCEPT
	{
		if (directories.empty())
		{
			return {};
		}

		std::unordered_set<std::string> processedPaths; // contains absolute paths that already have been processed 

		std::vector<std::string> headersPaths;

		for (auto&& currSource : directories)
		{
			// files
			if (!fs::is_directory(currSource))
			{
				auto&& path = fs::path{ currSource };

				auto&& absPathStr = fs::canonical(currSource).string();

				if (processedPaths.find(absPathStr) == processedPaths.cend())
				{
					headersPaths.emplace_back(currSource);
					processedPaths.emplace(absPathStr);
				}

				continue;
			}

			// directories
			for (auto&& directory : fs::recursive_directory_iterator{ currSource })
			{
				auto&& path = directory.path();

				auto&& absPathStr = fs::canonical(path).string();

				if (processedPaths.find(absPathStr) == processedPaths.cend())
				{
					headersPaths.emplace_back(path.string());
					processedPaths.emplace(absPathStr);
				}
			}
		}

		return headersPaths;
	}


	static E_ERROR_CODE WritePackageHeader(std::ofstream& packageFile, const TPackageFileHeader& headerData) TDE2_NOEXCEPT
	{
		packageFile.write(reinterpret_cast<const char*>(&headerData.mTag), sizeof(headerData.mTag));
		packageFile.write(reinterpret_cast<const char*>(&headerData.mVersion), sizeof(headerData.mVersion));

		const uint16_t padding = 0;

		packageFile.write(reinterpret_cast<const char*>(&padding), sizeof(padding));

		packageFile.write(reinterpret_cast<const char*>(&headerData.mEntitiesCount), sizeof(headerData.mEntitiesCount));
		packageFile.write(reinterpret_cast<const char*>(&headerData.mFilesTableOffset), sizeof(headerData.mFilesTableOffset));
		packageFile.write(reinterpret_cast<const char*>(&headerData.mFilesTableSize), sizeof(headerData.mFilesTableSize));

		return E_ERROR_CODE::OK;
	}


	static E_ERROR_CODE WriteFilesTableInfo(std::ofstream& packageFile, std::vector<TPackageFileEntryInfo>&& filesTable) TDE2_NOEXCEPT
	{
		for (auto&& currFileEntry : filesTable)
		{
			uint64_t filenameLength = currFileEntry.mFilename.length();
			packageFile.write(reinterpret_cast<char*>(&filenameLength), sizeof(uint64_t));
			packageFile.write(currFileEntry.mFilename.data(), filenameLength);

			packageFile.write(reinterpret_cast<char*>(&currFileEntry.mDataBlockOffset), sizeof(currFileEntry.mDataBlockOffset));
			packageFile.write(reinterpret_cast<char*>(&currFileEntry.mDataBlockSize), sizeof(currFileEntry.mDataBlockSize));
			packageFile.write(reinterpret_cast<char*>(&currFileEntry.mCompressedBlockSize), sizeof(currFileEntry.mCompressedBlockSize));
			packageFile.write(reinterpret_cast<char*>(&currFileEntry.mIsCompressed), sizeof(currFileEntry.mIsCompressed));
		}

		return E_ERROR_CODE::OK;
	}


	E_ERROR_CODE PackFiles(std::vector<std::string>&& files, const TUtilityOptions& options) TDE2_NOEXCEPT
	{
		if (files.empty())
		{
			return E_ERROR_CODE::INVALID_ARGS;
		}

		if (!fs::exists(options.mOutputDirname))
		{
			fs::create_directory(options.mOutputDirname);
		}

		std::ofstream packageFile{ fs::path(options.mOutputDirname).append(options.mOutputFilename), std::ios::binary };
		if (!packageFile.is_open())
		{
			return E_ERROR_CODE::FAIL;
		}

		// \note Compress files into the package
		std::vector<TPackageFileEntryInfo> filesTable;

		E_ERROR_CODE result = E_ERROR_CODE::OK;

		packageFile.seekp(sizeof(TPackageFileHeader)); // \note Skip writing header, make it as last step

		std::vector<unsigned char> tempDataBuffer;
		std::vector<unsigned char> compressedDataBuffer;

		// \note Copy files' data into the package's file
		for (auto&& currFilePath : files)
		{
			std::ifstream resourceFile{ currFilePath, std::ios::binary | std::ios::ate };
			if (!resourceFile.is_open())
			{
				std::cerr << "Error: Couldn't find file: " << currFilePath << std::endl;
				continue;
			}
			
			std::streampos dataSize = resourceFile.tellg();

			resourceFile.seekg(0);

			tempDataBuffer.resize(static_cast<size_t>(dataSize));
			resourceFile.read(reinterpret_cast<char*>(tempDataBuffer.data()), static_cast<std::streamsize>(dataSize));

			resourceFile.close();

			/// \note Compress data
			uLong compressedDataSize = options.mCompressFiles ? compressBound(static_cast<uLong>(tempDataBuffer.size())) : 0;
			
			if (options.mCompressFiles)
			{
				compressedDataBuffer.resize(compressedDataSize);

				if (Z_OK != compress(&compressedDataBuffer.front(), &compressedDataSize, &tempDataBuffer.front(), static_cast<uLong>(dataSize)))
				{
					continue;
				}
			}

			filesTable.push_back(TPackageFileEntryInfo
				{ 
					currFilePath, 
					static_cast<uint64_t>(packageFile.tellp()), 
					static_cast<uint64_t>(dataSize),
					static_cast<uint64_t>(compressedDataSize),
					options.mCompressFiles
				});

			/// \note Actual write data into the package file
			packageFile.write(reinterpret_cast<char*>(options.mCompressFiles ? compressedDataBuffer.data() : tempDataBuffer.data()), 
				options.mCompressFiles ? static_cast<size_t>(compressedDataSize) : static_cast<size_t>(dataSize));
		}

		TPackageFileHeader header;
		header.mEntitiesCount = static_cast<uint32_t>(filesTable.size());
		header.mFilesTableOffset = packageFile.tellp();

		// \note Write down files table to end of the file
		if (E_ERROR_CODE::OK != (result = WriteFilesTableInfo(packageFile, std::move(filesTable))))
		{
			packageFile.close();
			return result;
		}

		header.mFilesTableSize = static_cast<uint64_t>(packageFile.tellp()) - header.mFilesTableOffset;
		
		packageFile.seekp(0);

		// \note Write header's data
		if (E_ERROR_CODE::OK != (result = WritePackageHeader(packageFile, header)))
		{
			packageFile.close();
			return result;
		}

		packageFile.close();

		return E_ERROR_CODE::OK;
	}
}