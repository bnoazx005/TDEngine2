/*!
	\file CVideoPlayersPlugin.h
	\date 12.02.2024
	\authors Kasimov Ildar
*/

#pragma once


#include <core/CBaseObject.h>
#include <core/IPlugin.h>


namespace TDEngine2
{
	class IEngineCore;


	/*!
		class CVideoPlayersPlugin

		\brief The class implements a plugin which add support of different video codecs into the engine
	*/

	class CVideoPlayersPlugin : public CBaseObject, public IPlugin, public IECSPlugin
	{
		public:
			friend TDE2_API TDEngine2::IPlugin* TDE2_APIENTRY::CreatePlugin(TDEngine2::IEngineCore*, TDEngine2::E_RESULT_CODE&);
		public:
			/*!
				\brief The method initializes an internal state of a plugin

				\param[in] pEngineCore A pointer to IEngineCore's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/
			TDE2_API E_RESULT_CODE Init(IEngineCore* pEngineCore) override;

			/*!
				\brief The method is called before OnStart method from IEngineListener and after IWorld is fully initialized and ready to run

				\param[in, out] pEngineCore A pointer to IEngineCore's implementation
				\param[in, out] pWorld A pointer to IWorld's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE OnRegisterComponents(IEngineCore* pEngineCore, IWorld* pWorld) override;

			/*!
				\brief The method is called after OnStart method from IEngineListener and after IWorld is fully initialized and ready to run

				\param[in, out] pEngineCore A pointer to IEngineCore's implementation
				\param[in, out] pWorld A pointer to IWorld's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE OnRegisterSystems(IEngineCore* pEngineCore, IWorld* pWorld) override;

			/*!
				\brief The method returns an object, which contains all the information
				about the plugin. The structure TPluginInfo is declared in Types.h.

				\return The method returns an object, which contains all the information
				about the plugin
			*/

			TDE2_API const TPluginInfo& GetInfo() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CVideoPlayersPlugin)
		protected:
			IEngineCore* mpEngineCoreInstance;
	};
}