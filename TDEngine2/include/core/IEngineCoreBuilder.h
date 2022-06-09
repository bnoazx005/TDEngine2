/*!
\file IEngineCoreBuilder.h
\date 32.09.2018
\authors Kasimov Ildar
*/
#pragma once


#include "../utils/Types.h"
#include "../utils/Utils.h"
#include "../core/IBaseObject.h"
#include <string>
#include <thread>


namespace TDEngine2
{
	class IEngineCore;


	TDE2_DECLARE_SCOPED_PTR(IEngineCore)


	typedef IEngineCore* (*TCreateEngineCoreCallback)(E_RESULT_CODE& result); /// A callback type of IEngineCore's factory function 


	/*!
		interface IEngineCoreBuilder

		\brief The interface describes a functionality of IEngineCore's builder that
		is intended to simplify configuration process of an engine's core
	*/

	class IEngineCoreBuilder: public virtual IBaseObject
	{
		public:									
			/*!
				\brief The method returns a configured instances of an engine core

				\return The method returns a configured instances of an engine core
			*/

			TDE2_API virtual TPtr<IEngineCore> GetEngineCore() = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IEngineCoreBuilder)

			/*!
				\brief The method tries to configure graphics context based on specified parameters

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE _configureGraphicsContext(const std::string& rendererPluginPath) = 0;

			/*!
				\brief The method configures an audio context based on type of middleware API

				\param[in] type A type of a middleware which is a basis for the context

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE _configureAudioContext(const std::string& audioPluginPath) = 0;

			/*!
				\brief The method tries to configure windows system based on specified parameters

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE _configureWindowSystem(const std::string& name, U32 width, U32 height, U32 flags) = 0;

			/*!
				\brief The method tries to configure a file system based on current data that is stored in Config.h

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE _configureFileSystem() = 0;

			/*!
				\brief The method tries to configure a resource manager

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE _configureResourceManager() = 0;

			/*!
				\brief The method tries to configure a job manager

				\param[in] maxNumOfThreads A number of worker threads

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE _configureJobManager(U32 maxNumOfThreads = std::thread::hardware_concurrency() - 1) = 0;

			/*!
				\brief The method tries to configure a plugin manager. SHOULD be called
				after a window was created

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE _configurePluginManager() = 0;

			/*!
				\brief The method tries to configure an event manager.

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE _configureEventManager() = 0;

			/*!
				\brief The method tries to configure a renderer

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE _configureRenderer() = 0;

			/*!
				\brief The method tries to configure a low-level input context

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE _configureInputContext() = 0;

			/*!
				\brief The method tries to configure an immediate GUI context

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE _configureImGUIContext() = 0;

			/*!
				\brief The method tries to configure the manager of engine's editors

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE _configureEditorsManager() = 0;

			TDE2_API virtual E_RESULT_CODE _configureSceneManager() = 0;

			TDE2_API virtual E_RESULT_CODE _configureLocalizationManager() = 0;

			TDE2_API virtual E_RESULT_CODE _configureSaveManager() = 0;
	};
}
