/*!
	\file CEngineCore.h
	\date 23.09.2018
	\authors Kasimov Ildar
*/
#pragma once


#include "IEngineCore.h"
#include "CBaseObject.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <mutex>
#include <atomic>


namespace TDEngine2
{
	class IDLLManager;
	class IWindowSystem;
	class ITimer;
	class IInputContext;
	class IGraphicsContext;
	class IRenderer;
	class IWorld;
	class IEventManager;
	class IImGUIContext;
	class ISceneManager;
	class IDebugUtility;


	TDE2_DECLARE_SCOPED_PTR(IWorld)
	TDE2_DECLARE_SCOPED_PTR(ITimer)


	/*!
		class CEngineCore

		\brief The main class in the engine. It's literally gather all subsystems
		in a single place.
	*/

	class CEngineCore: public IEngineCore, public CBaseObject
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

			typedef std::vector<std::unique_ptr<IEngineListener>> TListenersArray;
		public:
			/*!
				\brief The method initializes the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init() override;

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

			TDE2_API E_RESULT_CODE RegisterSubsystem(TPtr<IEngineSubsystem> pSubsystem) override;

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

			TDE2_API TResult<TEngineListenerId> RegisterListener(std::unique_ptr<IEngineListener> pListener) override;

			/*!
				\brief The method unregisters a specified engine's listener

				\param[in] listenerHandle An identifier of the registered listener

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE UnregisterListener(TEngineListenerId listenerHandle) override;

			/*!
				\brief The method returns a pointer to a main logger of an engine

				\returns The method returns a pointer to a main logger of an engine
			*/

			TDE2_API TPtr<ILogger> GetLogger() const override;

			/*!
				\brief The method returns a pointer to a in-engine timer

				\returns The method returns a pointer to a in-engine timer
			*/

			TDE2_API TPtr<ITimer> GetTimer() const override;

			/*!
				\brief The method returns a pointer to IWorld implementation

				\return The method returns a pointer to IWorld implementation
			*/

			TDE2_API TPtr<IWorld> GetWorldInstance() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CEngineCore)

			TDE2_API TPtr<IEngineSubsystem> _getSubsystem(E_ENGINE_SUBSYSTEM_TYPE type) const override;

			TDE2_API void _onFrameUpdateCallback();

			TDE2_API E_RESULT_CODE _onNotifyEngineListeners(E_ENGINE_EVENT_TYPE eventType);

			TDE2_API E_RESULT_CODE _registerSubsystemInternal(TPtr<IEngineSubsystem> pSubsystem);
			TDE2_API E_RESULT_CODE _unregisterSubsystem(E_ENGINE_SUBSYSTEM_TYPE subsystemType);

			TDE2_API E_RESULT_CODE _registerBuiltinSystems(TPtr<IWorld> pWorldInstance, IWindowSystem* pWindowSystem, IGraphicsContext* pGraphicsContext,
														   IRenderer* pRenderer, IEventManager* pEventManager);

			TDE2_API E_RESULT_CODE _cleanUpSubsystems();

			template <typename T>
			TDE2_API T* _getSubsystemAs(E_ENGINE_SUBSYSTEM_TYPE type)
			{
				static_assert(std::is_base_of<IEngineSubsystem, T>::value, "The given template argument isn't implement IEngineSubsystem interface");

				return dynamic_cast<T*>(mSubsystems[type].Get());
			}

			TDE2_API E_RESULT_CODE _onFreeInternal() override;
		protected:
			TPtr<IEngineSubsystem>  mSubsystems[EST_UNKNOWN]; /// stores current registered subsystems, at one time the only subsystem of specific type can be loaded			

			TListenersArray    mEngineListeners;

			TPtr<ITimer>       mpInternalTimer;

			TPtr<IWorld>       mpWorldInstance;

			IInputContext*     mpInputContext;

			IImGUIContext*     mpImGUIContext;

			IEditorsManager*   mpEditorsManager;

			IDebugUtility*     mpDebugUtility;

			mutable std::mutex mMutex; ///< \todo the mutex doesn't use in the code, should be fixed later!
	};


	/*!
		\brief A factory function for creation objects of CEngineCore's type.
		SHOULDN'T be used by its own!

		\return A pointer to CEngineCore's implementation
	*/

	TDE2_API IEngineCore* CreateEngineCore(E_RESULT_CODE& result);
}
