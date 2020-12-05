/*!
	\file CBaseEngineCoreBuilder.h
	\date 17.11.2020
	\authors Kasimov Ildar
*/
#pragma once


#include "IEngineCoreBuilder.h"


namespace TDEngine2
{
	class IWindowSystem;
	class IJobManager;
	class IPluginManager;
	class IMemoryManager;
	class IGraphicsContext;
	class IFileSystem;
	class IResourceManager;
	class IEventManager;
	class IImGUIContext;


	/*!
		class CBaseEngineCoreBuilder

		\brief The class gathers common functionality of any engine core's builder
	*/

	class CBaseEngineCoreBuilder : public IEngineCoreBuilder
	{
		public:
			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method returns a configured instances of an engine core

				\return The method returns a configured instances of an engine core
			*/

			TDE2_API IEngineCore* GetEngineCore() override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseEngineCoreBuilder)

			TDE2_API E_RESULT_CODE _registerBuiltinInfrastructure();

			/*!
				\brief The method tries to configure graphics context based on specified parameters

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE _configureGraphicsContext(E_GRAPHICS_CONTEXT_GAPI_TYPE type) override;

			/*!
				\brief The method tries to configure windows system based on specified parameters.
				The window system is chosen based on macro definitions, which are declared in Config.h file.

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE _configureWindowSystem(const std::string& name, U32 width, U32 height, U32 flags) override;

			/*!
				\brief The method tries to configure a file system based on current data that is stored in Config.h

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE _configureFileSystem() override;

			/*!
				\brief The method tries to configure a resource manager

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE _configureResourceManager() override;

			/*!
				\brief The method tries to configure a job manager

				\param[in] maxNumOfThreads A number of worker threads

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE _configureJobManager(U32 maxNumOfThreads) override;

			/*!
				\brief The method tries to configure a plugin manager

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE _configurePluginManager() override;

			/*!
				\brief The method tries to configure an event manager.

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE _configureEventManager() override;

			/*!
				\brief The method trie tot configure a memory manager

				\param[in] totalMemorySize A size in bytes of a global memory block that
				will be allocated

				\return The method trie tot configure a memory manager
			*/

			TDE2_API E_RESULT_CODE _configureMemoryManager(U32 totalMemorySize) override;

			/*!
				\brief The method tries to configure a renderer

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE _configureRenderer() override;

			/*!
				\brief The method tries to configure a low-level input context

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE _configureInputContext() override;

			/*!
				\brief The method tries to configure an immediate GUI context

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE _configureImGUIContext() override;

			/*!
				\brief The method tries to configure the manager of engine's editors

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE _configureEditorsManager() override;

			TDE2_API E_RESULT_CODE _configureSceneManager() override;

			TDE2_API virtual TEngineSettings _initEngineSettings() = 0;
		protected:
			bool              mIsInitialized;

			TEngineSettings   mEngineSettings;

			IEngineCore*      mpEngineCoreInstance;

			IWindowSystem*    mpWindowSystemInstance;

			IJobManager*      mpJobManagerInstance;

			IPluginManager*   mpPluginManagerInstance;

			IMemoryManager*   mpMemoryManagerInstance;

			IGraphicsContext* mpGraphicsContextInstance;

			IFileSystem*      mpFileSystemInstance;

			IResourceManager* mpResourceManagerInstance;

			IEventManager*    mpEventManagerInstance;

			IImGUIContext*    mpImGUIContext;
	};
}