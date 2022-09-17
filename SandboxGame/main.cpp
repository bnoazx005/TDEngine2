#define TDE2_DEFINE_ENTRY_POINT
#include <TDEngine2.h>
#include "include/CCustomEngineListener.h"


extern std::string GetProjectSettingsFilePath() { return "SandboxGame.project"; }

extern std::unique_ptr<TDEngine2::IEngineListener> GetEngineListener() { return std::make_unique<CCustomEngineListener>(); }

extern TDEngine2::E_RESULT_CODE ParseOptions(int argc, const char** argv)
{
	return TDEngine2::RC_OK;
}
