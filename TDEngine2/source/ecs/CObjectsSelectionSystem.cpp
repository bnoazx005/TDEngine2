#include "./../../include/ecs/CObjectsSelectionSystem.h"
#include "./../../include/graphics/IRenderer.h"
#include "./../../include/graphics/IGraphicsObjectManager.h"
#include "./../../include/graphics/CRenderQueue.h"
#include "./../../include/graphics/COrthoCamera.h"
#include "./../../include/graphics/CPerspectiveCamera.h"
#include "./../../include/graphics/CStaticMeshContainer.h"
#include "./../../include/graphics/CQuadSprite.h"
#include "./../../include/graphics/IVertexDeclaration.h"
#include "./../../include/graphics/IVertexBuffer.h"
#include "./../../include/graphics/IIndexBuffer.h"
#include "./../../include/graphics/CBaseMaterial.h"
#include "./../../include/core/IResourceManager.h"
#include "./../../include/core/IResourceHandler.h"
#include "./../../include/ecs/CTransform.h"
#include "./../../include/ecs/IWorld.h"
#include "./../../include/ecs/CEntity.h"
#include "./../../include/utils/CFileLogger.h"


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	CObjectsSelectionSystem::CObjectsSelectionSystem() :
		CBaseSystem()
	{
	}

	E_RESULT_CODE CObjectsSelectionSystem::Init(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pGraphicsObjectManager || !pRenderer)
		{
			return RC_INVALID_ARGS;
		}

		mpEditorOnlyRenderQueue = pRenderer->GetRenderQueue(E_RENDER_QUEUE_GROUP::RQG_EDITOR_ONLY);

		mpGraphicsObjectManager = pGraphicsObjectManager;

		mpResourceManager = pRenderer->GetResourceManager();
		
		if (auto newVertDeclResult = pGraphicsObjectManager->CreateVertexDeclaration())
		{
			mpSelectionVertDecl = newVertDeclResult.Get();

			mpSelectionVertDecl->AddElement({ TDEngine2::FT_FLOAT4, 0, TDEngine2::VEST_POSITION });
		}

		E_RESULT_CODE result = RC_OK;

		if ((result = _initSpriteBuffers()) != RC_OK ||
			(result = _initSelectionMaterial()) != RC_OK)
		{
			return result;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CObjectsSelectionSystem::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		mIsInitialized = false;

		result = result | mpSelectionVertDecl->Free();
		result = result | mpSpritesVertexBuffer->Free();
		result = result | mpSpritesIndexBuffer->Free();

		delete this;

		return result;
	}

	void CObjectsSelectionSystem::InjectBindings(IWorld* pWorld)
	{
		std::vector<TEntityId> entities = pWorld->FindEntitiesWithAny<CTransform, CStaticMeshContainer, CQuadSprite>();

		mProcessingEntities.clear();
		std::copy(entities.begin(), entities.end(), std::back_inserter(mProcessingEntities));

		const auto& cameras = pWorld->FindEntitiesWithAny<CPerspectiveCamera, COrthoCamera>();
		mCameraEntityId = !cameras.empty() ? cameras.front() : InvalidEntityId;
	}

	void CObjectsSelectionSystem::Update(IWorld* pWorld, F32 dt)
	{
		// \note Test all objects for visibility
		ICamera* pEditorCameraComponent = _getEditorCamera(pWorld, mCameraEntityId);

		/*!
			foreach (object : objects) {
				if (frustum->Contains(object)) {
					process(object)
				}
			}
		*/

		CEntity* pCurrEntity = nullptr;

		U32 commandIndex = 0;

		for (TEntityId currEntityId : mProcessingEntities)
		{
			pCurrEntity = pWorld->FindEntity(currEntityId);
			if (!pCurrEntity)
			{
				continue;
			}

			if (pCurrEntity->HasComponent<CStaticMeshContainer>())
			{
				_processStaticMeshEntity(commandIndex, mpEditorOnlyRenderQueue, pCurrEntity);
			}
			else if (pCurrEntity->HasComponent<CQuadSprite>())
			{
				_processSpriteEntity(commandIndex, mpEditorOnlyRenderQueue, pCurrEntity);
			}
			
			++commandIndex;
		}

		// add all visible object into the queue

		//ICamera* pCameraComponent = GetValidPtrOrDefault<ICamera*>(mpCameraEntity->GetComponent<CPerspectiveCamera>(), mpCameraEntity->GetComponent<COrthoCamera>());

		//// \note first pass (construct an array of materials)
		//// \note Materials: | {opaque_material_group1}, ..., {opaque_material_groupN} | {transp_material_group1}, ..., {transp_material_groupM} |
		//_collectUsedMaterials(mProcessingEntities, mpResourceManager, mCurrMaterialsArray);

		//auto firstTransparentMatIter = std::find_if(mCurrMaterialsArray.cbegin(), mCurrMaterialsArray.cend(), [](const IMaterial* pCurrMaterial)
		//{
		//	return pCurrMaterial->IsTransparent();
		//});

		//// \note construct commands for opaque geometry
		//std::for_each(mCurrMaterialsArray.cbegin(), firstTransparentMatIter, [this, &pCameraComponent](const IMaterial* pCurrMaterial)
		//{
		//	_populateCommandsBuffer(mProcessingEntities, mpOpaqueRenderGroup, pCurrMaterial, pCameraComponent);
		//});

		//// \note construct commands for transparent geometry
		//std::for_each(firstTransparentMatIter, mCurrMaterialsArray.cend(), [this, &pCameraComponent](const IMaterial* pCurrMaterial)
		//{
		//	_populateCommandsBuffer(mProcessingEntities, mpTransparentRenderGroup, pCurrMaterial, pCameraComponent);
		//});
	}

	E_RESULT_CODE CObjectsSelectionSystem::_initSpriteBuffers()
	{
		static const TVector4 quadSpriteVertices[4]
		{
			{ -0.5f, 0.5f, 0.0f, 1.0f },
			{ 0.5f, 0.5f, 0.0f, 1.0f },
			{ -0.5f, -0.5f, 0.0f, 1.0f },
			{ 0.5f, -0.5f, 0.0f, 1.0f }
		};

		auto spriteVertexBufferResult = mpGraphicsObjectManager->CreateVertexBuffer(BUT_STATIC, sizeof(TVector4) * 4, quadSpriteVertices);
		if (spriteVertexBufferResult.HasError())
		{
			return spriteVertexBufferResult.GetError();
		}

		mpSpritesVertexBuffer = spriteVertexBufferResult.Get();

		static const U16 spriteTriangles[6] { 0, 1, 2, 2, 1, 3 };
		
		auto spriteIndexBufferResult = mpGraphicsObjectManager->CreateIndexBuffer(BUT_STATIC, IFT_INDEX16, sizeof(U16) * 6, spriteTriangles);
		if (spriteIndexBufferResult.HasError())
		{
			return spriteIndexBufferResult.GetError();
		}

		mpSpritesIndexBuffer = spriteIndexBufferResult.Get();

		return RC_OK;
	}

	E_RESULT_CODE CObjectsSelectionSystem::_initSelectionMaterial()
	{
		TMaterialParameters selectionMaterialParams
		{
			"Selection", false,
			{ true, true, E_COMPARISON_FUNC::LESS_EQUAL},
			{ E_CULL_MODE::BACK, false, false, 0.0f, 1.0f, false }
		};

		mpSelectionMaterial = mpResourceManager->Create<CBaseMaterial>("SelectionMaterial.material", selectionMaterialParams);

		return mpSelectionMaterial->IsValid() ? RC_OK : RC_FAIL;
	}

	void CObjectsSelectionSystem::_processStaticMeshEntity(U32 drawIndex, CRenderQueue* pCommandBuffer, CEntity* pEntity)
	{
		CStaticMeshContainer* pStaticMeshContainer = pEntity->GetComponent<CStaticMeshContainer>();
		
		// NOT IMPLEMENTED YET
	}

	void CObjectsSelectionSystem::_processSpriteEntity(U32 drawIndex, CRenderQueue* pCommandBuffer, CEntity* pEntity)
	{
		CQuadSprite* pSpriteComponent = pEntity->GetComponent<CQuadSprite>();
		CTransform* pTransform = pEntity->GetComponent<CTransform>();

		if (TDrawIndexedCommand* pDrawCommand = pCommandBuffer->SubmitDrawCommand<TDrawIndexedCommand>(drawIndex))
		{
			pDrawCommand->mpVertexBuffer           = mpSpritesVertexBuffer;
			pDrawCommand->mpIndexBuffer            = mpSpritesIndexBuffer;
			pDrawCommand->mpMaterialHandler        = mpSelectionMaterial;
			pDrawCommand->mPrimitiveType           = E_PRIMITIVE_TOPOLOGY_TYPE::PTT_TRIANGLE_LIST;
			pDrawCommand->mpVertexDeclaration      = mpSelectionVertDecl;
			pDrawCommand->mObjectData.mModelMatrix = Transpose(pTransform->GetTransform());
			pDrawCommand->mObjectData.mObjectID    = pEntity->GetId();
			pDrawCommand->mStartIndex              = 0;
			pDrawCommand->mStartVertex             = 0;
			pDrawCommand->mNumOfIndices            = 6;
		}
	}

	ICamera* CObjectsSelectionSystem::_getEditorCamera(IWorld* pWorld, TEntityId cameraEntityId)
	{
		TDE2_ASSERT(mCameraEntityId != InvalidEntityId);

		if (CEntity* pCameraEntity = pWorld->FindEntity(cameraEntityId))
		{
			return GetValidPtrOrDefault<ICamera*>(pCameraEntity->GetComponent<CPerspectiveCamera>(), pCameraEntity->GetComponent<COrthoCamera>());
		}

		return nullptr;
	}


	TDE2_API ISystem* CreateObjectsSelectionSystem(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager, E_RESULT_CODE& result)
	{
		CObjectsSelectionSystem* pSystemInstance = new (std::nothrow) CObjectsSelectionSystem();

		if (!pSystemInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pSystemInstance->Init(pRenderer, pGraphicsObjectManager);

		if (result != RC_OK)
		{
			delete pSystemInstance;

			pSystemInstance = nullptr;
		}

		return dynamic_cast<ISystem*>(pSystemInstance);
	}
}

#endif