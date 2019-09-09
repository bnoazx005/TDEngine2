#include "./../include/CUtilityListener.h"
#define TDE2_YAML_PLUGIN_IMPLEMENTATION
#include "./../plugins/YAMLFormatSupport/include/CYAMLSupportPlugin.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"


using namespace TDEngine2;


TDEngine2::E_RESULT_CODE CUtilityListener::OnStart()
{
	TDEngine2::E_RESULT_CODE result = RC_OK;

	auto pFileSystem = dynamic_cast<TDEngine2::IFileSystem*>(mpEngineCoreInstance->GetSubsystem(TDEngine2::EST_FILE_SYSTEM));

	auto pYamlFile = pFileSystem->Get<CYAMLFileReader>(pFileSystem->Open<CYAMLFileReader>("test.info").Get());

	Yaml::Node root;

	if (pYamlFile->Deserialize(root) != RC_OK)
	{
		return RC_FAIL;
	}

	return RC_OK;
}

TDEngine2::E_RESULT_CODE CUtilityListener::OnUpdate(const float& dt)
{
	mpWindowSystem->SetTitle(std::to_string(dt));

	return RC_OK;
}

TDEngine2::E_RESULT_CODE CUtilityListener::OnFree()
{
	return RC_OK;
}

void CUtilityListener::SetEngineInstance(TDEngine2::IEngineCore* pEngineCore)
{
	if (!pEngineCore)
	{
		return;
	}

	mpEngineCoreInstance = pEngineCore;

	mpGraphicsContext = dynamic_cast<TDEngine2::IGraphicsContext*>(mpEngineCoreInstance->GetSubsystem(TDEngine2::EST_GRAPHICS_CONTEXT));

	mpWindowSystem = dynamic_cast<TDEngine2::IWindowSystem*>(mpEngineCoreInstance->GetSubsystem(TDEngine2::EST_WINDOW));

	mpResourceManager = dynamic_cast<TDEngine2::IResourceManager*>(mpEngineCoreInstance->GetSubsystem(TDEngine2::EST_RESOURCE_MANAGER));
}
