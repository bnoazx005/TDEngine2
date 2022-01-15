/*!
	\file CMeshAnimatorUpdatingSystem.h
	\date 18.07.2021
	\authors Kasimov Ildar
*/

#pragma once


#include "CBaseSystem.h"
#include "IWorld.h"


namespace TDEngine2
{
	class IResourceManager;
	class CAnimationContainerComponent;
	class CSkinnedMeshContainer;
	class CMeshAnimatorComponent;
	class CBoundsComponent;


	/*!
		\brief A factory function for creation objects of CMeshAnimatorUpdatingSystem's type.

		\param[in, out] pResourceManager A pointer to IResourceManager implementation
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CMeshAnimatorUpdatingSystem's implementation
	*/

	TDE2_API ISystem* CreateMeshAnimatorUpdatingSystem(IResourceManager* pResourceManager, E_RESULT_CODE& result);


	/*!
		class CMeshAnimatorUpdatingSystem

		\brief The class is a system that processes skinnedMesh animator's states
	*/

	class CMeshAnimatorUpdatingSystem : public CBaseSystem
	{
		public:
			friend TDE2_API ISystem* CreateMeshAnimatorUpdatingSystem(IResourceManager*, E_RESULT_CODE& result);
		public:
			TDE2_SYSTEM(CMeshAnimatorUpdatingSystem);

			/*!
				\brief The method initializes an inner state of a system

				\param[in, out] pResourceManager A pointer to IResourceManager implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager);

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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CMeshAnimatorUpdatingSystem)

		protected:
			TComponentsQueryLocalSlice<CSkinnedMeshContainer, CMeshAnimatorComponent, CAnimationContainerComponent, CBoundsComponent> mEntitiesContext;
			
			IResourceManager* mpResourceManager;
	};
}