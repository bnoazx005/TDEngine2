/*!
	\file CStaticMeshRendererSystem.h
	\date 12.10.2019
	\authors Kasimov Ildar
*/

#pragma once


#include "CBaseSystem.h"
#include <vector>
#include <tuple>


namespace TDEngine2
{
	class IRenderer;
	class IGraphicsObjectManager;
	class CTransform;
	class CStaticMeshContainer;
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


	TDE2_DECLARE_SCOPED_PTR(IResourceManager)
	TDE2_DECLARE_SCOPED_PTR(IMaterial)


	/*!
		\brief A factory function for creation objects of CStaticMeshRendererSystem's type.

		\param[in, out] pRenderer A pointer to IRenderer implementation

		\param[in, out] pGraphicsObjectManager A pointer to IGraphicsObjectManager implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CStaticMeshRendererSystem's implementation
	*/

	TDE2_API ISystem* CreateStaticMeshRendererSystem(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager, E_RESULT_CODE& result);


	/*!
		class CStaticMeshRendererSystem

		\brief The class is a system that processes IStaticMesh components
	*/

	class CStaticMeshRendererSystem : public CBaseSystem
	{
		public:
			friend TDE2_API ISystem* CreateStaticMeshRendererSystem(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager, E_RESULT_CODE& result);
		public:
			typedef struct TMeshBuffersEntry
			{
				TBufferHandleId     mVertexBufferHandle;
				TBufferHandleId     mIndexBufferHandle;
				IVertexDeclaration* mpVertexDecl;
			} TMeshBuffersEntry, *TMeshBuffersEntryPtr;

			typedef std::vector<std::tuple<CTransform*, CStaticMeshContainer*, CBoundsComponent*>> TEntitiesArray;
			typedef std::vector<TPtr<IMaterial>>                                                   TMaterialsArray;
			typedef std::vector<TMeshBuffersEntry>                                                 TMeshBuffersMap;
		public:
			TDE2_SYSTEM(CStaticMeshRendererSystem);

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
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CStaticMeshRendererSystem)

			TDE2_API void _collectUsedMaterials(const TEntitiesArray& entities, IResourceManager* pResourceManager, TMaterialsArray& usedMaterials);

			TDE2_API void _populateCommandsBuffer(const TEntitiesArray& entities, CRenderQueue*& pRenderGroup, CRenderQueue* pDepthOnlyRenderGroup, TPtr<IMaterial> pCurrMaterial,
												  const ICamera* pCamera);

			TDE2_API U32 _computeMeshCommandHash(TResourceId materialId, F32 distanceToCamera);
		protected:
			TEntitiesArray          mProcessingEntities;

			IGraphicsObjectManager* mpGraphicsObjectManager = nullptr;

			TPtr<IResourceManager>  mpResourceManager = nullptr;

			CRenderQueue*           mpOpaqueRenderGroup = nullptr;
			CRenderQueue*           mpTransparentRenderGroup = nullptr;
			CRenderQueue*           mpDepthOnlyRenderGroup = nullptr;

			TMaterialsArray         mCurrMaterialsArray;

			TMeshBuffersMap         mMeshBuffersMap;
	};
}