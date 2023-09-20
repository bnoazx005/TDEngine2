#include "../../include/editor/CEditorSettings.h"
#include "../../include/core/IFile.h"


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	struct TEditorSettingsArchiveKeys
	{
		static const std::string mMetaGroupId;
		static const std::string mLevelEditorSettingsGroupId;
		
		struct TLevelEditorSettingsKeys
		{
			static const std::string mIsGridSnapEnabledKeyId;			
			static const std::string mSnapGridCellSizeKeyId;			
			static const std::string mIsGuidelinesSnapEnabledKeyId;			
			static const std::string mGuidelinesDisplayThresholdKeyId;			
			static const std::string mSnapDistanceKeyId;			
		};
	};


	const std::string TEditorSettingsArchiveKeys::mLevelEditorSettingsGroupId = "level_editor_settings";
	const std::string TEditorSettingsArchiveKeys::mMetaGroupId = "meta";

	const std::string TEditorSettingsArchiveKeys::TLevelEditorSettingsKeys::mIsGridSnapEnabledKeyId = "is_grid_snap_enabled";
	const std::string TEditorSettingsArchiveKeys::TLevelEditorSettingsKeys::mSnapGridCellSizeKeyId = "snap_grid_cell_size";
	const std::string TEditorSettingsArchiveKeys::TLevelEditorSettingsKeys::mIsGuidelinesSnapEnabledKeyId = "is_guidelines_snap_enabled";
	const std::string TEditorSettingsArchiveKeys::TLevelEditorSettingsKeys::mGuidelinesDisplayThresholdKeyId = "guidelines_display_threshold";
	const std::string TEditorSettingsArchiveKeys::TLevelEditorSettingsKeys::mSnapDistanceKeyId = "snap_distance";


	CEditorSettings::CEditorSettings():
		CBaseObject()
	{
	}


	static E_RESULT_CODE LoadLevelEditorSettings(IArchiveReader* pFileReader, CEditorSettings& EditorSettings)
	{
		E_RESULT_CODE result = RC_OK;

		auto& levelEditorSettings = EditorSettings.mLevelEditorSettings;

		result = result | pFileReader->BeginGroup(TEditorSettingsArchiveKeys::mLevelEditorSettingsGroupId);
		{
			levelEditorSettings.mIsGridSnapEnabled = pFileReader->GetBool(TEditorSettingsArchiveKeys::TLevelEditorSettingsKeys::mIsGridSnapEnabledKeyId, false);			
			levelEditorSettings.mSnapGridCellSize = pFileReader->GetFloat(TEditorSettingsArchiveKeys::TLevelEditorSettingsKeys::mSnapGridCellSizeKeyId, 10.0f);			
			levelEditorSettings.mIsGuidelinesSnapEnabled = pFileReader->GetBool(TEditorSettingsArchiveKeys::TLevelEditorSettingsKeys::mIsGuidelinesSnapEnabledKeyId, false);
			levelEditorSettings.mGuidelinesDisplayThreshold = pFileReader->GetFloat(TEditorSettingsArchiveKeys::TLevelEditorSettingsKeys::mGuidelinesDisplayThresholdKeyId, 20.0f);			
			levelEditorSettings.mSnapDistance = pFileReader->GetFloat(TEditorSettingsArchiveKeys::TLevelEditorSettingsKeys::mSnapDistanceKeyId, 0.5f);			
		}
		result = result | pFileReader->EndGroup();

		return result;
	}


	E_RESULT_CODE CEditorSettings::Init(IArchiveReader* pFileReader)
	{
		if (!pFileReader)
		{
			return RC_INVALID_ARGS;
		}

		E_RESULT_CODE result = RC_OK;

		pFileReader->BeginGroup(TEditorSettingsArchiveKeys::mMetaGroupId);
		{
			if (pFileReader->GetString("resource-type") != "editor-settings")
			{
				return RC_INVALID_FILE;
			}
		}		
		pFileReader->EndGroup();

		result = result | LoadLevelEditorSettings(pFileReader, *this);
		
		return RC_OK;
	}

	TDE2_API TPtr<CEditorSettings> CEditorSettings::Get()
	{
		static TPtr<CEditorSettings> pInstance = TPtr<CEditorSettings>(new (std::nothrow) CEditorSettings());
		return pInstance;
	}
}

#endif