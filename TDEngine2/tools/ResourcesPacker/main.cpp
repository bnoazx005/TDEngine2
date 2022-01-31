/*!
	\author Ildar Kasimov
	\date 08.11.2020

	\brief The utility is a resources packer that provides functionality for creating
	packages that can be used by TDEngine2. A package has simple structure like the one is used
	in Quake 3 / Unreal 4


	> beginning of a file ===========================

			PackageFileHeader
	----------------------------
			Files Data
	....
	----------------------------
		FilesTableDescription
	Entry1, Entry2, .... EntryN

	< end of the file ===============================

	More information can be found within the engine's sources in CPackageFile.h file
	and at link below
	https://github.com/bnoazx005/TDEngine2/wiki/%5BDraft%5D-Package-file-format-specification

	Beginning from 0.5.x version of the engine the packages support compressed files storage. The compression
	is implemented via zlib library

	\todo Implement endian independent file IO operations
*/

#include <iostream>

#define TDE2_USE_NOEXCEPT 1
#include "resourcePacker.h"


using namespace TDEngine2;


int main(int argc, const char** argv)
{
	Result<TUtilityOptions> parseOptionsResult = ParseOptions(argc, argv);
	if (parseOptionsResult.HasError())
	{
		return static_cast<int>(parseOptionsResult.GetError());
	}

	const TUtilityOptions options = parseOptionsResult.Get();
	
	return static_cast<int>(PackFiles(std::move(BuildFilesList(options.mInputFiles)), options));
}