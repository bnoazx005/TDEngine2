/*!
	\file IPlugin.h
	\date 04.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "IBaseObject.h"
#include "./../utils/Utils.h"


namespace TDEngine2
{
	class IEngineCore;
	class IWorld;


	/*!
		interface IPlugin

		\brief The interface represents a basic functionality of a plugin,
		which can be used by the engine.
	*/

	class IPlugin: public virtual IBaseObject
	{
		public:
			/*!
				\brief The method initializes an internal state of a plugin

				\param[in] pEngineCore A pointer to IEngineCore's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/
			TDE2_API virtual E_RESULT_CODE Init(IEngineCore* pEngineCore) = 0;

			/*!
				\brief The method returns an object, which contains all the information
				about the plugin. The structure TPluginInfo is declared in Types.h.

				\return The method returns an object, which contains all the information
				about the plugin
			*/

			TDE2_API virtual const TPluginInfo& GetInfo() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IPlugin)
	};


	/*!
		\brief If some plugin needs to work with ECS's world or something like that, it should implement this interface
	*/

	class IECSPlugin
	{
		public:
			/*!
				\brief The method is called before OnStart method from IEngineListener and after IWorld is fully initialized and ready to run

				\param[in, out] pEngineCore A pointer to IEngineCore's implementation
				\param[in, out] pWorld A pointer to IWorld's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE OnRegisterComponents(IEngineCore* pEngineCore, IWorld* pWorld) = 0;
			
			/*!
				\brief The method is called after OnStart method from IEngineListener and after IWorld is fully initialized and ready to run

				\param[in, out] pEngineCore A pointer to IEngineCore's implementation
				\param[in, out] pWorld A pointer to IWorld's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE OnRegisterSystems(IEngineCore* pEngineCore, IWorld* pWorld) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IECSPlugin)
	};


	typedef TDE2_API TDEngine2::IPlugin* (TDE2_APIENTRY *TCreatePluginCallback)(TDEngine2::IEngineCore* pEngineCore, TDEngine2::E_RESULT_CODE& result);
}


#define TDE2_CREATE_PLUGIN_FUNC_NAME "CreatePlugin"


/*!
	\brief The function should be implemented by derived classes to provide
	opportunity to create an instances of concrete plugins

	\return A pointer to IPlugin's implementation
*/

extern "C" TDE2_API TDEngine2::IPlugin* TDE2_APIENTRY CreatePlugin(TDEngine2::IEngineCore* pEngineCore, TDEngine2::E_RESULT_CODE& result);