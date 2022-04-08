/*!
	\file CLODMeshSwitchSystem.h
	\date 07.04.2022
	\authors Kasimov Ildar
*/

#pragma once


#include "CBaseSystem.h"
#include "IWorld.h"


namespace TDEngine2
{
	class CTransform;
	class CStaticMeshContainer;
	class CSkinnedMeshContainer;
	class CLODStrategyComponent;
	class CCamerasContextComponent;


	/*!
		\brief A factory function for creation objects of CLODMeshSwitchSystem's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CLODMeshSwitchSystem's implementation
	*/

	TDE2_API ISystem* CreateLODMeshSwitchSystem(E_RESULT_CODE& result);


	/*!
		class CLODMeshSwitchSystem

		\brief The class is a system that replaces meshes with their simplified alternatives when they go into long distance from a camera
	*/

	class CLODMeshSwitchSystem : public CBaseSystem
	{
		public:
			friend TDE2_API ISystem* CreateLODMeshSwitchSystem(E_RESULT_CODE& result);
		
		public:
			TDE2_SYSTEM(CLODMeshSwitchSystem);

			/*!
				\brief The method initializes an inner state of a system

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init();

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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CLODMeshSwitchSystem)
		private:
			TComponentsQueryLocalSlice<CLODStrategyComponent, CStaticMeshContainer, CTransform>  mStaticMeshesLODs;
			TComponentsQueryLocalSlice<CLODStrategyComponent, CSkinnedMeshContainer, CTransform> mSkinnedMeshesLODs;

			CCamerasContextComponent*                                                            mpCamerasContext = nullptr;
			CTransform*                                                                          mpCurrActiveCameraTransform = nullptr;
	};
}