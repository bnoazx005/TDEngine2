#include "../../include/core/CConfigFileEngineCoreBuilder.h"
#include "../../include/core/IFileSystem.h"
#include "../../include/core/CGameUserSettings.h"
#include "../../include/platform/CConfigFileReader.h"
#include "../../include/core/CGameUserSettings.h"
#include "../../include/core/CProjectSettings.h"
#include "../../include/utils/CFileLogger.h"
#include "stringUtils.hpp"
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


	template <typename TReader>
	E_RESULT_CODE TryLoadProjectSettingsAs(TPtr<IFileSystem> pFileSystem, const TResult<TFileEntryId>& fileReadResult)
	{
		E_RESULT_CODE result = RC_OK;

		if (fileReadResult.IsOk())
		{
			/// \note Read project's settings
			if (IArchiveReader* pProjectSettingsReader = pFileSystem->Get<TReader>(fileReadResult.Get()))
			{
				if (RC_OK != (result = CProjectSettings::Get()->Init(pProjectSettingsReader)))
				{
					return result;
				}
			}

			return result;
		}

		return fileReadResult.GetError();
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
		if (loadProjectSettingsResult.HasError())
		{
			///\note May be we work with binarized resources. In this case the project settings' file is binarized too. Try to load it thourgh IBinaryArchiveReader
			if (RC_OK != (result = TryLoadProjectSettingsAs<IBinaryArchiveReader>(mpFileSystemInstance, mpFileSystemInstance->Open<IBinaryArchiveReader>(mProjectConfigFilepath, false))))
			{
				LOG_WARNING(Wrench::StringUtils::Format("[CConfigFileEngineCoreBuilder] The project config file {0} wasn't found, use default settings instead...", mProjectConfigFilepath));
				return result;
			}

			CProjectSettings::Get()->mCommonSettings.mBinaryResourcesActive = true;

			return result;
		}

		if (RC_OK != (result = TryLoadProjectSettingsAs<IYAMLFileReader>(mpFileSystemInstance, loadProjectSettingsResult)))
		{
			LOG_WARNING(Wrench::StringUtils::Format("[CConfigFileEngineCoreBuilder] The project config file {0} wasn't found, use default settings instead...", mProjectConfigFilepath));
		}

		return result;
	}


	TDE2_API IEngineCoreBuilder* CreateConfigFileEngineCoreBuilder(const TConfigEngineCoreBuilderInputParams& params, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IEngineCoreBuilder, CConfigFileEngineCoreBuilder, result, params);
	}
}