/*!
	\file CObjectsSelectionSystem.h
	\date 02.03.2019
	\authors Kasimov Ildar
*/

#pragma once


#include "CBaseSystem.h"


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	class CEntity;
	class CTransform;
	class IRenderer;
	class IGraphicsObjectManager;
	class IResourceManager;
	class CRenderQueue;
	class IWorld;
	class ICamera;
	class IVertexDeclaration;
	class IVertexBuffer;
	class IIndexBuffer;
	class IMaterial;
	class CStaticMeshContainer;
	class CSkinnedMeshContainer;
	class CQuadSprite;
	class CUIElementMeshData;
	class CLayoutElement;


	TDE2_DECLARE_SCOPED_PTR(IResourceManager)


	/*!
		\brief A factory function for creation objects of CObjectsSelectionSystem's type.

		\param[in, out] pRenderer A pointer to IRenderer implementation
		\param[in, out] pGraphicsObjectManager A pointer to IGraphicsObjectManager implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CObjectsSelectionSystem's implementation
	*/

	TDE2_API ISystem* CreateObjectsSelectionSystem(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager, E_RESULT_CODE& result);


	/*!
		class CObjectsSelectionSystem

		\brief The class is a system that processes object picking
	*/

	class CObjectsSelectionSystem : public CBaseSystem
	{
		public:
			friend TDE2_API ISystem* CreateObjectsSelectionSystem(IRenderer*, IGraphicsObjectManager*, E_RESULT_CODE&);
		public:
			template <typename T>
			struct TSystemContext
			{
				std::vector<T*>          mpRenderables;
				std::vector<CTransform*> mpTransforms;
				std::vector<bool>        mHasSelectedEntityComponent;
				std::vector<TEntityId>   mEntityIds;
			};

			struct TUIElementsSystemContext : TSystemContext<CUIElementMeshData>
			{
				std::vector<CLayoutElement*> mLayoutElements;
			};

			typedef TSystemContext<CStaticMeshContainer>  TStaticMeshesContext;
			typedef TSystemContext<CSkinnedMeshContainer> TSkinnedMeshesContext;
			typedef TSystemContext<CQuadSprite>           TSpritesMeshesContext;
			typedef TUIElementsSystemContext              TUIElementsContext;
		public:
			TDE2_SYSTEM(CObjectsSelectionSystem);

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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CObjectsSelectionSystem)

			TDE2_API E_RESULT_CODE _initSpriteBuffers();
			TDE2_API E_RESULT_CODE _initSelectionMaterials();

			TDE2_API void _processSpriteEntity(U32 drawIndex, CRenderQueue* pCommandBuffer, CEntity* pEntity, TResourceId materialHandle);

			TDE2_API ICamera* _getEditorCamera(IWorld* pWorld, TEntityId cameraEntityId);
		protected:
			TStaticMeshesContext    mStaticMeshesContext;
			TSkinnedMeshesContext   mSkinnedMeshesContext;
			TSpritesMeshesContext   mSpritesContext;
			TUIElementsContext      mUIElementsContext;

			IGraphicsObjectManager* mpGraphicsObjectManager;

			TPtr<IResourceManager>  mpResourceManager;

			CRenderQueue*           mpEditorOnlyRenderQueue;
			CRenderQueue*           mpDebugRenderQueue;

			TEntityId               mCameraEntityId;

			IVertexDeclaration*     mpSelectionVertDecl;
			IVertexDeclaration*     mpSelectionSkinnedVertDecl;

			IVertexBuffer*          mpSpritesVertexBuffer;
			IIndexBuffer*           mpSpritesIndexBuffer;

			IVertexBuffer*          mpUIElementsVertexBuffer;

			TResourceId             mSelectionMaterialHandle;
			TResourceId             mSelectionSkinnedMaterialHandle;
			TResourceId             mSelectionUIMaterialHandle;

			TResourceId             mSelectionOutlineMaterialHandle;
			TResourceId             mSelectionSkinnedOutlineMaterialHandle;

			USIZE                   mUIElementsVertexBufferCurrOffset = 0;
	};
}

#endif