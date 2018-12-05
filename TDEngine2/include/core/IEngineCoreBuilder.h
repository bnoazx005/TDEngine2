/*!
\file IEngineCoreBuilder.h
\date 32.09.2018
\authors Kasimov Ildar
*/
#pragma once


#include "./../utils/Types.h"
#include "./../utils/Utils.h"
#include <string>
#include <thread>


namespace TDEngine2
{
	class IEngineCore;


	typedef IEngineCore* (*TCreateEngineCoreCallback)(E_RESULT_CODE& result); /// A callback type of IEngineCore's factory function 


	/*!
		interface IEngineCoreBuilder

		\brief The interface describes a functionality of IEngineCore's builder that
		is intended to simplify configuration process of an engine's core
	*/

	class IEngineCoreBuilder
	{
		public:			
			/*!
				\brief The method initialized the builder's object

				\param[in] A callback to a factory's function of IEngineCore's objects

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(TCreateEngineCoreCallback pEngineCoreFactoryCallback) = 0;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Free() = 0;

			/*!
				\brief The method tries to configure graphics context based on specified parameters

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE ConfigureGraphicsContext(E_GRAPHICS_CONTEXT_GAPI_TYPE type) = 0;

			/*!
				\brief The method tries to configure windows system based on specified parameters

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE ConfigureWindowSystem(const std::string& name, U32 width, U32 height, U32 flags) = 0;

			/*!
				\brief The method tries to configure a file system based on current data that is stored in Config.h

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE ConfigureFileSystem() = 0;

			/*!
				\brief The method tries to configure a resource manager

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE ConfigureResourceManager() = 0;

			/*!
				\brief The method tries to configure a job manager

				\param[in] maxNumOfThreads A number of worker threads

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE ConfigureJobManager(U32 maxNumOfThreads = std::thread::hardware_concurrency() - 1) = 0;
			
			/*!
				\brief The method tries to configure a plugin manager. SHOULD be called
				after a window was created
				
				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE ConfigurePluginManager() = 0;
			
			/*!
				\brief The method tries to configure an event manager.

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE ConfigureEventManager() = 0;
						
			/*!
				\brief The method returns a configured instances of an engine core

				\return The method returns a configured instances of an engine core
			*/

			TDE2_API virtual IEngineCore* GetEngineCore() = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IEngineCoreBuilder)
	};
}
