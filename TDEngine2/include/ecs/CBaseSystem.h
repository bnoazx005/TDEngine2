/*!
	\file CBaseSystem.h
	\date 15.11.2019
	\authors Kasimov Ildar
*/

#pragma once


#include "../core/CBaseObject.h"
#include "ISystem.h"
#include <vector>


namespace TDEngine2
{
	/*!
		abstract class CBaseSystem

		\brief The class is a base for all systems in the engine
	*/

	class CBaseSystem : public virtual ISystem, public CBaseObject
	{
		public:
			typedef std::vector<TEntityId> TEntitiesArray;
		public:
			/*!
				\brief The main method that should be implemented in all derived classes.
				A user can place initialization logic within this method
			*/

			TDE2_API void OnInit() override;

			/*!
				\brief The method adds a command into the deffered executed buffer

				\param[in] action The callback that will be executed later after all systems'll invoke their Update

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE AddDefferedCommand(const TCommandFunctor& action = nullptr) override;

			/*1
				\brief The method executes all deffered commands that were accumulated in current frame
			*/

			TDE2_API void ExecuteDefferedCommands() override;

			/*!
				\brief The method is invoked when the system is destroyed
			*/

			TDE2_API void OnDestroy() override;

			TDE2_API void OnActivated() override;			
			TDE2_API void OnDeactivated() override;

			/*!
				\brief The method returns true if the system is registered and active
			*/

			TDE2_API bool IsActive() const override;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(CBaseSystem)
		private:
			std::vector<TCommandFunctor> mDefferedCommandsBuffer;
			bool                         mIsActive;
	};
}