/*!
	\author Ildar Kasimov
	\date 26.06.2021

	\brief The utility is a replacement of deprecated tde2MeshConverter.py version. The output binary format is the same
	the difference is just in importer of models which is implemented via Assimp library instead of own implementation
*/

#include <iostream>

#define TDE2_USE_NOEXCEPT 1
#include "meshConverter.h"


using namespace TDEngine2;


int main(int argc, const char** argv)
{
	TResult<TUtilityOptions> parseOptionsResult = ParseOptions(argc, argv);
	if (parseOptionsResult.HasError())
	{
		return static_cast<int>(parseOptionsResult.GetError());
	}

	const TUtilityOptions options = parseOptionsResult.Get();
	
	return static_cast<int>(ProcessMeshFiles(std::move(BuildFilesList(options.mInputFiles)), options));
}