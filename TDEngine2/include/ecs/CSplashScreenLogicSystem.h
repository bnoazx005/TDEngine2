/*!
	\file CSplashScreenLogicSystem.h
	\date 20.02.2024
	\authors Kasimov Ildar
*/

#pragma once


#include "CBaseSystem.h"
#include "../core/CGameModesManager.h"
#include "../ecs/IWorld.h"


namespace TDEngine2
{
	class CSplashScreenItemComponent;


	/*!
		\brief A factory function for creation objects of CSplashScreenLogicSystem's type.

		\param[in] params An input parameters for the system
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CSplashScreenLogicSystem's implementation
	*/

	TDE2_API ISystem* CreateSplashScreenLogicSystem(const TSplashScreenModeParams& params, E_RESULT_CODE& result);


	class CSplashScreenLogicSystem : public CBaseSystem
	{
		public:
			friend TDE2_API ISystem* CreateSplashScreenLogicSystem(const TSplashScreenModeParams&, E_RESULT_CODE&);
		public:
			TDE2_SYSTEM(CSplashScreenLogicSystem);

			/*!
				\brief The method initializes an inner state of a system

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(const TSplashScreenModeParams& params);

			/*!
				\brief The method inject components array into a system

				\param[in] pWorld A pointer to a main scene's object
			*/

			TDE2_API void InjectBindings(IWorld* pWorld) override;

			/*!
				\brief The main method that should be implemented in all derived classes.
				It contains all the logic that the system will execute during engine's work.

				\param[in] pWorld A pointer to a main scene's object

				\param[in] dt A delta time's value
			*/

			TDE2_API void Update(IWorld* pWorld, F32 dt) override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CSplashScreenLogicSystem)
		protected:
			TComponentsQueryLocalSlice<CSplashScreenItemComponent> mContext;

			TSplashScreenModeParams::TSkipCallbackAction mShouldSkipPredicate = nullptr;
	};
}