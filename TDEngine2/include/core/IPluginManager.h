/*!
	\file IPluginManager.h
	\date 20.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "IEngineSubsystem.h"
#include "./../utils/Utils.h"
#include "./../utils/Types.h"
#include <string>


namespace TDEngine2
{
	class IEngineCore;
	class IWorld;


	/*!
		interface IPluginManager

		\brief The interface represents a manager that controls loading/unloading
		process of plugins
	*/

	class IPluginManager : public IEngineSubsystem
	{
		public:
			/*!
				\brief The method initializes the object

				\param[in, out] pEngineCore A pointer to IEngineCore's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IEngineCore* pEngineCore) = 0;

			/*!
				\brief The method load a specified plugin

				\param[in] filename A filename (WITHOUT extension) of a shared library with a plugin's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE LoadPlugin(const std::string& filename) = 0;

			/*!
				\brief The method unloads a specified plugin

				\param[in] filename A filename (WITHOUT extension) of a shared library with a plugin's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE UnloadPlugin(const std::string& filename) = 0;

			/*!
				\brief The method unloads all loaded plugins from memory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE UnloadAllPlugins() = 0;

			/*!
				\brief The method allows plugins to initialize and register ECS infrastructure like systems or components

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE RegisterECSComponents(IWorld* pWorld) = 0;

			TDE2_API static E_ENGINE_SUBSYSTEM_TYPE GetTypeID() { return EST_PLUGIN_MANAGER; }
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IPluginManager)
	};
}