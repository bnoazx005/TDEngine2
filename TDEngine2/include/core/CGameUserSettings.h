/*!
	\file CGameUserSettings.h
	\date 07.10.2021
	\author Kasimov Ildar
*/

#pragma once


#include "CBaseObject.h"
#include "CProjectSettings.h"


namespace TDEngine2
{
	class IConfigFileReader;
	class IConfigFileWriter;


	/*!
		class CGameUserSettings

		\brief The class is a singleton which contains all bunch of user related settings that
		vary from project to project.

		This is runtime settings of the application in contrast with CProjectSettings
	*/

	class CGameUserSettings : public CBaseObject
	{
		public:
			TDE2_API E_RESULT_CODE Init(IConfigFileReader* pConfigFileReader);

			TDE2_API E_RESULT_CODE Save(IConfigFileWriter* pConfigFileWriter);

			/*!
				\brief The function is replacement of factory method for instances of this type.
				The only instance will be created per program's lifetime.

				\return A pointer to an instance of CGameUserSettings type
			*/

			TDE2_API static TPtr<CGameUserSettings> Get();
		private:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CGameUserSettings)
		public:
			I32 mWindowWidth = 1024;
			I32 mWindowHeight = 768;

			CProjectSettings::TQualityPreset mCurrent;
	};
}