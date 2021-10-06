/*!
	\file IEngineCore.h
	\date 16.09.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Config.h"
#include "../utils/Types.h"
#include "../utils/Utils.h"
#include "IEngineListener.h"
#include "IBaseObject.h"
#include <string>
#include <memory>


namespace TDEngine2
{
	class IEngineSubsystem;
	class IEngineListener;
	class ILogger;
	class ITimer;
	class IPlugin;
	class IWorld;
	class IEditorsManager;


	/*!
		interface IEngineCore

		\brief The interface represents a functionality of a core object in the engine
	*/

	class IEngineCore: public virtual IBaseObject
	{
		public:
			/*!
				\brief The method initializes the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init() = 0;

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

			TDE2_API virtual E_RESULT_CODE RegisterSubsystem(IEngineSubsystem* pSubsystem) = 0;

			/*!
				\brief The method unregisters specified type of a subsystem

				\param[in] subsystemType A type of a subsystem

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE UnregisterSubsystem(E_ENGINE_SUBSYSTEM_TYPE subsystemType) = 0;
			
			/*!
				\brief The method registers a specified engine's listener

				\param[in] pListener A pointer to listener's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual TResult<TEngineListenerId> RegisterListener(std::unique_ptr<IEngineListener> pListener) = 0;

			/*!
				\brief The method unregisters a specified engine's listener

				\param[in] listenerHandle An identifier of the registered listener

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE UnregisterListener(TEngineListenerId listenerHandle) = 0;

			/*!
				\brief The method returns a pointer to a subsystem of specified type

				\returns The method returns a pointer to a subsystem of specified type
			*/

			template <typename T>
#if _HAS_CXX17
			std::enable_if_t<std::is_base_of_v<IEngineSubsystem, T>, T*>
#else
			typename std::enable_if<std::is_base_of<IEngineSubsystem, T>::value, T*>::type
#endif
			GetSubsystem() const
			{
				T* pInternalSystem = static_cast<T*>(_getSubsystem(T::GetTypeID()));

				TDE2_ASSERT(pInternalSystem);
				return pInternalSystem;
			}

			/*!
				\brief The method returns a pointer to a main logger of an engine if 
				_DEBUG flag is defined. Otherwise method always return nullptr.

				\returns The method returns a pointer to a main logger of an engine
			*/

			TDE2_API virtual TPtr<ILogger> GetLogger() const = 0;

			/*!
				\brief The method returns a pointer to a in-engine timer

				\returns The method returns a pointer to a in-engine timer
			*/

			TDE2_API virtual ITimer* GetTimer() const = 0;

			/*!
				\brief The method returns a pointer to IWorld implementation

				\return The method returns a pointer to IWorld implementation
			*/

			TDE2_API virtual IWorld* GetWorldInstance() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IEngineCore)

			TDE2_API virtual IEngineSubsystem* _getSubsystem(E_ENGINE_SUBSYSTEM_TYPE type) const = 0;
	};
}
