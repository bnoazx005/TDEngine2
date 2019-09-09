#include "./../include/CYAMLSupportPlugin.h"
#include "./../include/CYAMLFile.h"
#include <core/IEngineCore.h>
#include <core/IFileSystem.h>



namespace TDEngine2
{
	CYAMLSupportPlugin::CYAMLSupportPlugin():
		mIsInitialized(false), mpFileSystemInstance(nullptr)
	{
	}

	E_RESULT_CODE CYAMLSupportPlugin::Init(IEngineCore* pEngineCore)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pEngineCore)
		{
			return RC_INVALID_ARGS;
		}

		mpEngineCoreInstance = pEngineCore;

		if (!(mpFileSystemInstance = dynamic_cast<IFileSystem*>(mpEngineCoreInstance->GetSubsystem(EST_FILE_SYSTEM))))
		{
			return RC_FAIL;
		}

		// \note add YAML factory method into register of file factories of the engine
		E_RESULT_CODE result = RC_OK;

		if ((result = mpFileSystemInstance->RegisterFileFactory<CYAMLFileWriter>(CreateYAMLFileWriter)) != RC_OK ||
			(result = mpFileSystemInstance->RegisterFileFactory<CYAMLFileReader>(CreateYAMLFileReader)) != RC_OK)
		{
			return result;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CYAMLSupportPlugin::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		if ((result = mpFileSystemInstance->UnregisterFileFactory<CYAMLFileWriter>()) != RC_OK ||
			(result = mpFileSystemInstance->UnregisterFileFactory<CYAMLFileReader>()) != RC_OK)
		{
			return result;
		}

		delete this;

		mIsInitialized = false;

		return RC_OK;
	}

	const TPluginInfo& CYAMLSupportPlugin::GetInfo() const
	{
		return mPluginInfo;
	}
}


extern "C" TDE2_API TDEngine2::IPlugin* TDE2_APIENTRY CreatePlugin(TDEngine2::IEngineCore* pEngineCore, TDEngine2::E_RESULT_CODE& result)
{
	TDEngine2::CYAMLSupportPlugin* pPluginInstance = new (std::nothrow) TDEngine2::CYAMLSupportPlugin();

	if (!pPluginInstance)
	{
		result = TDEngine2::RC_OUT_OF_MEMORY;

		return nullptr;
	}

	result = pPluginInstance->Init(pEngineCore);

	if (result != TDEngine2::RC_OK)
	{
		delete pPluginInstance;

		pPluginInstance = nullptr;
	}

	return dynamic_cast<TDEngine2::IPlugin*>(pPluginInstance);
}