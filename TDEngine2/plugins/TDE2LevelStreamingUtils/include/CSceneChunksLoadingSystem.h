/*!
	\file CSceneChunksLoadingSystem.h
	\date 27.05.2022
	\authors Kasimov Ildar
*/

#pragma once


#include <utils/Types.h>
#include <utils/Utils.h>
#include <ecs/CBaseSystem.h>
#include <ecs/IWorld.h>
#include <vector>


namespace TDEngine2
{
	class ISceneManager;
	class CTransform;
	class CBoundsComponent;
	class CSceneLoadingTriggerComponent;


	/*!
		\brief A factory function for creation objects of CSceneChunksLoadingSystem's type.

		\param[in, out] pSceneManager A pointer to ISceneManager's implementation
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CSceneChunksLoadingSystem's implementation
	*/

	TDE2_API ISystem* CreateSceneChunksLoadingSystem(ISceneManager* pSceneManager, E_RESULT_CODE& result);


	/*!
		class CSceneChunksLoadingSystem

		\brief The class is a system that processes ISceneChunksLoading components
	*/

	class CSceneChunksLoadingSystem : public CBaseSystem
	{
		public:
			friend TDE2_API ISystem* CreateSceneChunksLoadingSystem(ISceneManager*, E_RESULT_CODE&);
		public:
			TDE2_SYSTEM(CSceneChunksLoadingSystem);

			/*!
				\brief The method initializes an inner state of a system

				\param[in, out] pSceneManager A pointer to ISceneManager's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(ISceneManager* pSceneManager);

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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CSceneChunksLoadingSystem)

		protected:
			ISceneManager* mpSceneManager;

			CTransform*    mCurrActiveCameraTransform;

			TComponentsQueryLocalSlice<CSceneLoadingTriggerComponent, CBoundsComponent, CTransform> mContext;
	};
}