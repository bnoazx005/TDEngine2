#include "shadersCompiler.h"


#if _HAS_CXX17
	#include <filesystem>
	namespace fs = std::filesystem;
#else
	#include <experimental/filesystem>
	namespace fs = std::experimental::filesystem;
#endif


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
}

#endif