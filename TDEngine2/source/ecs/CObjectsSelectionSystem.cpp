#include "../../include/ecs/CObjectsSelectionSystem.h"
#include "../../include/graphics/IRenderer.h"
#include "../../include/graphics/IGraphicsObjectManager.h"
#include "../../include/graphics/CRenderQueue.h"
#include "../../include/graphics/COrthoCamera.h"
#include "../../include/graphics/CPerspectiveCamera.h"
#include "../../include/graphics/CStaticMeshContainer.h"
#include "../../include/graphics/CStaticMesh.h"
#include "../../include/graphics/CQuadSprite.h"
#include "../../include/graphics/IVertexDeclaration.h"
#include "../../include/graphics/IVertexBuffer.h"
#include "../../include/graphics/IIndexBuffer.h"
#include "../../include/graphics/CBaseMaterial.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/ecs/CTransform.h"
#include "../../include/ecs/IWorld.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/utils/CFileLogger.h"
#include "../../include/editor/ecs/EditorComponents.h"


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
		mpDebugRenderQueue      = pRenderer->GetRenderQueue(E_RENDER_QUEUE_GROUP::RQG_DEBUG);

		mpGraphicsObjectManager = pGraphicsObjectManager;

		mpResourceManager = pRenderer->GetResourceManager();
		
		if (auto newVertDeclResult = pGraphicsObjectManager->CreateVertexDeclaration())
		{
			mpSelectionVertDecl = newVertDeclResult.Get();

			mpSelectionVertDecl->AddElement({ TDEngine2::FT_FLOAT4, 0, TDEngine2::VEST_POSITION });
		}

		E_RESULT_CODE result = RC_OK;

		if ((result = _initSpriteBuffers()) != RC_OK ||
			(result = _initSelectionMaterials()) != RC_OK)
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

		//result = result | mpSelectionVertDecl->Free();
		//result = result | mpSpritesVertexBuffer->Free();
		//result = result | mpSpritesIndexBuffer->Free();

		delete this;

		return result;
	}

	void CObjectsSelectionSystem::InjectBindings(IWorld* pWorld)
	{
		std::vector<TEntityId> entities = pWorld->FindEntitiesWithAny<CTransform, CStaticMeshContainer, CQuadSprite>();

		mProcessingEntities.clear();
		std::copy(entities.begin(), entities.end(), std::back_inserter(mProcessingEntities));

		const auto& cameras = pWorld->FindEntitiesWithAny<CPerspectiveCamera, COrthoCamera>();
		mCameraEntityId = !cameras.empty() ? cameras.front() : TEntityId::Invalid;
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
				_processStaticMeshEntity(commandIndex, mpEditorOnlyRenderQueue, pCurrEntity, mSelectionMaterialHandle);

				if (pCurrEntity->HasComponent<CSelectedEntityComponent>())
				{
					_processStaticMeshEntity(static_cast<U32>(E_GEOMETRY_SUBGROUP_TAGS::SELECTION_OUTLINE), mpDebugRenderQueue, pCurrEntity, mSelectionOutlineMaterialHandle);
				}
			}
			else if (pCurrEntity->HasComponent<CQuadSprite>())
			{
				_processSpriteEntity(commandIndex, mpEditorOnlyRenderQueue, pCurrEntity, mSelectionMaterialHandle);

				if (pCurrEntity->HasComponent<CSelectedEntityComponent>())
				{
					_processSpriteEntity(static_cast<U32>(E_GEOMETRY_SUBGROUP_TAGS::SELECTION_OUTLINE), mpDebugRenderQueue, pCurrEntity, mSelectionOutlineMaterialHandle);
				}
			}
			
			++commandIndex;
		}
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

	E_RESULT_CODE CObjectsSelectionSystem::_initSelectionMaterials()
	{
		const static TMaterialParameters selectionMaterialParams
		{
			"Selection", false,
			TDepthStencilStateDesc { true, true, E_COMPARISON_FUNC::LESS_EQUAL},
			TRasterizerStateDesc { E_CULL_MODE::NONE, false, false, 0.0f, 1.0f, false }
		};

		const static TMaterialParameters selectionOutlineMaterialParams
		{
			"SelectionOutline", true,
			TDepthStencilStateDesc { false, false, E_COMPARISON_FUNC::LESS_EQUAL},
			TRasterizerStateDesc { E_CULL_MODE::NONE, true, false, 0.0f, 1.0f, false },
			TBlendStateDesc 
			{ 
				true,
				E_BLEND_FACTOR_VALUE::SOURCE_ALPHA, 
				E_BLEND_FACTOR_VALUE::ONE_MINUS_SOURCE_ALPHA, 
				E_BLEND_OP_TYPE::ADD, 
				E_BLEND_FACTOR_VALUE::ONE_MINUS_SOURCE_ALPHA, 
				E_BLEND_FACTOR_VALUE::ZERO 
			}
		};
		
		mSelectionMaterialHandle        = mpResourceManager->Create<CBaseMaterial>("SelectionMaterial.material", selectionMaterialParams);
		mSelectionOutlineMaterialHandle = mpResourceManager->Create<CBaseMaterial>("SelectionOutlineMaterial.material", selectionOutlineMaterialParams);

		return (mSelectionMaterialHandle != TResourceId::Invalid && mSelectionOutlineMaterialHandle != TResourceId::Invalid) ? RC_OK : RC_FAIL;
	}

	void CObjectsSelectionSystem::_processStaticMeshEntity(U32 drawIndex, CRenderQueue* pCommandBuffer, CEntity* pEntity, TResourceId materialHandle)
	{
		CStaticMeshContainer* pStaticMeshContainer = pEntity->GetComponent<CStaticMeshContainer>();
		CTransform* pTransform = pEntity->GetComponent<CTransform>();
		
		// Skip skybox geometry
		// \todo Reimplement this later with CSkyboxComponent
		if (IMaterial* pMeshMainMaterial = dynamic_cast<IMaterial*>(mpResourceManager->GetResource(mpResourceManager->Load<CBaseMaterial>(pStaticMeshContainer->GetMaterialName()))))
		{
			if (pMeshMainMaterial->GetGeometrySubGroupTag() == E_GEOMETRY_SUBGROUP_TAGS::SKYBOX)
			{
				return;
			}
		}

		if (IStaticMesh* pStaticMeshResource = dynamic_cast<IStaticMesh*>(mpResourceManager->GetResource(mpResourceManager->Load<CStaticMesh>(pStaticMeshContainer->GetMeshName()))))
		{
			if (TDrawIndexedCommand* pDrawCommand = pCommandBuffer->SubmitDrawCommand<TDrawIndexedCommand>(drawIndex))
			{
				pDrawCommand->mpVertexBuffer           = pStaticMeshResource->GetPositionOnlyVertexBuffer();
				pDrawCommand->mpIndexBuffer            = pStaticMeshResource->GetSharedIndexBuffer();
				pDrawCommand->mMaterialHandle          = materialHandle;
				pDrawCommand->mPrimitiveType           = E_PRIMITIVE_TOPOLOGY_TYPE::PTT_TRIANGLE_LIST;
				pDrawCommand->mpVertexDeclaration      = mpSelectionVertDecl;
				pDrawCommand->mObjectData.mModelMatrix = Transpose(pTransform->GetLocalToWorldTransform());
				pDrawCommand->mObjectData.mObjectID    = static_cast<U32>(pEntity->GetId());
				pDrawCommand->mStartIndex              = 0;
				pDrawCommand->mStartVertex             = 0;
				pDrawCommand->mNumOfIndices            = pStaticMeshResource->GetFacesCount() * 3;
			}
		}
	}

	void CObjectsSelectionSystem::_processSpriteEntity(U32 drawIndex, CRenderQueue* pCommandBuffer, CEntity* pEntity, TResourceId materialHandle)
	{
		CQuadSprite* pSpriteComponent = pEntity->GetComponent<CQuadSprite>();
		CTransform* pTransform = pEntity->GetComponent<CTransform>();

		if (TDrawIndexedCommand* pDrawCommand = pCommandBuffer->SubmitDrawCommand<TDrawIndexedCommand>(drawIndex))
		{
			pDrawCommand->mpVertexBuffer           = mpSpritesVertexBuffer;
			pDrawCommand->mpIndexBuffer            = mpSpritesIndexBuffer;
			pDrawCommand->mMaterialHandle          = materialHandle;
			pDrawCommand->mPrimitiveType           = E_PRIMITIVE_TOPOLOGY_TYPE::PTT_TRIANGLE_LIST;
			pDrawCommand->mpVertexDeclaration      = mpSelectionVertDecl;
			pDrawCommand->mObjectData.mModelMatrix = Transpose(pTransform->GetLocalToWorldTransform());
			pDrawCommand->mObjectData.mObjectID    = static_cast<U32>(pEntity->GetId());
			pDrawCommand->mStartIndex              = 0;
			pDrawCommand->mStartVertex             = 0;
			pDrawCommand->mNumOfIndices            = 6;
		}
	}

	ICamera* CObjectsSelectionSystem::_getEditorCamera(IWorld* pWorld, TEntityId cameraEntityId)
	{
		TDE2_ASSERT(mCameraEntityId != TEntityId::Invalid);

		if (CEntity* pCameraEntity = pWorld->FindEntity(cameraEntityId))
		{
			return GetValidPtrOrDefault<ICamera*>(pCameraEntity->GetComponent<CPerspectiveCamera>(), pCameraEntity->GetComponent<COrthoCamera>());
		}

		return nullptr;
	}


	TDE2_API ISystem* CreateObjectsSelectionSystem(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem, CObjectsSelectionSystem, result, pRenderer, pGraphicsObjectManager);
	}
}

#endif