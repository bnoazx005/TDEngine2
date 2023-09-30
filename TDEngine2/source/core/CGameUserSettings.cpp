#include "../../include/core/CGameUserSettings.h"
#include "../../include/platform/CConfigFileReader.h"
#include "../../include/platform/CConfigFileWriter.h"
#include "stringUtils.hpp"


namespace TDEngine2
{
	struct TUserSettingsKeys
	{
		static const std::string mMainGroupKeyId;
		static const std::string mGraphicsGroupKeyId;

		static const std::string mWidthKeyId;
		static const std::string mHeightKeyId;
		static const std::string mIsFullscreenEnabledKeyId;
		static const std::string mShadowsKeyId;
		static const std::string mShadowMapSizesKeyId;
		static const std::string mShadowCascadesSplitsKeyId;
		static const std::string mShadowCascadesCountKeyId;
	};


	const std::string TUserSettingsKeys::mMainGroupKeyId = "main";
	const std::string TUserSettingsKeys::mGraphicsGroupKeyId = "graphics";

	const std::string TUserSettingsKeys::mWidthKeyId = "width";
	const std::string TUserSettingsKeys::mHeightKeyId = "height";
	const std::string TUserSettingsKeys::mIsFullscreenEnabledKeyId = "fullscreen";
	const std::string TUserSettingsKeys::mShadowCascadesSplitsKeyId = "shadow_cascades_splits.{0}";
	const std::string TUserSettingsKeys::mShadowCascadesCountKeyId = "shadow_cascades_count";
	const std::string TUserSettingsKeys::mShadowsKeyId = "shadows";
	const std::string TUserSettingsKeys::mShadowMapSizesKeyId = "shadow_map_size";


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

		mWindowWidth  = pConfigFileReader->GetInt(TUserSettingsKeys::mMainGroupKeyId, TUserSettingsKeys::mWidthKeyId, 640);
		mWindowHeight = pConfigFileReader->GetInt(TUserSettingsKeys::mMainGroupKeyId, TUserSettingsKeys::mHeightKeyId, 480);
	
		mIsFullscreenEnabled = pConfigFileReader->GetBool(TUserSettingsKeys::mMainGroupKeyId, TUserSettingsKeys::mIsFullscreenEnabledKeyId, false);

		mCurrent.mIsShadowMappingEnabled = pConfigFileReader->GetBool(TUserSettingsKeys::mGraphicsGroupKeyId, TUserSettingsKeys::mShadowsKeyId, true);
		mCurrent.mShadowMapSizes = static_cast<U32>(pConfigFileReader->GetInt(TUserSettingsKeys::mGraphicsGroupKeyId, TUserSettingsKeys::mShadowMapSizesKeyId, 512));
		mCurrent.mShadowCascadesCount = static_cast<U32>(pConfigFileReader->GetInt(TUserSettingsKeys::mGraphicsGroupKeyId, TUserSettingsKeys::mShadowCascadesCountKeyId, 3));

		mCurrent.mShadowCascadesSplits.x = static_cast<F32>(pConfigFileReader->GetFloat(TUserSettingsKeys::mGraphicsGroupKeyId, Wrench::StringUtils::Format(TUserSettingsKeys::mShadowCascadesSplitsKeyId, 0), 0.020f));
		mCurrent.mShadowCascadesSplits.y = static_cast<F32>(pConfigFileReader->GetFloat(TUserSettingsKeys::mGraphicsGroupKeyId, Wrench::StringUtils::Format(TUserSettingsKeys::mShadowCascadesSplitsKeyId, 1), 0.045f));
		mCurrent.mShadowCascadesSplits.z = static_cast<F32>(pConfigFileReader->GetFloat(TUserSettingsKeys::mGraphicsGroupKeyId, Wrench::StringUtils::Format(TUserSettingsKeys::mShadowCascadesSplitsKeyId, 2), 0.08f));
		mCurrent.mShadowCascadesSplits.w = static_cast<F32>(pConfigFileReader->GetFloat(TUserSettingsKeys::mGraphicsGroupKeyId, Wrench::StringUtils::Format(TUserSettingsKeys::mShadowCascadesSplitsKeyId, 3), 1.0f));

		return RC_OK;
	}

	E_RESULT_CODE CGameUserSettings::Save(IConfigFileWriter* pConfigFileWriter)
	{
		if (!pConfigFileWriter)
		{
			return RC_INVALID_ARGS;
		}

		E_RESULT_CODE result = RC_OK;
		
		result = result | pConfigFileWriter->SetInt(TUserSettingsKeys::mMainGroupKeyId, TUserSettingsKeys::mWidthKeyId, mWindowWidth);
		result = result | pConfigFileWriter->SetInt(TUserSettingsKeys::mMainGroupKeyId, TUserSettingsKeys::mHeightKeyId, mWindowHeight);

		result = result | pConfigFileWriter->SetBool(TUserSettingsKeys::mGraphicsGroupKeyId, TUserSettingsKeys::mShadowsKeyId, mCurrent.mIsShadowMappingEnabled);
		result = result | pConfigFileWriter->SetInt(TUserSettingsKeys::mGraphicsGroupKeyId, TUserSettingsKeys::mShadowMapSizesKeyId, mCurrent.mShadowMapSizes);
		result = result | pConfigFileWriter->SetInt(TUserSettingsKeys::mGraphicsGroupKeyId, TUserSettingsKeys::mShadowCascadesCountKeyId, mCurrent.mShadowCascadesCount);

		result = result | pConfigFileWriter->SetFloat(TUserSettingsKeys::mGraphicsGroupKeyId, Wrench::StringUtils::Format(TUserSettingsKeys::mShadowCascadesSplitsKeyId, 0), mCurrent.mShadowCascadesSplits.x);
		result = result | pConfigFileWriter->SetFloat(TUserSettingsKeys::mGraphicsGroupKeyId, Wrench::StringUtils::Format(TUserSettingsKeys::mShadowCascadesSplitsKeyId, 1), mCurrent.mShadowCascadesSplits.y);
		result = result | pConfigFileWriter->SetFloat(TUserSettingsKeys::mGraphicsGroupKeyId, Wrench::StringUtils::Format(TUserSettingsKeys::mShadowCascadesSplitsKeyId, 2), mCurrent.mShadowCascadesSplits.z);
		result = result | pConfigFileWriter->SetFloat(TUserSettingsKeys::mGraphicsGroupKeyId, Wrench::StringUtils::Format(TUserSettingsKeys::mShadowCascadesSplitsKeyId, 3), mCurrent.mShadowCascadesSplits.w);

		return result;
	}

	TDE2_API TPtr<CGameUserSettings> CGameUserSettings::Get()
	{
		static TPtr<CGameUserSettings> pInstance = TPtr<CGameUserSettings>(new (std::nothrow) CGameUserSettings());
		return pInstance;
	}
}