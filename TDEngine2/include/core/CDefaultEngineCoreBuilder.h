/*!
	\file CDefaultEngineCoreBuilder.h
	\date 31.09.2018
	\authors Kasimov Ildar
*/
#pragma once


#include "CBaseEngineCoreBuilder.h"


namespace TDEngine2
{
	/*!
		class CDefaultEngineCoreBuilder

		\brief This class is a default implementation of IEngineCoreBuilder.
	*/

	class CDefaultEngineCoreBuilder: public CBaseEngineCoreBuilder
	{
		public:
			friend TDE2_API IEngineCoreBuilder* CreateDefaultEngineCoreBuilder(TCreateEngineCoreCallback pEngineCoreFactoryCallback, const TEngineSettings& settings, E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initialized the builder's object

				\param[in] A callback to a factory's function of IEngineCore's objects

				\param[in] settings A structure's entry that contains information about engine's settings

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(TCreateEngineCoreCallback pEngineCoreFactoryCallback, const TEngineSettings& settings);
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CDefaultEngineCoreBuilder)

			TDE2_API TEngineSettings _initEngineSettings() override;
	};


	/*!
		\brief A factory function for creation objects of CDefaultEngineCoreBuilder's type

		\param[in] A callback to a factory's function of IEngineCore's objects

		\param[in] settings A structure's entry that contains information about engine's settings

		\return A pointer to CDefaultEngineCoreBuilder's implementation
	*/

	TDE2_API IEngineCoreBuilder* CreateDefaultEngineCoreBuilder(TCreateEngineCoreCallback pEngineCoreFactoryCallback, const TEngineSettings& settings, E_RESULT_CODE& result);
}
