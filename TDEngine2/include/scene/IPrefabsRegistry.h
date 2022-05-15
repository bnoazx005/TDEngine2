/*!
	\file IPrefabsRegistry.h
	\date 13.05.2022
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Utils.h"
#include "../utils/Types.h"
#include "../core/IBaseObject.h"


namespace TDEngine2
{
	class IFileSystem;
	class IResourceManager;
	class IWorld;
	class CEntity;


	TDE2_DECLARE_SCOPED_PTR(IFileSystem)
	TDE2_DECLARE_SCOPED_PTR(IResourceManager)
	TDE2_DECLARE_SCOPED_PTR(IWorld)


	/*!
		interface IPrefabsRegistry

		\brief The interface describes functionality of prefabs collection. It lazy loads prefabs and reuse this information later.
		
		The manifest of prefabs has the following structure

		prefabs:
			- prefab-id: PrefabId 
				physical-path-to-prefab: /Prefabs/Prefab1
			...

		It's implemented in the engine as a resource

	*/

	class IPrefabsRegistry: public virtual IBaseObject
	{
		public:
			/*!
				\brief The method initializes the internal state of the object

				\param[in, out] pResourceManager A pointer to IResourceManager implementation
				\param[in, out] pFileSystem A pointer to IFileSystem implementation
				\param[in, out] pWorld A pointer to IWorld implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(TPtr<IResourceManager> pResourceManager, TPtr<IFileSystem> pFileSystem, TPtr<IWorld> pWorld) = 0;

			TDE2_API virtual CEntity* Spawn(const std::string& id, CEntity* pParent = nullptr) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IPrefabsRegistry)
	};
}