/*!
	\file IGameModesManager.h
	\date 10.08.2022
	\authors Kasimov Ildar
*/

#pragma once


#include "IEngineSubsystem.h"
#include "../utils/Utils.h"


namespace TDEngine2
{
	/*!
		\brief The interface represents a state of a game and should provide
		update logic for that
	*/

	class IGameMode: public virtual IBaseObject
	{
		public:
			TDE2_API virtual E_RESULT_CODE Init() = 0;

			/*!
				\brief The method is invoked when game modes manager activates the state
			*/

			TDE2_API virtual void OnEnter() = 0;

			/*!
				\brief The method is invoked when a game modes manager is switcher to a new state from the current one
			*/

			TDE2_API virtual void OnExit() = 0;

			/*!
				\brief The method is invoked at least once per frame when the current mode is active
			*/

			TDE2_API virtual void Update(F32 dt) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IGameMode)
	};



	/*!
		interface IGameModesManager

		\brief The interface describes a functionality of game modes switcher 
	*/

	class IGameModesManager : public IEngineSubsystem
	{
		public:
			/*!
				\brief The method initializes a state of the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init() = 0;

			TDE2_API virtual E_RESULT_CODE SwitchMode(TPtr<IGameMode> pNewMode) = 0;

			/*!
				\brief The method is invoked at least once per frame
			*/

			TDE2_API virtual void Update(F32 dt) = 0;

			TDE2_API static E_ENGINE_SUBSYSTEM_TYPE GetTypeID() { return EST_GAME_MODES_MANAGER; }
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IGameModesManager)
	};


	TDE2_DECLARE_SCOPED_PTR(IGameMode)
	TDE2_DECLARE_SCOPED_PTR(IGameModesManager)
}
