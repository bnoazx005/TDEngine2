#include "../../include/editor/CResourcesBuildManifest.h"


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	CResourcesBuildManifest::CResourcesBuildManifest() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CResourcesBuildManifest::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CResourcesBuildManifest::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_INVALID_ARGS;
		}

		return RC_OK;
	}

	E_RESULT_CODE CResourcesBuildManifest::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_INVALID_ARGS;
		}

		pWriter->BeginGroup("meta");
		{
			pWriter->SetString("resource_type", "resources_build_manifest");
			pWriter->SetUInt16("version_tag", mVersionTag);
		}
		pWriter->EndGroup();

		return RC_OK;
	}

	E_RESULT_CODE CResourcesBuildManifest::AddResourceBuildInfo(std::unique_ptr<TResourceBuildInfo> pResourceInfo)
	{
		if (!pResourceInfo)
		{
			return RC_INVALID_ARGS;
		}

		mpResourcesBuildConfigs.push_back(std::move(pResourceInfo));

		return RC_OK;
	}


	TDE2_API CResourcesBuildManifest* CreateResourcesBuildManifest(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(CResourcesBuildManifest, CResourcesBuildManifest, result);
	}
}

#endif