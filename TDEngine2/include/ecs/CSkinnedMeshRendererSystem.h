/*!
	\file CSkinnedMeshRendererSystem.h
	\date 08.07.2021
	\authors Kasimov Ildar
*/

#pragma once


#include "CBaseSystem.h"
#include "../math/TMatrix4.h"
#include <vector>
#include <tuple>


namespace TDEngine2
{
	class IRenderer;
	class IGraphicsObjectManager;
	class CTransform;
	class CSkinnedMeshContainer;
	class IWorld;
	class CRenderQueue;
	class IResourceManager;
	class IMaterial;
	class IVertexDeclaration;
	class COrthoCamera;
	class CPerspectiveCamera;
	class CEntity;
	class ICamera;
	class CBoundsComponent;


	enum class TBufferHandleId : U32;
	enum class E_GEOMETRY_SUBGROUP_TAGS : U32;
	enum class TMaterialInstanceId : U32;


	TDE2_DECLARE_SCOPED_PTR(IResourceManager)
	TDE2_DECLARE_SCOPED_PTR(IMaterial)


	/*!
		\brief A factory function for creation objects of CSkinnedMeshRendererSystem's type.

		\param[in, out] pRenderer A pointer to IRenderer implementation
		\param[in, out] pGraphicsObjectManager A pointer to IGraphicsObjectManager implementation
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CSkinnedMeshRendererSystem's implementation
	*/

	TDE2_API ISystem* CreateSkinnedMeshRendererSystem(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager, E_RESULT_CODE& result);


	/*!
		class CSkinnedMeshRendererSystem

		\brief The class is a system that processes ISkinnedMesh components
	*/

	class CSkinnedMeshRendererSystem : public CBaseSystem, public IRenderSystem
	{
		public:
			friend TDE2_API ISystem* CreateSkinnedMeshRendererSystem(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager, E_RESULT_CODE& result);
		public:
			typedef struct TMeshBuffersEntry
			{
				TBufferHandleId     mVertexBufferHandle;
				TBufferHandleId     mIndexBufferHandle;
				IVertexDeclaration* mpVertexDecl;
			} TMeshBuffersEntry, *TMeshBuffersEntryPtr;

			typedef std::vector<std::tuple<CTransform*, CSkinnedMeshContainer*, CBoundsComponent*>> TSystemContext;
			typedef std::vector<TPtr<IMaterial>>                                                    TMaterialsArray;
			typedef std::vector<TMeshBuffersEntry>                                                  TMeshBuffersMap;


			struct TMeshDrawEntry
			{
				CSkinnedMeshContainer*   mpMeshContainer = nullptr;
				TMatrix4                 mModelMat{};
				TMatrix4                 mInvModelMat{};
				TBufferHandleId          mSharedPositionOnlyVertexBufferHandle;
				TBufferHandleId          mSharedIndexBufferHandle;
				TResourceId              mMaterialHandle;
				TResourceId              mMeshHandle;

				U32                      mStartIndex = 0;
				U32                      mIndicesCount = 0;
				U32                      mVertexFormatFlags = 0;

				F32                      mDistanceToCamera = 0.0f;
				E_GEOMETRY_SUBGROUP_TAGS mGeometrySubGroupTag;

				TMaterialInstanceId      mMaterialInstanceId;
			};

		public:
			TDE2_SYSTEM(CSkinnedMeshRendererSystem);

			/*!
				\brief The method initializes an inner state of a system

				\param[in, out] pRenderer A pointer to IRenderer implementation

				\param[in, out] pGraphicsObjectManager A pointer to IGraphicsObjectManager implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager);

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

			TDE2_API E_RESULT_CODE FillFramePacket(TFramePacket& framePacket) override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CSkinnedMeshRendererSystem)

			USIZE _collectUsedMaterials(const TSystemContext& entities, IResourceManager* pResourceManager, TMaterialsArray& usedMaterials);

			void _prepareLocalRenderCommands(const TSystemContext& entities, TPtr<IMaterial> pCurrMaterial, const ICamera* pCamera, Vector<TMeshDrawEntry>& visibleMeshes);
		protected:
			TSystemContext          mProcessingEntities;

			IGraphicsObjectManager* mpGraphicsObjectManager = nullptr;

			TPtr<IResourceManager>  mpResourceManager = nullptr;
			
			IRenderer*              mpRenderer = nullptr;

			TMaterialsArray         mCurrMaterialsArray;

			TMeshBuffersMap         mMeshBuffersMap;

			ICamera*                mpCameraComponent = nullptr;

			Vector<TMeshDrawEntry>  mVisibleOpaqueMeshes{};
			Vector<TMeshDrawEntry>  mVisibleTransparentMeshes{};
	};
}