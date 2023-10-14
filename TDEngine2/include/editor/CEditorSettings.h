/*!
	\file CEditorSettings.h
	\date 12.09.2023
	\author Kasimov Ildar
*/

#pragma once


#include "../core/CBaseObject.h"


#if TDE2_EDITORS_ENABLED


namespace TDEngine2
{
	class IArchiveReader;


	/*!
		class CEditorSettings

		\brief The class is a singleton which contains settings that're related to different editors
	*/

	class CEditorSettings : public CBaseObject
	{
		public:
			TDE2_API E_RESULT_CODE Init(IArchiveReader* pFileReader);

			/*!
				\brief The function is replacement of factory method for instances of this type.
				The only instance will be created per program's lifetime.

				\return A pointer to an instance of CEditorSettings type
			*/

			TDE2_API static TPtr<CEditorSettings> Get();
		private:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CEditorSettings)
		public:
			
			struct TLevelEditorSettings
			{
				bool mIsGridSnapEnabled = false;
				bool mIsGuidelinesSnapEnabled = true;

				F32  mSnapGridCellSize = 20.0f;
				F32  mGuidelinesDisplayThreshold = 20.0f;
				F32  mSnapDistance = 5.0f;

			} mLevelEditorSettings;

			struct TProfilerSettings
			{
				bool mIsCollectStartupProfileDataEnabled = false;
			} mProfilerSettings;
	};
}

#endif