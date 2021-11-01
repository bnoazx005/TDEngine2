/*!
	\file CBasePluginManager.h
	\date 20.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "IPluginManager.h"
#include "CBaseObject.h"
#include <unordered_map>
#include <mutex>


namespace TDEngine2
{
	class IPlugin;
	class IDLLManager;


	TDE2_DECLARE_SCOPED_PTR(IDLLManager)


	/*!
		\brief A factory function for creation objects of CBasePluginManager's type

		\param[in, out] pEngineCore A pointer to IEngineCore's implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CBasePluginManager's implementation
	*/

	TDE2_API IPluginManager* CreateBasePluginManager(TPtr<IEngineCore> pEngineCore, E_RESULT_CODE& result);


	/*!
		class CBasePluginManager

		\brief The class implements common functionality of a plugin manager
	*/

	class CBasePluginManager : public IPluginManager, public CBaseObject
	{
		public:
			friend TDE2_API IPluginManager* CreateBasePluginManager(TPtr<IEngineCore>, E_RESULT_CODE&);
		protected:
			typedef std::unordered_map<std::string, TPtr<IPlugin>> TPluginsMap;
		public:
			/*!
				\brief The method initializes the object

				\param[in, out] pEngineCore A pointer to IEngineCore's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(TPtr<IEngineCore> pEngineCore) override;

			/*!
				\brief The method load a specified plugin

				\param[in] filename A filename (WITHOUT extension) of a shared library with a plugin's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE LoadPlugin(const std::string& filename) override;

			/*!
				\brief The method unload a specified plugin

				\param[in] filename A filename (WITHOUT extension) of a shared library with a plugin's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE UnloadPlugin(const std::string& filename) override;
			
			/*!
				\brief The method unloads all loaded plugins from memory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE UnloadAllPlugins() override;

			/*!
				\brief The method allows plugins to initialize and register ECS infrastructure like systems or components

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE RegisterECSComponents(TPtr<IWorld> pWorld) override;

			/*!
				\brief The method returns a type of the subsystem

				\return A type, which is represented with E_ENGINE_SUBSYSTEM_TYPE's value
			*/

			TDE2_API E_ENGINE_SUBSYSTEM_TYPE GetType() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBasePluginManager)
			
			TDE2_API virtual E_RESULT_CODE _onFreeInternal();
		protected:
			TPluginsMap        mLoadedPlugins;

			TPtr<IEngineCore>  mpEngineCore;

			TPtr<IDLLManager>  mpDLLManager;

			mutable std::mutex mMutex; ///< \todo the mutex doesn't use in the code, should be fixed later!
	};
}