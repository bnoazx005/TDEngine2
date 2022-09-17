#define TDE2_DEFINE_ENTRY_POINT
#include <TDEngine2.h>
#include "include/CCustomEngineListener.h"


extern std::string GetProjectSettingsFilePath() { return "TDE2TestsRunner.project"; }

extern std::unique_ptr<TDEngine2::IEngineListener> GetEngineListener() { return std::make_unique<CCustomEngineListener>(); }

extern TDEngine2::E_RESULT_CODE ParseOptions(int argc, const char** argv)
{
	TDEngine2::CProgramOptions::TParseArgsParams optionsParams;
	optionsParams.mArgsCount = argc;
	optionsParams.mpArgsValues = argv;
	
	TDEngine2::CProgramOptions::Get()->AddArgument({ '\0', "autotests-enabled", Wrench::StringUtils::GetEmptyStr(), TDEngine2::CProgramOptions::TArgumentParams::E_VALUE_TYPE::BOOLEAN });

	return TDEngine2::CProgramOptions::Get()->ParseArgs(optionsParams);
}
