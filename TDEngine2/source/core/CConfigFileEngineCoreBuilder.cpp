#include "../../include/core/CConfigFileEngineCoreBuilder.h"
#include "../../include/core/IFileSystem.h"
#include "../../include/core/CGameUserSettings.h"
#include "../../include/platform/CConfigFileReader.h"
#include "../../include/core/CGameUserSettings.h"
#include "../../include/core/CProjectSettings.h"
#include "../../include/utils/CFileLogger.h"
#include "../../include/utils/CProgramOptions.h"
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

		/// \note Load project settings in the following order 
		/// - own project config (text/binary) / 
		/// - any project config if exists in app's directory (text/binary) 
		std::array<std::function<E_RESULT_CODE(const std::string&)>, 4> configLoaders
		{
			[this](auto&& projectConfigPath) 
			{ 
				return TryLoadProjectSettingsAs<IYAMLFileReader>(mpFileSystemInstance, mpFileSystemInstance->Open<IYAMLFileReader>(projectConfigPath, false));
			},
			[this](auto&& projectConfigPath)
			{
				///\note May be we work with binarized resources. In this case the project settings' file is binarized too. Try to load it thourgh IBinaryArchiveReader
				E_RESULT_CODE result = TryLoadProjectSettingsAs<IBinaryArchiveReader>(
					mpFileSystemInstance, 
					mpFileSystemInstance->Open<IBinaryArchiveReader>(projectConfigPath, false));

				if (RC_OK != result)
				{
					return result;
				}

				CProjectSettings::Get()->mCommonSettings.mBinaryResourcesActive = true;

				return result;
			},
			[&](auto&& projectConfigPath) /// Try either to load project's config specified at --project-config CLI option or load first one that exists at app's directory
			{
				E_RESULT_CODE result = RC_OK;

				auto&& pathResult = CProgramOptions::Get()->GetValue<std::string>("project-config"); /// \todo Replace with named constant
				if (pathResult.IsOk())
				{
					for (U32 i = 0; i < 2; i++)
					{
						result = configLoaders[i](pathResult.Get());
						if (RC_OK == result)
						{
							return result;
						}
					}
				}

				for (auto&& currConfigFilePath : mpFileSystemInstance->GetFilesListAtDirectory("."))
				{
					if (!Wrench::StringUtils::EndsWith(currConfigFilePath, ".project"))
					{
						continue;
					}

					for (U32 i = 0; i < 2; i++)
					{
						result = configLoaders[i](currConfigFilePath);
						if (RC_OK == result)
						{
							auto&& projectName = mpFileSystemInstance->ExtractFilename(mProjectConfigFilepath);
							CProjectSettings::Get()->mCommonSettings.mApplicationName = projectName.substr(0, projectName.find_first_of('.'));

							return result;
						}
					}
				}

				return RC_FAIL;
			},
			[this](auto&& projectConfigPath)
			{
				auto&& projectName = mpFileSystemInstance->ExtractFilename(projectConfigPath);
				CProjectSettings::Get()->mCommonSettings.mApplicationName = projectName.substr(0, projectName.find_first_of('.'));

				return RC_OK;
			},
		};

		for (auto&& currProjectConfigLoader : configLoaders)
		{
			E_RESULT_CODE result = currProjectConfigLoader(mProjectConfigFilepath);
			if (RC_OK == result)
			{
				break;
			}
		}

		return result;
	}


	TDE2_API IEngineCoreBuilder* CreateConfigFileEngineCoreBuilder(const TConfigEngineCoreBuilderInputParams& params, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IEngineCoreBuilder, CConfigFileEngineCoreBuilder, result, params);
	}
}