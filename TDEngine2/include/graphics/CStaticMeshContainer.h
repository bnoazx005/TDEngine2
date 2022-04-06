/*!
	\file CStaticMeshContainer.h
	\date 07.11.2019
	\authors Kasimov Ildar
*/

#pragma once


#include "IStaticMeshContainer.h"
#include "IMesh.h"
#include "../ecs/CBaseComponent.h"


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CStaticMeshContainer's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CStaticMeshContainer's implementation
	*/

	TDE2_API IComponent* CreateStaticMeshContainer(E_RESULT_CODE& result);


	/*!
		interface CStaticMeshContainer

		\brief The interface describes a functionality of a container for 3d static meshes
		that stores a reference to a mesh resource, a material and other things
	*/

	class CStaticMeshContainer : public IStaticMeshContainer, public CBaseComponent, public CPoolMemoryAllocPolicy<CStaticMeshContainer, 1 << 20>
	{
		public:
			friend TDE2_API IComponent* CreateStaticMeshContainer(E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CStaticMeshContainer)

			/*!
				\brief The method deserializes object's state from given reader

				\param[in, out] pReader An input stream of data that contains information about the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Load(IArchiveReader* pReader) override;

			/*!
				\brief The method serializes object's state into given stream

				\param[in, out] pWriter An output stream of data that writes information about the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Save(IArchiveWriter* pWriter) override;

			/*!
				\brief The method sets up an identifier fo a material that will be used for the sprite

				\param[in] materialName A string that contains material's name

				\param[in] materialId An identifier of a material
			*/

			TDE2_API void SetMaterialName(const std::string& materialName) override;

			/*!
				\brief The method sets up an identifier fo a mesh that will be used

				\param[in] meshName A string that contains mesh's name
			*/

			TDE2_API void SetMeshName(const std::string& meshName) override;

			/*!
				\brief The method specifies sub-mesh identifier if it's empty the whole mesh is renderer with single material
			*/

			TDE2_API void SetSubMeshId(const std::string& meshName) override;

			TDE2_API void SetSubMeshRenderInfo(const TSubMeshRenderInfo& info) override;

			/*!
				\brief The method sets up an internal handle which points to pair
				vertex and index buffers within system that renders static meshes

				\param[in] handle An internal handle which points to pair
				vertex and index buffers within system that renders static meshes
			*/

			TDE2_API void SetSystemBuffersHandle(U32 handle) override;

			TDE2_API void SetDirty(bool value) override;

#if TDE2_EDITORS_ENABLED
			TDE2_API void AddSubmeshIdentifier(const std::string& submeshId) override;
#endif

			/*!
				\brief The method returns an identifier of used material

				\return The method returns an identifier of used material
			*/

			TDE2_API const std::string& GetMaterialName() const override;

			/*!
				\brief The method returns an identifier of used mesh

				\return The method returns an identifier of used mesh
			*/

			TDE2_API const std::string& GetMeshName() const override;

			TDE2_API const std::string& GetSubMeshId() const override;

			TDE2_API const TSubMeshRenderInfo& GetSubMeshInfo() const override;

			/*!
				\brief The method returns an internal handle which points to pair
				vertex and index buffers within system that renders static meshes
			*/

			TDE2_API U32 GetSystemBuffersHandle() const override;

			TDE2_API bool IsDirty() const override;

#if TDE2_EDITORS_ENABLED
			TDE2_API const std::vector<std::string>& GetSubmeshesIdentifiers() const override;
#endif
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CStaticMeshContainer)
		protected:
			std::string              mMaterialName;
			std::string              mMeshName; /// \todo replace with GUID or something like that
			std::string              mSubMeshId = Wrench::StringUtils::GetEmptyStr(); ///< If the field's value is empty the whole mesh will be rendered with same material

			U32                      mSystemBuffersHandle = static_cast<U32>(-1);

			TSubMeshRenderInfo       mSubMeshInfo;

			bool                     mIsDirty = true;

#if TDE2_EDITORS_ENABLED
			std::vector<std::string> mSubmeshesIdentifiers;
#endif
	};


	/*!
		struct TStaticMeshContainerParameters

		\brief The structure contains parameters for creation of CStaticMeshContainer
	*/

	typedef struct TStaticMeshContainerParameters : public TBaseComponentParameters
	{
	} TStaticMeshContainerParameters;


	TDE2_DECLARE_COMPONENT_FACTORY(StaticMeshContainer, TStaticMeshContainerParameters);
}