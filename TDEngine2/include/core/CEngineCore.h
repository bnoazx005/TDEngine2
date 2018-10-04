/*!
	\file CEngineCore.h
	\date 32.09.2018
	\authors Kasimov Ildar
*/
#pragma once


#include "IEngineCore.h"
#include <vector>
#include <string>
#include <unordered_map>


namespace TDEngine2
{
	/*!
		class CEngineCore

		\brief The main class in the engine. It's literally gather all subsystems
		in a single place.
	*/

	class CEngineCore: public IEngineCore
	{
		public:
			friend TDE2_API IEngineCore* CreateEngineCore(E_RESULT_CODE& result);	
		protected:
			/*!
				\brief The enumeration contains all types of built-in events that can occur during engine's work
			*/

			enum E_ENGINE_EVENT_TYPE: U8
			{
				EET_ONSTART,
				EET_ONUPDATE,
				EET_ONFREE
			};

			typedef std::vector<IEngineListener*>                          TListenersArray;

			typedef std::unordered_map<std::string, IPlugin*>              TPluginsMap;

			typedef std::unordered_map<std::string, TSharedLibraryHandler> TLibrariesMap;
		public:
			TDE2_API virtual ~CEngineCore();

			/*!
				\brief The method initializes the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init() override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method starts up the engine

				All the code that is placed under a call of the method will be executed only
				after the engine stops or Quit method will be invoked.

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Run() override;

			/*!
				\brief The method stops the engine's main loop

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Quit() override;

			/*!
				\brief The method registers a specified interface as an engine's subsystem of
				a concrete type

				\param[in] pSubsystem A pointer to subsystem's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE RegisterSubsystem(IEngineSubsystem* pSubsystem) override;

			/*!
				\brief The method unregisters specified type of a subsystem

				\param[in] subsystemType A type of a subsystem

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE UnregisterSubsystem(E_ENGINE_SUBSYSTEM_TYPE subsystemType) override;

			/*!
				\brief The method registers a specified engine's listener

				\param[in] pSubsystem A pointer to listener's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE RegisterListener(IEngineListener* pListener) override;

			/*!
				\brief The method unregisters a specified engine's listener

				\param[in] pSubsystem A pointer to listener's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE UnregisterListener(IEngineListener* pListener) override;

			/*!
				\brief The method load a specified plugin

				\param[in] filename A filename of a shared library with a plugin's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE LoadPlugin(const std::string& filename) override;

			/*!
				\brief The method unload a specified plugin

				\param[in] filename A filename of a shared library with a plugin's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE UnloadPlugin(const std::string& filename) override;

			/*!
				\brief The method registers a specified plugin

				\param[in] pPlugin A pointer to IPlugin's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE RegisterPlugin(IPlugin* pPlugin) override;

				/*!
				\brief The method unregisters a specified plugin

				\param[in] pPlugin A pointer to IPlugin's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE UnregisterPlugin(IPlugin* pPlugin) override;

			/*!
				\brief The method returns a pointer to a subsystem of specified type

				\param[in] type A type of a subsystem which should be returned

				\returns The method returns a pointer to a subsystem of specified type
			*/

			TDE2_API IEngineSubsystem* GetSubsystem(E_ENGINE_SUBSYSTEM_TYPE type) const override;

			/*!
				\brief The method returns a pointer to a main logger of an engine

				\returns The method returns a pointer to a main logger of an engine
			*/

			TDE2_API ILogger* GetLogger() const override;

			/*!
				\brief The method returns a pointer to a in-engine timer

				\returns The method returns a pointer to a in-engine timer
			*/

			TDE2_API ITimer* GetTimer() const override;
		protected:
			TDE2_API CEngineCore();
			TDE2_API CEngineCore(const CEngineCore& engineCore) = delete;
			TDE2_API virtual CEngineCore& operator= (CEngineCore& engineCore) = delete;

			TDE2_API void _onFrameUpdateCallback();

			TDE2_API E_RESULT_CODE _onNotifyEngineListeners(E_ENGINE_EVENT_TYPE eventType);
		protected:
			bool              mIsInitialized;
			IEngineSubsystem* mSubsystems[EST_UNKNOWN]; /// stores current registered subsystems, at one time the only subsystem of specific type can be loaded			

			TListenersArray   mEngineListeners;

			TPluginsMap       mLoadedPlugins;
			TLibrariesMap     mLoadedLibraries;
	};


	/*!
		\brief A factory function for creation objects of CEngineCore's type.
		SHOULDN'T be used by its own!

		\return A pointer to CEngineCore's implementation
	*/

	TDE2_API IEngineCore* CreateEngineCore(E_RESULT_CODE& result);
}
