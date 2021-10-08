/*!
	\file CConfigFileEngineCoreBuilder.h
	\date 23.06.2019
	\authors Kasimov Ildar
*/
#pragma once


#include "CBaseEngineCoreBuilder.h"
#include "CEngineCore.h"
#include <string>


namespace TDEngine2
{
	typedef struct TConfigEngineCoreBuilderInputParams
	{
		TCreateEngineCoreCallback mpEngineCoreFactoryCallback = CreateEngineCore;
		std::string               mProjectConfigFilepath = "Default.project";
		std::string               mUserConfigFilepath = "settings.cfg";
	} TConfigEngineCoreBuilderInputParams, *TConfigEngineCoreBuilderInputParamsPtr;


	/*!
		\brief A factory function for creation objects of CConfigFileEngineCoreBuilder's type

		\return A pointer to CConfigFileEngineCoreBuilder's implementation
	*/

	TDE2_API IEngineCoreBuilder* CreateConfigFileEngineCoreBuilder(const TConfigEngineCoreBuilderInputParams& params, E_RESULT_CODE& result);


	/*!
		class CConfigFileEngineCoreBuilder

		\brief The class implements a builder of the engine's instance that's configured based on
		configuration file
	*/

	class CConfigFileEngineCoreBuilder : public CBaseEngineCoreBuilder
	{
		public:
			friend TDE2_API IEngineCoreBuilder* CreateConfigFileEngineCoreBuilder(const TConfigEngineCoreBuilderInputParams&, E_RESULT_CODE&);
		public:
			/*!
				\brief The method initialized the builder's object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(const TConfigEngineCoreBuilderInputParams& params);
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CConfigFileEngineCoreBuilder)

			TDE2_API E_RESULT_CODE _initEngineSettings() override;
		protected:
			std::string mProjectConfigFilepath;
			std::string mGameUserSettingsFilepath;
	};
}
