/*!
	\file CGameModesManager.h
	\date 10.08.2022
	\authors Kasimov Ildar
*/

#pragma once


#include "IGameModesManager.h"
#include "CBaseObject.h"
#include <stack>


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CGameModesManager's type.
		
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CGameModesManager's implementation
	*/

	TDE2_API IGameModesManager* CreateGameModesManager(E_RESULT_CODE& result);


	/*!
		class CGameModesManager

		\brief The interface describes a functionality of game modes switcher that supports stacking modes to
		provide backtracking between them
	*/

	class CGameModesManager : public IGameModesManager, public CBaseObject
	{
		public:
			friend TDE2_API IGameModesManager* CreateGameModesManager(E_RESULT_CODE&);

		public:
			/*!
				\brief The method initializes an object's state

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init() override;

			TDE2_API E_RESULT_CODE SwitchMode(TPtr<IGameMode> pNewMode) override;

			/*!
				\brief The method is invoked at least once per frame
			*/

			TDE2_API void Update(F32 dt) override;

			/*!
				\brief The method returns a type of the subsystem

				\return A type, which is represented with E_ENGINE_SUBSYSTEM_TYPE's value
			*/

			TDE2_API E_ENGINE_SUBSYSTEM_TYPE GetType() const override;

			TDE2_API static E_ENGINE_SUBSYSTEM_TYPE GetTypeID() { return EST_GAME_MODES_MANAGER; }
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CGameModesManager)

			TDE2_API TPtr<IGameMode> _getCurrentGameMode();
		protected:
			std::stack<TPtr<IGameMode>> mModesContext;
	};
}
