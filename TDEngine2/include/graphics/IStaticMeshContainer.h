/*!
	\file IStaticMeshContainer.h
	\date 07.11.2019
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Types.h"
#include "../utils/Utils.h"
#include "../ecs/IComponentFactory.h"
#include "../graphics/IRenderable.h"
#include "../utils/Color.h"
#include <string>


namespace TDEngine2
{
	struct TSubMeshRenderInfo;


	/*!
		interface IStaticMeshContainer

		\brief The interface describes a functionality of a holder of a 3D static mesh
	*/

	class IStaticMeshContainer : public IRenderable
	{
		public:
			/*!
				\brief The method sets up an identifier fo a material that will be used for the mesh's instance

				\param[in] materialName A string that contains material's name
			*/

			TDE2_API virtual void SetMaterialName(const std::string& materialName) = 0;

			/*!
				\brief The method sets up an identifier fo a mesh that will be used

				\param[in] meshName A string that contains mesh's name
			*/

			TDE2_API virtual void SetMeshName(const std::string& meshName) = 0;

			/*!
				\brief The method specifies sub-mesh identifier if it's empty the whole mesh is renderer with single material
			*/

			TDE2_API virtual void SetSubMeshId(const std::string& meshName) = 0;

			TDE2_API virtual void SetSubMeshRenderInfo(const TSubMeshRenderInfo& info) = 0;

			/*!
				\brief The method sets up an internal handle which points to pair 
				vertex and index buffers within system that renders static meshes

				\param[in] handle An internal handle which points to pair 
				vertex and index buffers within system that renders static meshes
			*/

			TDE2_API virtual void SetSystemBuffersHandle(U32 handle) = 0;

			TDE2_API virtual void SetDirty(bool value) = 0;

#if TDE2_EDITORS_ENABLED
			TDE2_API virtual void AddSubmeshIdentifier(const std::string& submeshId) = 0;
#endif

			/*!
				\brief The method returns an identifier of used material

				\return The method returns an identifier of used material
			*/

			TDE2_API virtual const std::string& GetMaterialName() const = 0;

			/*!
				\brief The method returns an identifier of used mesh

				\return The method returns an identifier of used mesh
			*/

			TDE2_API virtual const std::string& GetMeshName() const = 0;

			TDE2_API virtual const std::string& GetSubMeshId() const = 0;

			/*!
				\brief The method returns an internal handle which points to pair 
				vertex and index buffers within system that renders static meshes
			*/

			TDE2_API virtual U32 GetSystemBuffersHandle() const = 0;

			TDE2_API virtual const TSubMeshRenderInfo& GetSubMeshInfo() const = 0;

			TDE2_API virtual bool IsDirty() const = 0;

#if TDE2_EDITORS_ENABLED
			TDE2_API virtual const std::vector<std::string>& GetSubmeshesIdentifiers() const = 0;
#endif
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IStaticMeshContainer)
	};


	class IStaticMeshContainerFactory: public IGenericComponentFactory<> {};
}