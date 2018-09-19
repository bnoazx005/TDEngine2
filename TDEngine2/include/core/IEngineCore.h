/*!
	\file IEngineCore.h
	\date 16.09.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Config.h"
#include "./../utils/Types.h"


namespace TDEngine2
{
	class IEngineSubsystem;


	/*!
		interface IEngineCore

		\brief The interface represents a functionality of a core object in the engine.

		\todo	- specify Init's arguments list
				- specify a way of register/unregister callbacks
	*/

	class IEngineCore
	{
		public:
			TDE2_API virtual ~IEngineCore() = default;

			/*!
				\brief The method initializes the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init() = 0;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Free() = 0;

			/*!
				\brief The method starts up the engine

				All the code that is placed under a call of the method will be executed only
				after the engine stops or Quit method will be invoked.

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Run() = 0;

			/*!
				\brief The method stops the engine's main loop

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Quit() = 0;

			/*!
				\brief The method registers a specified interface as an engine's subsystem of
				a concrete type

				\param[in] pSubsystem A pointer to subsystem's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE RegisterSubsystem(const IEngineSubsystem* pSubsystem) = 0;

			/*!
				\brief The method unregisters specified type of a subsystem

				\param[in] subsystemType A type of a subsystem

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE UnregisterSubsystem(E_ENGINE_SUBSYSTEM_TYPE subsystemType) = 0;
		protected:
			IEngineCore() = default;
			IEngineCore(const IEngineCore& engineCore) = delete;
			virtual IEngineCore& operator= (IEngineCore& engineCore) = delete;
	};
}
