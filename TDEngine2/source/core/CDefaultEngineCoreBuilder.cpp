#include "../../include/core/CDefaultEngineCoreBuilder.h"
#include <memory>
#include <cstring>
#include <tuple>


namespace TDEngine2
{
	CDefaultEngineCoreBuilder::CDefaultEngineCoreBuilder():
		CBaseEngineCoreBuilder()
	{
	}

	/*!
		\brief The method initialized the builder's object

		\param[in] A callback to a factory's function of IEngineCore's objects

		\return RC_OK if everything went ok, or some other code, which describes an error
	*/

	E_RESULT_CODE CDefaultEngineCoreBuilder::Init(TCreateEngineCoreCallback pEngineCoreFactoryCallback, const TEngineSettings& settings)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		if (!pEngineCoreFactoryCallback)
		{
			return RC_INVALID_ARGS;
		}

		memcpy(&mEngineSettings, &settings, sizeof(settings));

		E_RESULT_CODE result = RC_OK;

		mpEngineCoreInstance = pEngineCoreFactoryCallback(result);

		if (result != RC_OK)
		{
			return result;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	TEngineSettings CDefaultEngineCoreBuilder::_initEngineSettings() 
	{
		return mEngineSettings;
	}

	
	TDE2_API IEngineCoreBuilder* CreateDefaultEngineCoreBuilder(TCreateEngineCoreCallback pEngineCoreFactoryCallback, const TEngineSettings& settings, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IEngineCoreBuilder, CDefaultEngineCoreBuilder, result, pEngineCoreFactoryCallback, settings);
	}
}