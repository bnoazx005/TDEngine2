/*!
	\file ISystem.h
	\date 29.09.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "../core/IBaseObject.h"
#include "../utils/Utils.h"
#include <string>
#include <functional>


namespace TDEngine2
{
	class IJobManager;


	TDE2_DECLARE_SCOPED_PTR(IJobManager)


	/*!
		\brief The macro is used to declare virtual method for
		system's type retrieving
	*/

	#define REGISTER_SYSTEM_TYPE(Type)								\
		TDE2_API ::TDEngine2::TypeId GetSystemType() const override	\
		{															\
			return Type::GetTypeId();								\
		}															\
																	\
		TDE2_API const std::string& GetName() const override		\
		{															\
			static const std::string systemName(#Type);				\
			return systemName;										\
		}

	/*!
		\brief The macro is used to simplify system body's definition
	*/

	#define TDE2_SYSTEM(SystemName)			\
		TDE2_REGISTER_TYPE(SystemName)		\
		REGISTER_SYSTEM_TYPE(SystemName)	


	class IWorld;
	class IDebugUtility;


	/*!
		interface ISystem

		\brief The interface describes a functionality of a system.
	*/

	class ISystem : public virtual IBaseObject
	{
		public:
			typedef std::function<void()> TCommandFunctor;
		public:
			/*!
				\brief The method inject components array into a system
				
				\param[in] pWorld A pointer to a main scene's object
			*/

			TDE2_API virtual void InjectBindings(IWorld* pWorld) = 0;

			/*!
				\brief The method adds a command into the deferred executed buffer 

				\param[in] action The callback that will be executed later after all systems'll invoke their Update

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE AddDeferredCommand(const TCommandFunctor& action = nullptr) = 0;

			/*1
				\brief The method executes all deferred commands that were accumulated in current frame
			*/

			TDE2_API virtual void ExecuteDeferredCommands() = 0;

			/*!
				\brief The main method that should be implemented in all derived classes.
				A user can place initialization logic within this method
			*/

			TDE2_API virtual void OnInit(TPtr<IJobManager> pJobManager) = 0;

			/*!
				\brief The main method that should be implemented in all derived classes.
				It contains all the logic that the system will execute during engine's work.

				\param[in] pWorld A pointer to a main scene's object
				\param[in] dt A delta time's value
			*/

			TDE2_API virtual void Update(IWorld* pWorld, F32 dt) = 0;

#if TDE2_EDITORS_ENABLED

			/*!
				\brief The method is targeted to draw debug information (textual or graphical) which is related with the given system
			*/

			TDE2_API virtual void DebugOutput(IDebugUtility* pDebugUtility, F32 dt) const = 0;

#endif

			/*!
				\brief The method is invoked when the system is destroyed
			*/

			TDE2_API virtual void OnDestroy() = 0;

			TDE2_API virtual void OnActivated() = 0;
			TDE2_API virtual void OnDeactivated() = 0;

			/*!
				\brief The method returns true if the system is registered and active
			*/

			TDE2_API virtual bool IsActive() const = 0;

			TDE2_API virtual const std::string& GetName() const = 0;

			/*!
				\brief The method returns system type's identifier
				
				\return The method returns system type's identifier
			*/

			TDE2_API virtual TypeId GetSystemType() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ISystem)
	};
}