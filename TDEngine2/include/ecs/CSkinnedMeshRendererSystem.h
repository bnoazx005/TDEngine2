/*!
	\file CSkinnedMeshRendererSystem.h
	\date 08.07.2021
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
	class CSkinnedMeshContainer;
	class IWorld;
	class CRenderQueue;
	class IResourceManager;
	class IMaterial;
	class IVertexBuffer;
	class IIndexBuffer;
	class IVertexDeclaration;
	class COrthoCamera;
	class CPerspectiveCamera;
	class CEntity;
	class ICamera;


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

	class CSkinnedMeshRendererSystem : public CBaseSystem
	{
		public:
			friend TDE2_API ISystem* CreateSkinnedMeshRendererSystem(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager, E_RESULT_CODE& result);
		public:
			typedef struct TMeshBuffersEntry
			{
				IVertexBuffer*      mpVertexBuffer;
				IIndexBuffer*       mpIndexBuffer;
				IVertexDeclaration* mpVertexDecl;
			} TMeshBuffersEntry, *TMeshBuffersEntryPtr;

			typedef std::vector<std::tuple<CTransform*, CSkinnedMeshContainer*>> TEntitiesArray;
			typedef std::vector<TPtr<IMaterial>>                                 TMaterialsArray;
			typedef std::vector<TMeshBuffersEntry>                               TMeshBuffersMap;
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
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CSkinnedMeshRendererSystem)

			TDE2_API void _collectUsedMaterials(const TEntitiesArray& entities, IResourceManager* pResourceManager, TMaterialsArray& usedMaterials);

			TDE2_API void _populateCommandsBuffer(const TEntitiesArray& entities, CRenderQueue*& pRenderGroup, TPtr<IMaterial> pCurrMaterial,
												  const ICamera* pCamera);

			TDE2_API U32 _computeMeshCommandHash(TResourceId materialId, F32 distanceToCamera);
		protected:
			TEntitiesArray          mProcessingEntities;

			IGraphicsObjectManager* mpGraphicsObjectManager;

			TPtr<IResourceManager>  mpResourceManager;
			
			IRenderer*              mpRenderer;

			CRenderQueue*           mpOpaqueRenderGroup;
			CRenderQueue*           mpTransparentRenderGroup;

			TMaterialsArray         mCurrMaterialsArray;

			TMeshBuffersMap         mMeshBuffersMap;
	};
}