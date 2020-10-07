/*!
	\file CBoundsUpdatingSystem.h
	\date 28.04.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Types.h"
#include "./../utils/Utils.h"
#include "CBaseSystem.h"
#include <vector>
#include <functional>


namespace TDEngine2
{
	class CTransform;
	class CEntity;
	class IDebugUtility;
	class IResourceManager;


	/*!
		\brief A factory function for creation objects of CBoundsUpdatingSystem's type.

		\param[in, out] pResourceManager A pointer to IResourceManager implementation
		\param[in, out] pDebugUtility A pointer to IDebugUtility implementation
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CBoundsUpdatingSystem's implementation
	*/

	TDE2_API ISystem* CreateBoundsUpdatingSystem(IResourceManager* pResourceManager, IDebugUtility* pDebugUtility, E_RESULT_CODE& result);


	/*!
		class CBoundsUpdatingSystem

		\brief The class represents a system that updates boundaries of renderable objects
	*/

	class CBoundsUpdatingSystem : public CBaseSystem
	{
		public:
			friend TDE2_API ISystem* CreateBoundsUpdatingSystem(IResourceManager*, IDebugUtility*, E_RESULT_CODE&);
		public:
			TDE2_SYSTEM(CBoundsUpdatingSystem);

			/*!
				\brief The method initializes an inner state of a system

				\param[in, out] pResourceManager A pointer to IResourceManager implementation
				\param[in, out] pDebugUtility A pointer to IDebugUtility implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IDebugUtility* pDebugUtility);

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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBoundsUpdatingSystem)

			TDE2_API void _processEntities(IWorld* pWorld, const std::vector<TEntityId>& entities, const std::function<void(CEntity*)>& processCallback);

			TDE2_API void _computeStaticMeshBounds(CEntity* pEntity);
			TDE2_API void _computeSpritesBounds(CEntity* pEntity);
		protected:
			std::vector<TEntityId> mStaticMeshesEntities;
			std::vector<TEntityId> mSpritesEntities;

			IResourceManager*      mpResourceManager;

			IDebugUtility*         mpDebugUtility;
	};
}