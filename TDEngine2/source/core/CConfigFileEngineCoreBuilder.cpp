#include "../../include/core/CConfigFileEngineCoreBuilder.h"
#include "../../include/core/IFileSystem.h"
#include "../../include/platform/CConfigFileReader.h"
#include "../../include/metadata.h"
#include <memory>
#include <thread>
#include <functional>


namespace TDEngine2
{
	CConfigFileEngineCoreBuilder::CConfigFileEngineCoreBuilder() :
		CBaseEngineCoreBuilder()
	{
	}

	E_RESULT_CODE CConfigFileEngineCoreBuilder::Init(TCreateEngineCoreCallback pEngineCoreFactoryCallback, const std::string& projectConfigFilename)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		if (!pEngineCoreFactoryCallback || projectConfigFilename.empty())
		{
			return RC_INVALID_ARGS;
		}
		
		E_RESULT_CODE result = RC_OK;

		mProjectConfigFilename = projectConfigFilename;

		mpEngineCoreInstance = pEngineCoreFactoryCallback(result);

		if (result != RC_OK)
		{
			return result;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	static TResult<TEngineSettings> ReadProjectConfigFile(IFileSystem* pFileSystem, const std::string& configFilename)
	{
		TFileEntryId configFileHandle = pFileSystem->Open<IConfigFileReader>(configFilename, false).Get();

		IConfigFileReader* pConfigFileReader = pFileSystem->Get<IConfigFileReader>(configFileHandle);

		TEngineSettings settings;

		settings.mApplicationName       = pConfigFileReader->GetString("main", "name", "Default App");
		settings.mWindowWidth           = pConfigFileReader->GetInt("main", "width", 640);
		settings.mWindowHeight          = pConfigFileReader->GetInt("main", "height", 480);
		settings.mFlags                 = pConfigFileReader->GetInt("main", "flags", 0x0);
		settings.mMaxNumOfWorkerThreads = pConfigFileReader->GetInt("main", "max-num-worker-threads", std::thread::hardware_concurrency() - 1);

		settings.mTotalPreallocatedMemorySize = pConfigFileReader->GetInt("memory", "total-preallocated-memory-size", DefaultGlobalMemoryBlockSize);
		settings.mGraphicsContextType         = Meta::EnumTrait<E_GRAPHICS_CONTEXT_GAPI_TYPE>::FromString(pConfigFileReader->GetString("graphics", "context-type", "unknown"));
		settings.mAudioContextType            = E_AUDIO_CONTEXT_API_TYPE::FMOD;

		pConfigFileReader->Close();

		return Wrench::TOkValue<TEngineSettings>(settings);
	}

	TEngineSettings CConfigFileEngineCoreBuilder::_initEngineSettings()
	{
		if (auto readConfigResult = ReadProjectConfigFile(mpFileSystemInstance, mProjectConfigFilename))
		{
			return (mEngineSettings = readConfigResult.Get());
		}

		TDE2_UNREACHABLE();

		return mEngineSettings;
	}


	TDE2_API IEngineCoreBuilder* CreateConfigFileEngineCoreBuilder(TCreateEngineCoreCallback pEngineCoreFactoryCallback, const std::string& projectConfigFilename, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IEngineCoreBuilder, CConfigFileEngineCoreBuilder, result, pEngineCoreFactoryCallback, projectConfigFilename);
	}
}