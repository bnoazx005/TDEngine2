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

		return RC_OK;
	}

	TDE2_API TPtr<CGameUserSettings> CGameUserSettings::Get()
	{
		static TPtr<CGameUserSettings> pInstance = TPtr<CGameUserSettings>(new (std::nothrow) CGameUserSettings());
		return pInstance;
	}
}