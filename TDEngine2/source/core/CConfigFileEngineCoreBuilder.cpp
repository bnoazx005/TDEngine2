#include "../../include/core/CConfigFileEngineCoreBuilder.h"
#include "../../include/core/IFileSystem.h"
#include "../../include/core/CGameUserSettings.h"
#include "../../include/platform/CConfigFileReader.h"
#include "../../include/metadata.h"
#include "../../include/core/CGameUserSettings.h"
#include "../../include/core/CProjectSettings.h"
#include <memory>
#include <thread>
#include <functional>


namespace TDEngine2
{
	CConfigFileEngineCoreBuilder::CConfigFileEngineCoreBuilder() :
		CBaseEngineCoreBuilder()
	{
	}

	E_RESULT_CODE CConfigFileEngineCoreBuilder::Init(const TConfigEngineCoreBuilderInputParams& params)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		if (!params.mpEngineCoreFactoryCallback || params.mProjectConfigFilepath.empty())
		{
			return RC_INVALID_ARGS;
		}
		
		E_RESULT_CODE result = RC_OK;

		mProjectConfigFilepath = params.mProjectConfigFilepath;
		mGameUserSettingsFilepath = params.mUserConfigFilepath;

		mpEngineCoreInstance = (params.mpEngineCoreFactoryCallback)(result);

		if (result != RC_OK)
		{
			return result;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CConfigFileEngineCoreBuilder::_initEngineSettings()
	{
		E_RESULT_CODE result = RC_OK;

		/// \note Load application settings
		auto loadConfigFileResult = mpFileSystemInstance->Open<IConfigFileReader>(mGameUserSettingsFilepath, false);

		if (loadConfigFileResult.IsOk())
		{
			TFileEntryId configFileHandle = loadConfigFileResult.Get();

			/// \note Read user's settings
			if (IConfigFileReader* pConfigFileReader = mpFileSystemInstance->Get<IConfigFileReader>(configFileHandle))
			{
				if (RC_OK != (result = CGameUserSettings::Get()->Init(pConfigFileReader)))
				{
					return result;
				}

				pConfigFileReader->Close();
			}
		}
		else
		{
			LOG_WARNING(Wrench::StringUtils::Format("[CConfigFileEngineCoreBuilder] The config file {0} wasn't found, use default settings instead...", mGameUserSettingsFilepath));
		}

		/// \note Load project settings
		auto loadProjectSettingsResult = mpFileSystemInstance->Open<IYAMLFileReader>(mProjectConfigFilepath, false);
		
		if (loadProjectSettingsResult.IsOk())
		{
			TFileEntryId projectSettingsFileHandle = loadProjectSettingsResult.Get();

			/// \note Read project's settings
			if (IArchiveReader* pProjectSettingsReader = mpFileSystemInstance->Get<IYAMLFileReader>(projectSettingsFileHandle))
			{
				if (RC_OK != (result = CProjectSettings::Get()->Init(pProjectSettingsReader)))
				{
					return result;
				}
			}
		}
		else
		{
			LOG_WARNING(Wrench::StringUtils::Format("[CConfigFileEngineCoreBuilder] The project config file {0} wasn't found, use default settings instead...", mProjectConfigFilepath));
		}

		return RC_OK;
	}


	TDE2_API IEngineCoreBuilder* CreateConfigFileEngineCoreBuilder(const TConfigEngineCoreBuilderInputParams& params, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IEngineCoreBuilder, CConfigFileEngineCoreBuilder, result, params);
	}
}