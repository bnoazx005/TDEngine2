#include "../../include/core/CConfigFileEngineCoreBuilder.h"
#include "../../include/core/IFileSystem.h"
#include "../../include/platform/CConfigFileReader.h"
#include <memory>
#include <thread>
#include <functional>


namespace TDEngine2
{
	CConfigFileEngineCoreBuilder::CConfigFileEngineCoreBuilder() :
		CBaseEngineCoreBuilder()
	{
	}

	/*!
		\brief The method initialized the builder's object

		\param[in] A callback to a factory's function of IEngineCore's objects

		\return RC_OK if everything went ok, or some other code, which describes an error
	*/

	E_RESULT_CODE CConfigFileEngineCoreBuilder::Init(TCreateEngineCoreCallback pEngineCoreFactoryCallback, const std::string& configFilename)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		if (!pEngineCoreFactoryCallback || configFilename.empty())
		{
			return RC_INVALID_ARGS;
		}
		
		E_RESULT_CODE result = RC_OK;

		mConfigFilename = configFilename;

		mpEngineCoreInstance = pEngineCoreFactoryCallback(result);

		if (result != RC_OK)
		{
			return result;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	TResult<TEngineSettings> CConfigFileEngineCoreBuilder::_readConfigurationFile(IFileSystem* pFileSystem, const std::string& configFilename)
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
		settings.mGraphicsContextType         = StringToGraphicsContextType(pConfigFileReader->GetString("graphics", "context-type", "unknown"));

		pConfigFileReader->Close();

		return Wrench::TOkValue<TEngineSettings>(settings);
	}

	TEngineSettings CConfigFileEngineCoreBuilder::_initEngineSettings()
	{
		if (auto readConfigResult = _readConfigurationFile(mpFileSystemInstance, mConfigFilename))
		{
			return (mEngineSettings = readConfigResult.Get());
		}

		TDE2_ASSERT(false);

		return mEngineSettings;
	}


	TDE2_API IEngineCoreBuilder* CreateConfigFileEngineCoreBuilder(TCreateEngineCoreCallback pEngineCoreFactoryCallback, const std::string& configFilename, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IEngineCoreBuilder, CConfigFileEngineCoreBuilder, result, pEngineCoreFactoryCallback, configFilename);
	}
}