/*!
	\file CAnimationSystem.h
	\date 30.01.2021
	\authors Kasimov Ildar
*/

#pragma once


#include "CBaseSystem.h"
#include <vector>


namespace TDEngine2
{
	class IResourceManager;


	/*!
		\brief A factory function for creation objects of CAnimationSystem's type.

		\param[in, out] pResourceManager A pointer to IResourceManager implementation
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CAnimationSystem's implementation
	*/

	TDE2_API ISystem* CreateAnimationSystem(IResourceManager* pResourceManager, E_RESULT_CODE& result);


	/*!
		class CAnimationSystem

		\brief The class is a system that processes IAnimation components
	*/

	class CAnimationSystem : public CBaseSystem
	{
		public:
			friend TDE2_API ISystem* CreateAnimationSystem(IResourceManager*, E_RESULT_CODE& result);
		public:
			TDE2_SYSTEM(CAnimationSystem);

			/*!
				\brief The method initializes an inner state of a system

		\param[in, out] pResourceManager A pointer to IResourceManager implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager);

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CAnimationSystem)

			static TDE2_API F32 _adjustTimeToFitRange(F32 time, bool isLooping, F32 startTime, F32 endTime);

		protected:
			IResourceManager* mpResourceManager;
			
			std::vector<TEntityId> mAnimatedEntities;
	};
}