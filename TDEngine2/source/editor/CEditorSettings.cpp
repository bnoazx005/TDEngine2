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
		};
	};


	const std::string TEditorSettingsArchiveKeys::mLevelEditorSettingsGroupId = "level_editor_settings";
	const std::string TEditorSettingsArchiveKeys::mMetaGroupId = "meta";

	const std::string TEditorSettingsArchiveKeys::TLevelEditorSettingsKeys::mIsGridSnapEnabledKeyId = "is_grid_snap_enabled";
	const std::string TEditorSettingsArchiveKeys::TLevelEditorSettingsKeys::mSnapGridCellSizeKeyId = "snap_grid_cell_size";


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