/*!
	\file EntryPoint.h
	\date 30.07.2022
	\author Kasimov Ildar
*/

#pragma once


#include "../utils/Types.h"
#include "../utils/CProgramOptions.h"
#include "IEngineCoreBuilder.h"
#include "IEngineListener.h"
#include "CConfigFileEngineCoreBuilder.h"
#include "CEngineCore.h"
#include <string>
#include <memory>


#ifdef TDE2_DEFINE_ENTRY_POINT

#if defined (TDE2_USE_WINPLATFORM)
	#pragma comment(lib, "TDEngine2.lib")
#endif


	extern std::string GetProjectSettingsFilePath();

	extern std::unique_ptr<TDEngine2::IEngineListener> GetEngineListener();

	extern TDEngine2::E_RESULT_CODE ParseOptions(int argc, const char** argv);


	int main(int argc, const char** argv)
	{
		TDEngine2::E_RESULT_CODE result = ParseOptions(argc, argv);
		if (TDEngine2::RC_OK != result)
		{
			return -1;
		}

		auto&& pEngineCoreBuilder = TDEngine2::TPtr<TDEngine2::IEngineCoreBuilder>(TDEngine2::CreateConfigFileEngineCoreBuilder({ TDEngine2::CreateEngineCore, GetProjectSettingsFilePath() }, result));

		if (result != TDEngine2::RC_OK)
		{
			return -1;
		}

		TDEngine2::TPtr<TDEngine2::IEngineCore> pEngineCore = TDEngine2::TPtr<TDEngine2::IEngineCore>(pEngineCoreBuilder->GetEngineCore());

		pEngineCore->RegisterListener(GetEngineListener());
		pEngineCore->Run();

		return 0;
	}

#endif