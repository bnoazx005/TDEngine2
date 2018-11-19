#include "./../../include/utils/CResult.h"
#include "./../../include/utils/CFileLogger.h"
#include <iostream>
#include <cstdlib>


namespace TDEngine2
{
	TDE2_API void Panic(const std::string& message)
	{
		LOG_ERROR(message);

		std::cerr << message << std::endl;

		std::exit(-1);
	}
}