/*!
	\file IPrefabsManifest.h
	\date 14.05.2022
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Utils.h"
#include "../utils/Types.h"
#include "../core/IBaseObject.h"
#include "../core/IResource.h"
#include "../core/IResourceLoader.h"
#include "../core/IResourceFactory.h"
#include "../core/Serialization.h"


namespace TDEngine2
{
	class IFileSystem;
	class IResourceManager;


	/*!
		struct TPrefabsManifestParameters
	*/

	typedef struct TPrefabsManifestParameters : TBaseResourceParameters
	{
	} TPrefabsManifestParameters, *TPrefabsManifestParametersPtr;


	/*!
		interface IPrefabsManifest

		\brief The manifest of prefabs has the following structure

		prefabs:
			- prefab-id: PrefabId 
				physical-path-to-prefab: /Prefabs/Prefab1
			...

		It's implemented in the engine as a resource

	*/

	class IPrefabsManifest: public ISerializable, public virtual IBaseObject
	{
		public:
			TDE2_REGISTER_TYPE(IPrefabsManifest);

			/*!
				\brief The method initializes an internal state of a font

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation
				\param[in] name A resource's name

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, const std::string& name) = 0;

			TDE2_API virtual E_RESULT_CODE AddPrefabInfo(const std::string& prefabId, const std::string& pathToPrefab) = 0;

			TDE2_API virtual const std::string& GetPathToPrefabById(const std::string& prefabId) const = 0;

			TDE2_API virtual std::vector<std::string> GetPrefabsIdentifiers() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IPrefabsManifest)
	};
}