/*!
	\file CBasePluginManager.h
	\date 20.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "IPluginManager.h"
#include <unordered_map>
#include <mutex>


namespace TDEngine2
{
	class IPlugin;
	class IDLLManager;


	/*!
		\brief A factory function for creation objects of CBasePluginManager's type

		\param[in, out] pEngineCore A pointer to IEngineCore's implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CBasePluginManager's implementation
	*/

	TDE2_API IPluginManager* CreateBasePluginManager(IEngineCore* pEngineCore, E_RESULT_CODE& result);


	/*!
		class CBasePluginManager

		\brief The class implements common functionality of a plugin manager
	*/

	class CBasePluginManager : public IPluginManager
	{
		public:
			friend TDE2_API IPluginManager* CreateBasePluginManager(IEngineCore* pEngineCore, E_RESULT_CODE& result);
		protected:
			typedef std::unordered_map<std::string, IPlugin*> TPluginsMap;
		public:
			/*!
				\brief The method initializes the object

				\param[in, out] pEngineCore A pointer to IEngineCore's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IEngineCore* pEngineCore) override;

			/*!
			\brief The method frees all memory occupied by the object

			\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

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
				\brief The method returns a type of the subsystem

				\return A type, which is represented with E_ENGINE_SUBSYSTEM_TYPE's value
			*/

			TDE2_API E_ENGINE_SUBSYSTEM_TYPE GetType() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBasePluginManager)
		protected:
			bool               mIsInitialized;

			TPluginsMap        mLoadedPlugins;

			IEngineCore*       mpEngineCore;

			IDLLManager*       mpDLLManager;

			mutable std::mutex mMutex; ///< \todo the mutex doesn't use in the code, should be fixed later!
	};
}