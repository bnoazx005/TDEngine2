/*!
	\file CYAMLSupportPlugin.h
	\date 09.09.2019
	\authors Kasimov Ildar
*/

#pragma once

#if defined (TDE2_YAML_PLUGIN_STATIC) /// \note the macro means that the plugin will be used as a part of the engine not as dynamically linked plugin
	#include "./../../include/core/IPlugin.h"
#else
	#include <core/IPlugin.h>
#endif

#include "IYAMLFile.h"
#include "CYAMLFile.h"
#include "./../deps/yaml/Yaml.hpp"
#if defined (TDE2_YAML_PLUGIN_IMPLEMENTATION) // \note this solution is used because of mini-yaml library is inaccessible from code outisde of the plugin
#include "./../deps/yaml/Yaml.cpp"			  // if you can propose better solution, you're welcome!
#endif


namespace TDEngine2
{
	class IEngineCore;
	class IFileSystem;


	/*!
		class CYAMLSupportPlugin

		\brief The class implements a plugin, which provides a support
		of serializing/deserializing of data into/from YAML format 1.0
	*/

	class CYAMLSupportPlugin : public IPlugin
	{
		public:
			friend TDE2_API TDEngine2::IPlugin* TDE2_APIENTRY::CreatePlugin(TDEngine2::IEngineCore* pEngineCore, TDEngine2::E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an internal state of a plugin

				\param[in] pEngineCore A pointer to IEngineCore's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/
			TDE2_API E_RESULT_CODE Init(IEngineCore* pEngineCore) override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method returns an object, which contains all the information
				about the plugin. The structure TPluginInfo is declared in Types.h.

				\return The method returns an object, which contains all the information
				about the plugin
			*/

			TDE2_API const TPluginInfo& GetInfo() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CYAMLSupportPlugin)
		protected:
			bool         mIsInitialized;

			IEngineCore* mpEngineCoreInstance;

			IFileSystem* mpFileSystemInstance;

			TPluginInfo  mPluginInfo;
	};
}