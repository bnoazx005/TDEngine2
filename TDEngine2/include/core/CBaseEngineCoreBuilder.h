/*!
	\file CBaseEngineCoreBuilder.h
	\date 17.11.2020
	\authors Kasimov Ildar
*/
#pragma once


#include "IEngineCoreBuilder.h"
#include "../core/CBaseObject.h"


namespace TDEngine2
{
	class IWindowSystem;
	class IJobManager;
	class IPluginManager;
	class IGraphicsContext;
	class IFileSystem;
	class IResourceManager;
	class IEventManager;
	class IImGUIContext;


	TDE2_DECLARE_SCOPED_PTR(IEventManager);
	TDE2_DECLARE_SCOPED_PTR(IGraphicsContext);
	TDE2_DECLARE_SCOPED_PTR(IWindowSystem);
	TDE2_DECLARE_SCOPED_PTR(IFileSystem);
	TDE2_DECLARE_SCOPED_PTR(IResourceManager);
	TDE2_DECLARE_SCOPED_PTR(IJobManager);
	TDE2_DECLARE_SCOPED_PTR(IPluginManager);


	/*!
		class CBaseEngineCoreBuilder

		\brief The class gathers common functionality of any engine core's builder
	*/

	class CBaseEngineCoreBuilder : public IEngineCoreBuilder, public CBaseObject
	{
		public:
			/*!
				\brief The method returns a configured instances of an engine core

				\return The method returns a configured instances of an engine core
			*/

			TDE2_API TPtr<IEngineCore> GetEngineCore() override;

#if TDE2_EDITORS_ENABLED
			TDE2_API void RegisterObjectInProfiler(const std::string& id) override;
			TDE2_API void OnBeforeMemoryRelease() override;
#endif
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseEngineCoreBuilder)

			TDE2_API E_RESULT_CODE _registerBuiltinInfrastructure(bool isWindowModeEnabled);

			/*!
				\brief The method tries to configure graphics context based on specified parameters

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE _configureGraphicsContext(E_GRAPHICS_CONTEXT_GAPI_TYPE type) override;

			/*!
				\brief The method configures an audio context based on type of middleware API

				\param[in] type A type of a middleware which is a basis for the context

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE _configureAudioContext(E_AUDIO_CONTEXT_API_TYPE type) override;

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

			TDE2_API E_RESULT_CODE _configureLocalizationManager() override;

			TDE2_API E_RESULT_CODE _configureSaveManager() override;

			TDE2_API E_RESULT_CODE _mountDirectories(E_GRAPHICS_CONTEXT_GAPI_TYPE type);

			TDE2_API virtual E_RESULT_CODE _initEngineSettings() = 0;
		protected:
			TPtr<IEngineCore> mpEngineCoreInstance;

			TPtr<IWindowSystem> mpWindowSystemInstance;

			TPtr<IJobManager> mpJobManagerInstance;

			TPtr<IPluginManager> mpPluginManagerInstance;

			TPtr<IGraphicsContext> mpGraphicsContextInstance;

			TPtr<IFileSystem> mpFileSystemInstance;

			TPtr<IResourceManager> mpResourceManagerInstance;

			TPtr<IEventManager> mpEventManagerInstance;
	};
}
