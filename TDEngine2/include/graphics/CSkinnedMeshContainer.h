/*!
	\file CSkinnedMeshContainer.h
	\date 08.07.2021
	\authors Kasimov Ildar
*/

#pragma once


#include "ISkinnedMeshContainer.h"
#include "../ecs/CBaseComponent.h"
#include "IMesh.h"


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CSkinnedMeshContainer's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CSkinnedMeshContainer's implementation
	*/

	TDE2_API IComponent* CreateSkinnedMeshContainer(E_RESULT_CODE& result);


	/*!
		class CSkinnedMeshContainer

		\brief The interface describes a functionality of a container for 3d animated meshes
	*/

	class CSkinnedMeshContainer : public ISkinnedMeshContainer, public CBaseComponent, public CPoolMemoryAllocPolicy<CSkinnedMeshContainer, 1 << 20>
	{
		public:
			friend TDE2_API IComponent* CreateSkinnedMeshContainer(E_RESULT_CODE& result);
		public:
			static const std::string mJointsArrayUniformVariableId;
			static const std::string mJointsCountUniformVariableId;
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CSkinnedMeshContainer)

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

			TDE2_API void SetDirty(bool value) override;

#if TDE2_EDITORS_ENABLED
			TDE2_API void AddSubmeshIdentifier(const std::string& submeshId) override;
#endif

			/*!
				\brief The method sets up an internal handle which points to pair
				vertex and index buffers within system that renders static meshes

				\param[in] handle An internal handle which points to pair
				vertex and index buffers within system that renders static meshes
			*/

			TDE2_API void SetSystemBuffersHandle(U32 handle) override;

			TDE2_API void SetMaterialInstanceHandle(TMaterialInstanceId materialInstanceId) override;
			
			TDE2_API void SetSkeletonName(const std::string& skeletonName) override;

			TDE2_API void SetShowDebugSkeleton(bool value) override;

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

			TDE2_API TMaterialInstanceId GetMaterialInstanceHandle() const override;

			/*!
				\brief The method returns an internal handle which points to pair
				vertex and index buffers within system that renders static meshes
			*/

			TDE2_API U32 GetSystemBuffersHandle() const override;

			TDE2_API const std::string& GetSkeletonName() const override;

			TDE2_API std::vector<TMatrix4>& GetCurrentAnimationPose() override;

			TDE2_API bool ShouldShowDebugSkeleton() const override;

			TDE2_API bool IsDirty() const override;

#if TDE2_EDITORS_ENABLED
			TDE2_API const std::vector<std::string>& GetSubmeshesIdentifiers() const override;
#endif
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CSkinnedMeshContainer)
		protected:
			std::string              mMaterialName;
			std::string              mMeshName; /// \todo replace with GUID or something like that
			std::string              mSkeletonName;			
			std::string              mSubMeshId = Wrench::StringUtils::GetEmptyStr(); ///< If the field's value is empty the whole mesh will be rendered with same material

			U32                      mSystemBuffersHandle = static_cast<U32>(-1);

			TMaterialInstanceId      mMaterialInstanceId;

			std::vector<TMatrix4>    mCurrAnimationPose;

			bool                     mShouldShowDebugSkeleton = false;

			TSubMeshRenderInfo       mSubMeshInfo;

			bool                     mIsDirty = true;

#if TDE2_EDITORS_ENABLED
			std::vector<std::string> mSubmeshesIdentifiers;
#endif
	};


	/*!
		struct TSkinnedMeshContainerParameters

		\brief The structure contains parameters for creation of CSkinnedMeshContainer
	*/

	typedef struct TSkinnedMeshContainerParameters : public TBaseComponentParameters
	{
	} TSkinnedMeshContainerParameters;


	TDE2_DECLARE_COMPONENT_FACTORY(SkinnedMeshContainer, TSkinnedMeshContainerParameters);
}