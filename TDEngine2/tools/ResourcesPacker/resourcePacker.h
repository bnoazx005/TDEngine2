/*!
	\file resourcePacker.h
	\date 08.11.2020
	\author Ildar Kasimov

	\brief 
*/

#pragma once


#include "../../deps/Wrench/source/result.hpp"
#include <vector>
#include <string>


namespace TDEngine2
{
	enum class E_ERROR_CODE : uint32_t
	{
		FAIL = 0x1,
		INVALID_ARGS = 0x2,
		OK = 0x0
	};


	template <typename T> using Result = Wrench::Result<T, E_ERROR_CODE>;


#if TDE2_USE_NOEXCEPT
	#define TDE2_NOEXCEPT noexcept
#else 
	#define TDE2_NOEXCEPT 
#endif


	static struct TVersion
	{
		const uint32_t mMajor = 0;
		const uint32_t mMinor = 1;
	} ToolVersion;


	struct TUtilityOptions
	{
		std::vector<std::string> mInputFiles;

		std::string mOutputDirname = ".";
		std::string mOutputFilename = "NewArchive.pak";

		bool mCompressFiles = false;
	};


#pragma pack(push, 1)

	typedef struct TPackageFileHeader
	{
		const char mTag[4]{ "PAK" };

		const uint16_t mVersion = 0x100;
		const uint16_t mPadding = 0x0;

		uint32_t mEntitiesCount = 0;

		uint64_t mFilesTableOffset = 0;
		uint64_t mFilesTableSize = 0;
	} TPackageFileHeader, *TPackageFileHeaderPtr;

#pragma pack(pop)


	typedef struct TPackageFileEntryInfo
	{
		std::string mFilename;

		uint64_t mDataBlockOffset = 0;
		uint64_t mDataBlockSize = 0;
		uint64_t mCompressedBlockSize = 0;
		bool     mIsCompressed = false;
	} TPackageFileEntryInfo, *TPackageFileEntryInfoPtr;


	Result<TUtilityOptions> ParseOptions(int argc, const char** argv) TDE2_NOEXCEPT;

	std::vector<std::string> BuildFilesList(const std::vector<std::string>& directories) TDE2_NOEXCEPT;

	E_ERROR_CODE PackFiles(std::vector<std::string>&& files, const TUtilityOptions& options) TDE2_NOEXCEPT;

}