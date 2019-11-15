/*!
	\file CSpriteRendererSystem.h
	\date 04.12.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "CBaseSystem.h"
#include <vector>
#include <tuple>
#include <unordered_map>
#include "./../math/TMatrix4.h"
#include "./../math/TVector2.h"
#include "./../utils/Color.h"
#include "./../utils/CContainers.h"


namespace TDEngine2
{
	class CTransform;
	class CQuadSprite;
	class CRenderQueue;
	class IGraphicsObjectManager;
	class IVertexBuffer;
	class IIndexBuffer;
	class IVertexDeclaration;
	class IGraphicsLayersInfo;
	class IRenderer;
	class IResourceManager;
	class IResourceHandler;
	class IAllocator;


	/*!
		\brief A factory function for creation objects of CSpriteRendererSystem's type.

		\param[in, out] allocator A reference to IAllocator implementation

		\param[in, out] pRenderer A pointer to IRenderer implementation

		\param[in, out] pGraphicsObjectManager A pointer to IGraphicsObjectManager implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CSpriteRendererSystem's implementation
	*/

	TDE2_API ISystem* CreateSpriteRendererSystem(IAllocator& allocator, IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager, E_RESULT_CODE& result);


	/*!
		class CSpriteRendererSystem

		\brief The class is a system that processes ISprite components
	*/

	class CSpriteRendererSystem : public CBaseSystem
	{
		public:
			friend TDE2_API ISystem* CreateSpriteRendererSystem(IAllocator& allocator, IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager, E_RESULT_CODE& result);
		protected:
			typedef struct TSpriteVertex
			{
				TVector4 mPosition;

				TVector2 mUVs;
			} TSpriteVertex, *TSpriteVertexPtr;

			typedef struct TSpriteInstanceData
			{
				TMatrix4  mModelMat;

				TColor32F mColor;
			} TSpriteInstanceData, *TSpriteInstanceDataPtr;

			typedef struct TBatchEntry
			{
				//std::vector<TSpriteInstanceData> mInstancesData;
				CDynamicArray<TSpriteInstanceData>* mpInstancesData;
				
				IResourceHandler*                mpMaterialHandler;
			} TBatchEntry, *TBatchEntryPtr;

			typedef std::unordered_map<U32, TBatchEntry> TBatchesBuffer;
		public:
			/*!
				\brief The method initializes an inner state of a system

		\		param[in, out] allocator A reference to IAllocator implementation

				\param[in, out] pRenderer A pointer to IRenderer implementation

				\param[in, out] pGraphicsObjectManager A pointer to IGraphicsObjectManager implementation
				
				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IAllocator& allocator, IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager);

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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CSpriteRendererSystem)

			TDE2_API U32 _computeSpriteCommandKey(TResourceId materialId, U16 graphicsLayerId);

			TDE2_API void _initializeBatchVertexBuffers(IGraphicsObjectManager* pGraphicsObjectManager, U32 numOfBuffers);
		protected:
			IAllocator*                 mpTempAllocator;

			std::vector<CTransform*>    mTransforms;

			std::vector<CQuadSprite*>   mSprites;

			IRenderer*                  mpRenderer;

			IResourceManager*           mpResourceManager;

			CRenderQueue*               mpRenderQueue;

			IGraphicsObjectManager*     mpGraphicsObjectManager;

			std::vector<IVertexBuffer*> mSpritesPerInstanceData;

			IVertexBuffer*              mpSpriteVertexBuffer;

			IIndexBuffer*               mpSpriteIndexBuffer;

			IVertexDeclaration*         mpSpriteVertexDeclaration;

			U16                         mSpriteFaces[6];

			IGraphicsLayersInfo*        mpGraphicsLayers;

			TBatchesBuffer              mBatches;
	};
}