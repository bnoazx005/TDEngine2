/*!
	\file CConfigFileEngineCoreBuilder.h
	\date 23.06.2019
	\authors Kasimov Ildar
*/
#pragma once


#include "CBaseEngineCoreBuilder.h"
#include <string>


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CConfigFileEngineCoreBuilder's type

		\param[in] A callback to a factory's function of IEngineCore's objects
		\param[in] configFilename A name of a configuration file that stores engine's settings

		\return A pointer to CConfigFileEngineCoreBuilder's implementation
	*/

	TDE2_API IEngineCoreBuilder* CreateConfigFileEngineCoreBuilder(TCreateEngineCoreCallback pEngineCoreFactoryCallback, const std::string& projectConfigFilename, E_RESULT_CODE& result);


	/*!
		class CConfigFileEngineCoreBuilder

		\brief The class implements a builder of the engine's instance that's configured based on
		configuration file
	*/

	class CConfigFileEngineCoreBuilder : public CBaseEngineCoreBuilder
	{
		public:
			friend TDE2_API IEngineCoreBuilder* CreateConfigFileEngineCoreBuilder(TCreateEngineCoreCallback, const std::string&, E_RESULT_CODE&);
		public:
			/*!
				\brief The method initialized the builder's object

				\param[in] A callback to a factory's function of IEngineCore's objects

				\param[in] configFilename A name of a configuration file with settings

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(TCreateEngineCoreCallback pEngineCoreFactoryCallback, const std::string& projectConfigFilename);
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CConfigFileEngineCoreBuilder)

			TDE2_API TEngineSettings _initEngineSettings() override;
		protected:
			std::string mProjectConfigFilename;
	};
}
