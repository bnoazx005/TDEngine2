/*!
	\file CGameModesManager.h
	\date 10.08.2022
	\authors Kasimov Ildar
*/

#pragma once


#include "IGameModesManager.h"
#include "CBaseObject.h"
#include <stack>
#include <string>


namespace TDEngine2
{
	enum class TSceneId : U32;


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
				\brief The method places a new mode over the existing one and allows to back to that later instead of SwitchMode
				which replaces the mode

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE PushMode(TPtr<IGameMode> pNewMode) override;

			/*!
				\brief The method extracts current active mode and activates the previous one if it exists

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE PopMode() override;

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


	/*!
		\brief A factory function for creation objects of CBaseGameMode's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CBaseGameMode's implementation
	*/

	TDE2_API IGameMode* CreateBaseGameMode(IGameModesManager* pOwner, E_RESULT_CODE& result);


	/*!
		class CBaseGameMode

		\brief The class is a base for all user-defined game modes
	*/

	class CBaseGameMode : public IGameMode, public CBaseObject
	{
		public:
			friend TDE2_API IGameMode* CreateBaseGameMode(IGameModesManager* pOwner, E_RESULT_CODE&);

		public:
			TDE2_API E_RESULT_CODE Init(IGameModesManager* pOwner) override;

			/*!
				\brief The method is invoked when game modes manager activates the state
			*/

			TDE2_API void OnEnter() override;

			/*!
				\brief The method is invoked when a game modes manager is switcher to a new state from the current one
			*/

			TDE2_API void OnExit() override;

			/*!
				\brief The method is invoked at least once per frame when the current mode is active
			*/

			TDE2_API void Update(F32 dt) override;
		protected:
			TDE2_API explicit CBaseGameMode(const std::string& name);

			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseGameMode)
		protected:
			IGameModesManager* mpOwner;
			std::string        mName;
	};


	class ISceneManager;


	typedef struct TSplashScreenModeParams
	{
		F32 mMaxShowDuration = 4.0f;
	} TSplashScreenModeParams, *TSplashScreenModeParamsPtr;


	/*!
		\brief A factory function for creation objects of CBaseGameMode's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CBaseGameMode's implementation
	*/

	TDE2_API IGameMode* CreateSplashScreenGameMode(IGameModesManager* pOwner, TPtr<ISceneManager> pSceneManager, const TSplashScreenModeParams& params, E_RESULT_CODE& result);


	/*!
		class CSplashScreenGameMode

		\brief The class represents a mode which is active when application is launched
	*/

	class CSplashScreenGameMode : public CBaseGameMode
	{
		public:
			friend TDE2_API IGameMode* CreateSplashScreenGameMode(IGameModesManager*, TPtr<ISceneManager>, const TSplashScreenModeParams& params, E_RESULT_CODE&);
		public:
			/*!
				\brief The method is invoked when game modes manager activates the state
			*/

			TDE2_API void OnEnter() override;

			/*!
				\brief The method is invoked when a game modes manager is switcher to a new state from the current one
			*/

			TDE2_API void OnExit() override;

			/*!
				\brief The method is invoked at least once per frame when the current mode is active
			*/

			TDE2_API void Update(F32 dt) override;
		private:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CSplashScreenGameMode)
		private:
			TPtr<ISceneManager> mpSceneManager;

			F32                 mMaxShowDuration = 4.0f;
			F32                 mCurrTime = 0.0f;

			TSceneId            mSplashScreenSceneHandle;
	};


	TDE2_DECLARE_SCOPED_PTR(ISceneManager)
}
