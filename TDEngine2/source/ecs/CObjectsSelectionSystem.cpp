#include "../../include/ecs/CObjectsSelectionSystem.h"
#include "../../include/graphics/IRenderer.h"
#include "../../include/graphics/IGraphicsObjectManager.h"
#include "../../include/graphics/CRenderQueue.h"
#include "../../include/graphics/COrthoCamera.h"
#include "../../include/graphics/CBaseCamera.h"
#include "../../include/graphics/CPerspectiveCamera.h"
#include "../../include/graphics/CStaticMeshContainer.h"
#include "../../include/graphics/CStaticMesh.h"
#include "../../include/graphics/CSkinnedMeshContainer.h"
#include "../../include/graphics/CSkinnedMesh.h"
#include "../../include/graphics/CQuadSprite.h"
#include "../../include/graphics/IVertexDeclaration.h"
#include "../../include/graphics/IVertexBuffer.h"
#include "../../include/graphics/IIndexBuffer.h"
#include "../../include/graphics/CBaseMaterial.h"
#include "../../include/graphics/UI/CUIElementMeshDataComponent.h"
#include "../../include/graphics/UI/CLayoutElementComponent.h"
#include "../../include/graphics/UI/CCanvasComponent.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/ecs/CTransform.h"
#include "../../include/ecs/IWorld.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/utils/CFileLogger.h"
#include "../../include/editor/ecs/EditorComponents.h"
#include "../../include/editor/CPerfProfiler.h"


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

		mpSelectionVertDecl = nullptr;
		mpSelectionSkinnedVertDecl = nullptr;
		
		if (auto newVertDeclResult = pGraphicsObjectManager->CreateVertexDeclaration())
		{
			mpSelectionVertDecl = newVertDeclResult.Get();

			mpSelectionVertDecl->AddElement({ FT_FLOAT4, 0, VEST_POSITION });
		}

		if (auto newVertDeclResult = pGraphicsObjectManager->CreateVertexDeclaration())
		{
			mpSelectionSkinnedVertDecl = newVertDeclResult.Get();

			mpSelectionSkinnedVertDecl->AddElement({ FT_FLOAT4, 0, VEST_POSITION });
			mpSelectionSkinnedVertDecl->AddElement({ FT_FLOAT4, 0, VEST_JOINT_WEIGHTS });
			mpSelectionSkinnedVertDecl->AddElement({ FT_UINT4, 0, VEST_JOINT_INDICES });
		}

		E_RESULT_CODE result = RC_OK;

		if ((result = _initSpriteBuffers()) != RC_OK ||
			(result = _initSelectionMaterials()) != RC_OK)
		{
			return result;
		}

		mpUIElementsVertexBuffer = nullptr;

		mIsInitialized = true;

		return RC_OK;
	}


	template <typename T>
	static CObjectsSelectionSystem::TSystemContext<T> CreateContext(IWorld* pWorld)
	{
		auto&& entities = pWorld->FindEntitiesWithComponents<CTransform, T>();

		CObjectsSelectionSystem::TSystemContext<T> result;

		for (auto&& currEntityId : entities)
		{
			if (auto pEntity = pWorld->FindEntity(currEntityId))
			{
				result.mpTransforms.push_back(pEntity->GetComponent<CTransform>());
				result.mpRenderables.push_back(pEntity->GetComponent<T>());
				result.mHasSelectedEntityComponent.push_back(pEntity->HasComponent<CSelectedEntityComponent>());
				result.mEntityIds.push_back(currEntityId);
			}
		}

		return std::move(result);
	}


	static CObjectsSelectionSystem::TUIElementsContext CreateUIElementsContext(IWorld* pWorld)
	{
		auto&& entities = pWorld->FindEntitiesWithComponents<CTransform, CLayoutElement, CUIElementMeshData>();

		CObjectsSelectionSystem::TUIElementsContext result;

		for (auto&& currEntityId : entities)
		{
			if (auto pEntity = pWorld->FindEntity(currEntityId))
			{
				result.mpTransforms.push_back(pEntity->GetComponent<CTransform>());
				result.mpRenderables.push_back(pEntity->GetComponent<CUIElementMeshData>());
				result.mLayoutElements.push_back(pEntity->GetComponent<CLayoutElement>());
				result.mHasSelectedEntityComponent.push_back(pEntity->HasComponent<CSelectedEntityComponent>());
				result.mEntityIds.push_back(currEntityId);
			}
		}

		return std::move(result);
	}


	static void ProcessStaticMeshEntity(CObjectsSelectionSystem::TSystemContext<CStaticMeshContainer>& context, TPtr<IResourceManager> pResourceManager, IVertexDeclaration* pVertDecl,
										U32 drawIndex, CRenderQueue* pCommandBuffer, USIZE index, TResourceId materialHandle)
	{
		CStaticMeshContainer* pStaticMeshContainer = context.mpRenderables[index];
		CTransform* pTransform = context.mpTransforms[index];

		// Skip skybox geometry
		// \todo Reimplement this later with CSkyboxComponent
		if (TPtr<IMaterial> pMeshMainMaterial = pResourceManager->GetResource<IMaterial>(pResourceManager->Load<IMaterial>(pStaticMeshContainer->GetMaterialName())))
		{
			if (pMeshMainMaterial->GetGeometrySubGroupTag() == E_GEOMETRY_SUBGROUP_TAGS::SKYBOX)
			{
				return;
			}
		}

		auto&& pMeshResource = pResourceManager->GetResource<IResource>(pResourceManager->Load<IStaticMesh>(pStaticMeshContainer->GetMeshName()));
		if (E_RESOURCE_STATE_TYPE::RST_LOADED != pMeshResource->GetState())
		{
			return;
		}

		if (TPtr<IStaticMesh> pStaticMeshResource = DynamicPtrCast<IStaticMesh>(pMeshResource))
		{
			if (TDrawIndexedCommand* pDrawCommand = pCommandBuffer->SubmitDrawCommand<TDrawIndexedCommand>(drawIndex))
			{
				pDrawCommand->mpVertexBuffer = pStaticMeshResource->GetPositionOnlyVertexBuffer();
				pDrawCommand->mpIndexBuffer = pStaticMeshResource->GetSharedIndexBuffer();
				pDrawCommand->mMaterialHandle = materialHandle;
				pDrawCommand->mPrimitiveType = E_PRIMITIVE_TOPOLOGY_TYPE::PTT_TRIANGLE_LIST;
				pDrawCommand->mpVertexDeclaration = pVertDecl;
				pDrawCommand->mObjectData.mModelMatrix = Transpose(pTransform->GetLocalToWorldTransform());
				pDrawCommand->mObjectData.mObjectID = static_cast<U32>(context.mEntityIds[index]);
				pDrawCommand->mStartIndex = 0;
				pDrawCommand->mStartVertex = 0;
				pDrawCommand->mNumOfIndices = pStaticMeshResource->GetFacesCount() * 3;
			}
		}
	}


	static void ProcessSkinnedMeshEntity(CObjectsSelectionSystem::TSystemContext<CSkinnedMeshContainer>& context, TPtr<IResourceManager> pResourceManager, IVertexDeclaration* pVertDecl, 
										U32 drawIndex, CRenderQueue* pCommandBuffer, USIZE index, TResourceId materialHandle)
	{
		CSkinnedMeshContainer* pSkinnedMeshContainer = context.mpRenderables[index];
		CTransform* pTransform = context.mpTransforms[index];

		auto&& pMeshResource = pResourceManager->GetResource<IResource>(pResourceManager->Load<ISkinnedMesh>(pSkinnedMeshContainer->GetMeshName()));
		if (E_RESOURCE_STATE_TYPE::RST_LOADED != pMeshResource->GetState())
		{
			return;
		}

		if (TPtr<ISkinnedMesh> pSkinnedMeshResource = DynamicPtrCast<ISkinnedMesh>(pMeshResource))
		{
			const auto& currAnimationPose = pSkinnedMeshContainer->GetCurrentAnimationPose();
			const U32 jointsCount = static_cast<U32>(currAnimationPose.size());

			if (TPtr<IMaterial> pMaterial = pResourceManager->GetResource<IMaterial>(materialHandle))
			{
				pMaterial->SetVariableForInstance(DefaultMaterialInstanceId, CSkinnedMeshContainer::mJointsArrayUniformVariableId, &currAnimationPose.front(), static_cast<U32>(sizeof(TMatrix4) * currAnimationPose.size()));
				pMaterial->SetVariableForInstance(DefaultMaterialInstanceId, CSkinnedMeshContainer::mJointsCountUniformVariableId, &jointsCount, sizeof(U32));
			}

			if (TDrawIndexedCommand* pDrawCommand = pCommandBuffer->SubmitDrawCommand<TDrawIndexedCommand>(drawIndex))
			{
				pDrawCommand->mpVertexBuffer = pSkinnedMeshResource->GetPositionOnlyVertexBuffer();
				pDrawCommand->mpIndexBuffer = pSkinnedMeshResource->GetSharedIndexBuffer();
				pDrawCommand->mMaterialHandle = materialHandle;
				pDrawCommand->mPrimitiveType = E_PRIMITIVE_TOPOLOGY_TYPE::PTT_TRIANGLE_LIST;
				pDrawCommand->mpVertexDeclaration = pVertDecl;
				pDrawCommand->mObjectData.mModelMatrix = Transpose(pTransform->GetLocalToWorldTransform());
				pDrawCommand->mObjectData.mObjectID = static_cast<U32>(context.mEntityIds[index]);
				pDrawCommand->mStartIndex = 0;
				pDrawCommand->mStartVertex = 0;
				pDrawCommand->mNumOfIndices = static_cast<U32>(pSkinnedMeshResource->GetIndices().size());
			}
		}
	}


	static void ProcessSpriteEntity(CObjectsSelectionSystem::TSystemContext<CQuadSprite>& context, TPtr<IResourceManager> pResourceManager, IVertexDeclaration* pVertDecl, 
									IVertexBuffer* pVertBuffer, IIndexBuffer* pIndexBuffer, U32 drawIndex, CRenderQueue* pCommandBuffer, USIZE index, TResourceId materialHandle)
	{
		CQuadSprite* pSpriteComponent = context.mpRenderables[index];
		CTransform* pTransform = context.mpTransforms[index];

		if (TDrawIndexedCommand* pDrawCommand = pCommandBuffer->SubmitDrawCommand<TDrawIndexedCommand>(drawIndex))
		{
			pDrawCommand->mpVertexBuffer = pVertBuffer;
			pDrawCommand->mpIndexBuffer = pIndexBuffer;
			pDrawCommand->mMaterialHandle = materialHandle;
			pDrawCommand->mPrimitiveType = E_PRIMITIVE_TOPOLOGY_TYPE::PTT_TRIANGLE_LIST;
			pDrawCommand->mpVertexDeclaration = pVertDecl;
			pDrawCommand->mObjectData.mModelMatrix = Transpose(pTransform->GetLocalToWorldTransform());
			pDrawCommand->mObjectData.mObjectID = static_cast<U32>(context.mEntityIds[index]);
			pDrawCommand->mStartIndex = 0;
			pDrawCommand->mStartVertex = 0;
			pDrawCommand->mNumOfIndices = 6;
		}
	}


	static void ProcessUIElementEntity(IWorld* pWorld, std::vector<TVector4>& vertsOutput, CObjectsSelectionSystem::TUIElementsContext& context, TPtr<IResourceManager> pResourceManager, 
									IVertexDeclaration* pVertDecl, IVertexBuffer*& pVertBuffer, IIndexBuffer*& pIndexBuffer, U32 drawIndex, 
									CRenderQueue* pCommandBuffer, USIZE index, TResourceId materialHandle, USIZE& vertexBufferOffset)
	{
		CUIElementMeshData* pUIMeshData = context.mpRenderables[index];
		CLayoutElement* pLayoutElement  = context.mLayoutElements[index];
		CTransform* pTransform          = context.mpTransforms[index];

		const auto& minBound = pUIMeshData->GetMinBound();
		const auto& maxBound = pUIMeshData->GetMaxBound();

		vertsOutput.push_back(TVector4(minBound.x, minBound.y, 1.0f, 1.0f));
		vertsOutput.push_back(TVector4(maxBound.x, minBound.y, 1.0f, 1.0f));
		vertsOutput.push_back(TVector4(minBound.x, maxBound.y, 1.0f, 1.0f));
		vertsOutput.push_back(TVector4(maxBound.x, maxBound.y, 1.0f, 1.0f));

		CEntity* pCanvasEntity = pWorld->FindEntity(pLayoutElement->GetOwnerCanvasId());
		CCanvas* pCanvasData = pCanvasEntity ? pCanvasEntity->GetComponent<CCanvas>() : nullptr;

		if (TDrawIndexedCommand* pDrawCommand = pCommandBuffer->SubmitDrawCommand<TDrawIndexedCommand>(drawIndex))
		{
			pDrawCommand->mpVertexBuffer = pVertBuffer;
			pDrawCommand->mpIndexBuffer = pIndexBuffer;
			pDrawCommand->mMaterialHandle = materialHandle;
			pDrawCommand->mPrimitiveType = E_PRIMITIVE_TOPOLOGY_TYPE::PTT_TRIANGLE_LIST;
			pDrawCommand->mpVertexDeclaration = pVertDecl;
			pDrawCommand->mObjectData.mModelMatrix = IdentityMatrix4;
			pDrawCommand->mObjectData.mObjectID = static_cast<U32>(context.mEntityIds[index]);
			pDrawCommand->mStartIndex = 0;
			pDrawCommand->mStartVertex = static_cast<U32>(vertexBufferOffset);
			pDrawCommand->mNumOfIndices = 6;

			auto&& rect = pLayoutElement->GetWorldRect();
			auto pivot = rect.GetLeftBottom() + pLayoutElement->GetPivot() * rect.GetSizes();

			auto pivotTranslation = TranslationMatrix(TVector3{ -pivot.x, -pivot.y, 0.0f });
			TMatrix4 localObjectTransform = Inverse(pivotTranslation) * RotationMatrix(pTransform->GetRotation()) * ScaleMatrix(pTransform->GetScale()) * pivotTranslation;

			pDrawCommand->mObjectData.mModelMatrix = Transpose((pCanvasData ? pCanvasData->GetProjMatrix() : IdentityMatrix4) * localObjectTransform);
		}

		vertexBufferOffset += 4;
	}


	void CObjectsSelectionSystem::InjectBindings(IWorld* pWorld)
	{
		mStaticMeshesContext  = CreateContext<CStaticMeshContainer>(pWorld);
		mSkinnedMeshesContext = CreateContext<CSkinnedMeshContainer>(pWorld);
		mSpritesContext       = CreateContext<CQuadSprite>(pWorld);
		mUIElementsContext    = CreateUIElementsContext(pWorld);

		const auto& cameras = pWorld->FindEntitiesWithAny<CEditorCamera>();
		mCameraEntityId = !cameras.empty() ? cameras.front() : TEntityId::Invalid;
	}

	void CObjectsSelectionSystem::Update(IWorld* pWorld, F32 dt)
	{
		TDE2_PROFILER_SCOPE("CObjectsSelectionSystem::Update");

		// \note Test all objects for visibility
		ICamera* pEditorCameraComponent = _getEditorCamera(pWorld, mCameraEntityId);

		/*!
			foreach (object : objects) {
				if (frustum->Contains(object)) {
					process(object)
				}
			}
		*/

		U32 commandIndex = 0;

		/// \note Static meshes
		for (USIZE i = 0; i < static_cast<U32>(mStaticMeshesContext.mpRenderables.size()); ++i)
		{
			ProcessStaticMeshEntity(mStaticMeshesContext, mpResourceManager, mpSelectionVertDecl, commandIndex++, mpEditorOnlyRenderQueue, i, mSelectionMaterialHandle);

			if (mStaticMeshesContext.mHasSelectedEntityComponent[i])
			{
				ProcessStaticMeshEntity(mStaticMeshesContext, mpResourceManager, mpSelectionVertDecl, static_cast<U32>(E_GEOMETRY_SUBGROUP_TAGS::SELECTION_OUTLINE),
										mpDebugRenderQueue, i, mSelectionOutlineMaterialHandle);
			}
		}

		/// \note Skinned meshes
		for (USIZE i = 0; i < static_cast<U32>(mSkinnedMeshesContext.mpRenderables.size()); ++i)
		{
			ProcessSkinnedMeshEntity(mSkinnedMeshesContext, mpResourceManager, mpSelectionSkinnedVertDecl, commandIndex++, mpEditorOnlyRenderQueue, i, mSelectionSkinnedMaterialHandle);

			if (mSkinnedMeshesContext.mHasSelectedEntityComponent[i])
			{
				ProcessSkinnedMeshEntity(mSkinnedMeshesContext, mpResourceManager, mpSelectionSkinnedVertDecl, static_cast<U32>(E_GEOMETRY_SUBGROUP_TAGS::SELECTION_OUTLINE),
					mpDebugRenderQueue, i, mSelectionSkinnedOutlineMaterialHandle);
			}
		}

		/// \note Quad sprites
		for (USIZE i = 0; i < static_cast<U32>(mSpritesContext.mpRenderables.size()); ++i)
		{
			ProcessSpriteEntity(mSpritesContext, mpResourceManager, mpSelectionVertDecl, mpSpritesVertexBuffer, mpSpritesIndexBuffer,
				commandIndex++, mpEditorOnlyRenderQueue, i, mSelectionMaterialHandle);

			if (mSpritesContext.mHasSelectedEntityComponent[i])
			{
				ProcessSpriteEntity(mSpritesContext, mpResourceManager, mpSelectionVertDecl, mpSpritesVertexBuffer, mpSpritesIndexBuffer,
					static_cast<U32>(E_GEOMETRY_SUBGROUP_TAGS::SELECTION_OUTLINE), mpDebugRenderQueue, i, mSelectionOutlineMaterialHandle);
			}
		}

		/// \note UI elements 
		{
			static constexpr USIZE MaxVerticesCount = 1024;
			static constexpr USIZE VertexBufferDefaultSize = sizeof(TVector4) * MaxVerticesCount;

			/// \note If there is no a created vertex buffer or we go out of space extend/create it
			if (!mpUIElementsVertexBuffer || (mpUIElementsVertexBuffer && mpUIElementsVertexBuffer->GetUsedSize() >= mpUIElementsVertexBuffer->GetSize()))
			{
				if (!mpUIElementsVertexBuffer)
				{
					mpUIElementsVertexBuffer = mpGraphicsObjectManager->CreateVertexBuffer(BUT_DYNAMIC, VertexBufferDefaultSize, nullptr).Get();
				}

				mpUIElementsVertexBuffer = mpGraphicsObjectManager->CreateVertexBuffer(BUT_DYNAMIC, mpUIElementsVertexBuffer->GetSize() + VertexBufferDefaultSize, nullptr).Get();
			}

			mUIElementsVertexBufferCurrOffset = 0;

			static std::vector<TVector4> uiElementsVerts;

			uiElementsVerts.clear();

			for (USIZE i = 0; i < static_cast<U32>(mUIElementsContext.mpRenderables.size()); ++i)
			{
				/// \note Use sprites' index buffer because ui elements are just quads too
				ProcessUIElementEntity(pWorld, uiElementsVerts, mUIElementsContext, mpResourceManager, mpSelectionVertDecl, mpUIElementsVertexBuffer, mpSpritesIndexBuffer,
									commandIndex++, mpEditorOnlyRenderQueue, i, mSelectionUIMaterialHandle, mUIElementsVertexBufferCurrOffset);
			}

			E_RESULT_CODE result = mpUIElementsVertexBuffer->Map(BMT_WRITE_DISCARD);
			TDE2_ASSERT(RC_OK == result);

			mpUIElementsVertexBuffer->Write(uiElementsVerts.data(), sizeof(TVector4) * uiElementsVerts.size());
			mpUIElementsVertexBuffer->Unmap();
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


	static TMaterialParameters CreateSelectionMaterialParams(const std::string& materialName)
	{
		const TMaterialParameters selectionMaterialParams
		{
			materialName, false,
			TDepthStencilStateDesc { true, true, E_COMPARISON_FUNC::LESS_EQUAL},
			TRasterizerStateDesc { E_CULL_MODE::NONE, false, false, 0.0f, 1.0f, false }
		};

		return selectionMaterialParams;
	}


	static TMaterialParameters CreateSelectionOutlineMaterialParams(const std::string& materialName)
	{
		const TMaterialParameters selectionOutlineMaterialParams
		{
			materialName, true,
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

		return selectionOutlineMaterialParams;
	}


	E_RESULT_CODE CObjectsSelectionSystem::_initSelectionMaterials()
	{
		const static TMaterialParameters selectionMaterialParams        = CreateSelectionMaterialParams("Selection");
		const static TMaterialParameters selectionSkinnedMaterialParams = CreateSelectionMaterialParams("SelectionSkinned");
		const static TMaterialParameters selectionUIMaterialParams      = CreateSelectionMaterialParams("SelectionUI");

		const static TMaterialParameters selectionOutlineMaterialParams        = CreateSelectionOutlineMaterialParams("SelectionOutline");
		const static TMaterialParameters selectionSkinnedOutlineMaterialParams = CreateSelectionOutlineMaterialParams("SelectionSkinnedOutline");

		mSelectionMaterialHandle               = mpResourceManager->Create<IMaterial>("SelectionMaterial.material", selectionMaterialParams);
		mSelectionSkinnedMaterialHandle        = mpResourceManager->Create<IMaterial>("SelectionSkinnedMaterial.material", selectionSkinnedMaterialParams);
		mSelectionUIMaterialHandle             = mpResourceManager->Create<IMaterial>("SelectionUIMaterial.material", selectionUIMaterialParams);
		mSelectionOutlineMaterialHandle        = mpResourceManager->Create<IMaterial>("SelectionOutlineMaterial.material", selectionOutlineMaterialParams);
		mSelectionSkinnedOutlineMaterialHandle = mpResourceManager->Create<IMaterial>("SelectionSkinnedOutlineMaterial.material", selectionSkinnedOutlineMaterialParams);

		return (mSelectionMaterialHandle != TResourceId::Invalid && mSelectionOutlineMaterialHandle != TResourceId::Invalid) ? RC_OK : RC_FAIL;
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