#include "../../include/core/CGameUserSettings.h"
#include "../../include/platform/CConfigFileReader.h"


namespace TDEngine2
{
	CGameUserSettings::CGameUserSettings():
		CBaseObject()
	{
	}

	E_RESULT_CODE CGameUserSettings::Init(IConfigFileReader* pConfigFileReader)
	{
		if (!pConfigFileReader)
		{
			return RC_INVALID_ARGS;
		}

		mWindowWidth  = pConfigFileReader->GetInt("main", "width", 640);
		mWindowHeight = pConfigFileReader->GetInt("main", "height", 480);

		mCurrent.mIsShadowMappingEnabled = pConfigFileReader->GetBool("graphics", "shadows", true);
		mCurrent.mShadowMapSizes = static_cast<U32>(pConfigFileReader->GetInt("graphics", "shadow_map_size", 512));
		mCurrent.mShadowCascadesCount = static_cast<U32>(pConfigFileReader->GetInt("graphics", "shadow_cascades_count", 3));

		mCurrent.mShadowCascadesSplits.x = static_cast<F32>(pConfigFileReader->GetFloat("graphics", "shadow_cascades_splits.0", 0.01f));
		mCurrent.mShadowCascadesSplits.y = static_cast<F32>(pConfigFileReader->GetFloat("graphics", "shadow_cascades_splits.1", 0.25f));
		mCurrent.mShadowCascadesSplits.z = static_cast<F32>(pConfigFileReader->GetFloat("graphics", "shadow_cascades_splits.2", 0.28f));
		mCurrent.mShadowCascadesSplits.w = static_cast<F32>(pConfigFileReader->GetFloat("graphics", "shadow_cascades_splits.3", 1.0f));

		return RC_OK;
	}

	TDE2_API TPtr<CGameUserSettings> CGameUserSettings::Get()
	{
		static TPtr<CGameUserSettings> pInstance = TPtr<CGameUserSettings>(new (std::nothrow) CGameUserSettings());
		return pInstance;
	}
}