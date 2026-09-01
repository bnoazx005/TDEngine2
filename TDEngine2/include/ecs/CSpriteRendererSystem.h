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
#include "../math/TMatrix4.h"
#include "../math/TVector2.h"
#include "../utils/Color.h"
#include "../utils/CContainers.h"


namespace TDEngine2
{
	class CTransform;
	class CQuadSprite;
	class CRenderQueue;
	class IGraphicsObjectManager;
	class IVertexDeclaration;
	class IGraphicsLayersInfo;
	class IRenderer;
	class IResourceManager;
	class IAllocator;
	class CBoundsComponent;


	template <typename T> class CSTLAllocatorWrapper;


	enum class TBufferHandleId : U32;


	TDE2_DECLARE_SCOPED_PTR(IResourceManager)
	TDE2_DECLARE_SCOPED_PTR(IAllocator)
	TDE2_DECLARE_SCOPED_PTR(IGraphicsLayersInfo)


	/*!
		\brief A factory function for creation objects of CSpriteRendererSystem's type.

		\param[in, out] allocator A reference to IAllocator implementation

		\param[in, out] pRenderer A pointer to IRenderer implementation

		\param[in, out] pGraphicsObjectManager A pointer to IGraphicsObjectManager implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CSpriteRendererSystem's implementation
	*/

	TDE2_API ISystem* CreateSpriteRendererSystem(TPtr<IAllocator> allocator, IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager, E_RESULT_CODE& result);


	/*!
		class CSpriteRendererSystem

		\brief The class is a system that processes ISprite components
	*/

	class CSpriteRendererSystem : public CBaseSystem, public IRenderSystem
	{
		public:
			friend TDE2_API ISystem* CreateSpriteRendererSystem(TPtr<IAllocator>, IRenderer*, IGraphicsObjectManager*, E_RESULT_CODE&);
		public:
			typedef struct TSpriteVertex
			{
				TVector4 mPosition;

				TVector2 mUVs;
				TVector2 mPadding;
			} TSpriteVertex, *TSpriteVertexPtr;

			typedef struct TSpriteInstanceData
			{
				TMatrix4  mModelMat;

				TColor32F mColor;
			} TSpriteInstanceData, *TSpriteInstanceDataPtr;

			static_assert(std::is_trivially_destructible_v<TSpriteInstanceData>, "TSpriteInstanceData should be trivially destructible");

			typedef struct TBatchEntry
			{
				std::vector<TSpriteInstanceData, CSTLAllocatorWrapper<TSpriteInstanceData>> mInstancesData;
				
				TResourceId mMaterialHandle = TResourceId::Invalid;

				TBatchEntry() = delete;
				explicit TBatchEntry(TPtr<IAllocator> pAllocator);
			} TBatchEntry, *TBatchEntryPtr;

			typedef std::unordered_map<U32, TBatchEntry> TBatchesBuffer;

			TDE2_STATIC_CONSTEXPR U32 SPRITE_INSTANCE_DATA_BUFFER_SIZE = sizeof(TSpriteInstanceData) * (1 << 12);
		public:
			TDE2_SYSTEM(CSpriteRendererSystem);

			/*!
				\brief The method initializes an inner state of a system

				\param[in, out] allocator A reference to IAllocator implementation

				\param[in, out] pRenderer A pointer to IRenderer implementation

				\param[in, out] pGraphicsObjectManager A pointer to IGraphicsObjectManager implementation
				
				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(TPtr<IAllocator> allocator, IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager);

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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CSpriteRendererSystem)
		protected:
			TPtr<IAllocator>          mpTempAllocator;

			Vector<CTransform*>       mTransforms;

			Vector<CQuadSprite*>      mSprites;
			Vector<CBoundsComponent*> mSpritesBounds;

			IRenderer*                mpRenderer;

			TPtr<IResourceManager>    mpResourceManager;

			IGraphicsObjectManager*   mpGraphicsObjectManager;

			Vector<TBufferHandleId>   mSpritesPerInstanceDataHandles;

			TBufferHandleId           mSpriteVertexBufferHandle;
			TBufferHandleId           mSpriteIndexBufferHandle;

			U32                       mSpriteFaces[6];

			TPtr<IGraphicsLayersInfo> mpGraphicsLayers;

			TBatchesBuffer            mBatches;
	};
}