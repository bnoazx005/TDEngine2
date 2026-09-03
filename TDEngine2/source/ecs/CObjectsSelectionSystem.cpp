#include "../../include/ecs/CObjectsSelectionSystem.h"
#include "../../include/graphics/IRenderer.h"
#include "../../include/graphics/IGraphicsObjectManager.h"
#include "../../include/graphics/CRenderQueue.h"
#include "../../include/graphics/CCamera.h"
#include "../../include/graphics/CStaticMeshContainer.h"
#include "../../include/graphics/CStaticMesh.h"
#include "../../include/graphics/CSkinnedMeshContainer.h"
#include "../../include/graphics/CSkinnedMesh.h"
#include "../../include/graphics/CQuadSprite.h"
#include "../../include/graphics/IVertexDeclaration.h"
#include "../../include/graphics/CBaseMaterial.h"
#include "../../include/graphics/UI/CUIElementMeshDataComponent.h"
#include "../../include/graphics/UI/CLayoutElementComponent.h"
#include "../../include/graphics/UI/CCanvasComponent.h"
#include "../../include/graphics/CFramePacketsStorage.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/ecs/CTransform.h"
#include "../../include/ecs/IWorld.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/ecs/CSystemManager.h"
#include "../../include/ecs/CCameraSystem.h"
#include "../../include/ecs/CStaticMeshRendererSystem.h"
#include "../../include/ecs/CSkinnedMeshRendererSystem.h"
#include "../../include/ecs/CSpriteRendererSystem.h"
#include "../../include/ecs/CUIElementsProcessSystem.h"
#include "../../include/ecs/components/CBoundsComponent.h"
#include "../../include/utils/CFileLogger.h"
#include "../../include/editor/ecs/EditorComponents.h"
#include "../../include/editor/CPerfProfiler.h"


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	CObjectsSelectionSystem::CObjectsSelectionSystem() :
		CBaseSystem(),
		mSpritesVertexBufferHandle(TBufferHandleId::Invalid),
		mSpritesIndexBufferHandle(TBufferHandleId::Invalid),
		mSelectionMaterialHandle(TResourceId::Invalid),
		mSelectionSkinnedMaterialHandle(TResourceId::Invalid),
		mSelectionUIMaterialHandle(TResourceId::Invalid),
		mSelectionOutlineMaterialHandle(TResourceId::Invalid),
		mSelectionSkinnedOutlineMaterialHandle(TResourceId::Invalid)
	{
	}

	E_RESULT_CODE CObjectsSelectionSystem::Init(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager)
	{
		TDE2_PROFILER_SCOPE("CObjectsSelectionSystem::Init");

		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pGraphicsObjectManager || !pRenderer)
		{
			return RC_INVALID_ARGS;
		}

		mpGraphicsObjectManager = pGraphicsObjectManager;
		mpResourceManager       = pRenderer->GetResourceManager();

		E_RESULT_CODE result = RC_OK;

		if ((result = _initSpriteBuffers()) != RC_OK ||
			(result = _initSelectionMaterials()) != RC_OK)
		{
			return result;
		}

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
				result.mpTransforms.push_back(pEntity->template GetComponent<CTransform>());
				result.mpRenderables.push_back(pEntity->template GetComponent<T>());
				result.mHasSelectedEntityComponent.push_back(pEntity->template HasComponent<CSelectedEntityComponent>());
				result.mpBounds.push_back(pEntity->template GetComponent<CBoundsComponent>());
				result.mEntityIds.push_back(currEntityId);
			}
		}

		return result;
	}


	static CObjectsSelectionSystem::TUIElementsContext CreateUIElementsContext(IWorld* pWorld)
	{
		auto&& entities = pWorld->FindEntitiesWithComponents<CTransform, CLayoutElement, CUIElementMeshData>();

		CObjectsSelectionSystem::TUIElementsContext result;

		for (auto&& currEntityId : entities)
		{
			if (auto pEntity = pWorld->FindEntity(currEntityId))
			{
				result.mpTransforms.push_back(pEntity->template GetComponent<CTransform>());
				result.mpRenderables.push_back(pEntity->template GetComponent<CUIElementMeshData>());
				result.mLayoutElements.push_back(pEntity->template GetComponent<CLayoutElement>());
				result.mHasSelectedEntityComponent.push_back(pEntity->template HasComponent<CSelectedEntityComponent>());
				result.mVisibleFlags.push_back(!pEntity->template HasComponent<CCanvas>());
				result.mEntityIds.push_back(currEntityId);
			}
		}

		return result;
	}


	static void ProcessStaticMeshEntity(CObjectsSelectionSystem::TSystemContext<CStaticMeshContainer>& context, TPtr<IResourceManager> pResourceManager,
										U32 drawIndex, USIZE index, TResourceId materialHandle, CObjectsSelectionSystem::TMeshDrawCommands& drawCommands)
	{
		CStaticMeshContainer* pStaticMeshContainer = context.mpRenderables[index];
		CTransform* pTransform = context.mpTransforms[index];

		auto&& materialId = pStaticMeshContainer->GetMaterialName();
		if (materialId.empty())
		{
			return;
		}

		// Skip skybox geometry
		// \todo Reimplement this later with CSkyboxComponent
		if (TPtr<IMaterial> pMeshMainMaterial = pResourceManager->GetResource<IMaterial>(pResourceManager->Load<IMaterial>(materialId)))
		{
			if (pMeshMainMaterial->GetGeometrySubGroupTag() == E_GEOMETRY_SUBGROUP_TAGS::SKYBOX)
			{
				return;
			}
		}

		auto&& pMeshResource = pResourceManager->GetResource<IResource>(pResourceManager->Load<IStaticMesh>(pStaticMeshContainer->GetMeshName()));
		if (!pMeshResource || E_RESOURCE_STATE_TYPE::RST_LOADED != pMeshResource->GetState())
		{
			return;
		}

		auto&& subMeshInfo = pStaticMeshContainer->GetSubMeshInfo();

		if (TPtr<IStaticMesh> pStaticMeshResource = DynamicPtrCast<IStaticMesh>(pMeshResource))
		{
			drawCommands.emplace_back(CObjectsSelectionSystem::TMeshDrawEntry
				{
					Transpose(pTransform->GetLocalToWorldTransform()),
					pStaticMeshResource->GetVertexBufferForStream(E_VERTEX_STREAM_TYPE::POSITIONS),
					pStaticMeshResource->GetSharedIndexBuffer(),
					drawIndex,
					subMeshInfo.mStartIndex,
					subMeshInfo.mIndicesCount,
					static_cast<U32>(context.mEntityIds[index])
				});
		}
	}


	static void ProcessSkinnedMeshEntity(CObjectsSelectionSystem::TSystemContext<CSkinnedMeshContainer>& context, TPtr<IResourceManager> pResourceManager,
										U32 drawIndex, USIZE index, TResourceId materialHandle, CObjectsSelectionSystem::TMeshDrawCommands& drawCommands)
	{
		CSkinnedMeshContainer* pSkinnedMeshContainer = context.mpRenderables[index];
		CTransform* pTransform = context.mpTransforms[index];

		auto&& pMeshResource = pResourceManager->GetResource<IResource>(pResourceManager->Load<ISkinnedMesh>(pSkinnedMeshContainer->GetMeshName()));
		if (!pMeshResource || E_RESOURCE_STATE_TYPE::RST_LOADED != pMeshResource->GetState())
		{
			return;
		}

		if (TPtr<ISkinnedMesh> pSkinnedMeshResource = DynamicPtrCast<ISkinnedMesh>(pMeshResource))
		{
			const auto& currAnimationPose = pSkinnedMeshContainer->GetCurrentAnimationPose();
			const U32 jointsCount = static_cast<U32>(currAnimationPose.size());

			if (!jointsCount)
			{
				return;
			}

			if (TPtr<IMaterial> pMaterial = pResourceManager->GetResource<IMaterial>(materialHandle))
			{
				pMaterial->SetVariableForInstance(DefaultMaterialInstanceId, CSkinnedMeshContainer::mJointsArrayUniformVariableId, &currAnimationPose.front(), static_cast<U32>(sizeof(TMatrix4) * currAnimationPose.size()));
				pMaterial->SetVariableForInstance(DefaultMaterialInstanceId, CSkinnedMeshContainer::mJointsCountUniformVariableId, &jointsCount, sizeof(U32));
			}

			auto&& subMeshInfo = pSkinnedMeshContainer->GetSubMeshInfo();

			drawCommands.emplace_back(CObjectsSelectionSystem::TMeshDrawEntry
				{
					Transpose(pTransform->GetLocalToWorldTransform()),
					pSkinnedMeshResource->GetVertexBufferForStream(E_VERTEX_STREAM_TYPE::POSITIONS),
					pSkinnedMeshResource->GetSharedIndexBuffer(),
					drawIndex,
					subMeshInfo.mStartIndex,
					subMeshInfo.mIndicesCount,
					static_cast<U32>(context.mEntityIds[index])
				});
		}
	}


	static void ProcessSpriteEntity(CObjectsSelectionSystem::TSystemContext<CQuadSprite>& context, U32 drawIndex, USIZE index, CObjectsSelectionSystem::TSpriteDrawCommands& drawCommands)
	{
		CQuadSprite* pSpriteComponent = context.mpRenderables[index];
		CTransform* pTransform = context.mpTransforms[index];

		drawCommands.emplace_back(CObjectsSelectionSystem::TSpriteDrawEntry
			{
				Transpose(pTransform->GetLocalToWorldTransform()),
				drawIndex,
				static_cast<U32>(context.mEntityIds[index])
			});
	}


	static void ProcessUIElementEntity(IWorld* pWorld, std::vector<TVector4>& vertsOutput, CObjectsSelectionSystem::TUIElementsContext& context, TPtr<IResourceManager> pResourceManager, 
									U32 drawIndex, USIZE index, USIZE& vertexBufferOffset, CObjectsSelectionSystem::TUIElementDrawCommands& drawCommands)
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

		auto&& rect = pLayoutElement->GetWorldRect();
		auto pivot = rect.GetLeftBottom() + pLayoutElement->GetPivot() * rect.GetSizes();

		auto pivotTranslation = TranslationMatrix(TVector3{ -pivot.x, -pivot.y, 0.0f });
		TMatrix4 localObjectTransform = Inverse(pivotTranslation) * RotationMatrix(pTransform->GetRotation()) * ScaleMatrix(pTransform->GetScale()) * pivotTranslation;

		drawCommands.emplace_back(CObjectsSelectionSystem::TUIElementDrawEntry
			{
				Transpose((pCanvasData ? pCanvasData->GetProjMatrix() : IdentityMatrix4) * localObjectTransform),
				drawIndex,
				static_cast<U32>(context.mEntityIds[index]),
				static_cast<U32>(vertexBufferOffset)
			});

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

		mMeshesCommands.clear();
		mSelectedMeshesCommands.clear();
		mSpritesCommands.clear();
		mSelectedSpritesCommands.clear();
		mUIElementsCommands.clear();

		U32 commandIndex = 0;

		/// \note Static meshes
		for (USIZE i = 0; i < static_cast<U32>(mStaticMeshesContext.mpRenderables.size()); ++i)
		{
			if (!pEditorCameraComponent->GetFrustum()->TestAABB(mStaticMeshesContext.mpBounds[i]->GetBounds()))
			{
				continue;
			}

			ProcessStaticMeshEntity(mStaticMeshesContext, mpResourceManager, commandIndex++, i, mSelectionMaterialHandle, mMeshesCommands);

			if (mStaticMeshesContext.mHasSelectedEntityComponent[i])
			{
				ProcessStaticMeshEntity(mStaticMeshesContext, mpResourceManager, static_cast<U32>(E_GEOMETRY_SUBGROUP_TAGS::SELECTION_OUTLINE),
										i, mSelectionOutlineMaterialHandle, mSelectedMeshesCommands);
			}
		}

		/// \note Skinned meshes
		for (USIZE i = 0; i < static_cast<U32>(mSkinnedMeshesContext.mpRenderables.size()); ++i)
		{
			if (!pEditorCameraComponent->GetFrustum()->TestAABB(mSkinnedMeshesContext.mpBounds[i]->GetBounds()))
			{
				continue;
			}

			ProcessSkinnedMeshEntity(mSkinnedMeshesContext, mpResourceManager, commandIndex++, i, mSelectionSkinnedMaterialHandle, mMeshesCommands);

			if (mSkinnedMeshesContext.mHasSelectedEntityComponent[i])
			{
				ProcessSkinnedMeshEntity(mSkinnedMeshesContext, mpResourceManager, static_cast<U32>(E_GEOMETRY_SUBGROUP_TAGS::SELECTION_OUTLINE),
					i, mSelectionSkinnedOutlineMaterialHandle, mSelectedMeshesCommands);
			}
		}

		/// \note Quad sprites
		for (USIZE i = 0; i < static_cast<U32>(mSpritesContext.mpRenderables.size()); ++i)
		{
			if (!pEditorCameraComponent->GetFrustum()->TestAABB(mSpritesContext.mpBounds[i]->GetBounds()))
			{
				continue;
			}

			ProcessSpriteEntity(mSpritesContext, commandIndex++, i, mSpritesCommands);

			if (mSpritesContext.mHasSelectedEntityComponent[i])
			{
				ProcessSpriteEntity(mSpritesContext, static_cast<U32>(E_GEOMETRY_SUBGROUP_TAGS::SELECTION_OUTLINE), i, mSelectedSpritesCommands);
			}
		}

		/// \note UI elements 
		mUIElementsVertexBufferCurrOffset = 0;
		mUIElementsVertices.clear();

		for (USIZE i = 0; i < static_cast<U32>(mUIElementsContext.mpRenderables.size()); ++i)
		{
			/// \note Use sprites' index buffer because ui elements are just quads too
			ProcessUIElementEntity(pWorld, mUIElementsVertices, mUIElementsContext, mpResourceManager, commandIndex++, i, mUIElementsVertexBufferCurrOffset, mUIElementsCommands);
		}
	}

	E_RESULT_CODE CObjectsSelectionSystem::FillFramePacket(TFramePacket& framePacket)
	{
		TDE2_PROFILER_SCOPE("CObjectsSelectionSystem::FillFramePacket");

		CRenderQueue* pEditorOnlyRenderQueue = framePacket.mpRenderQueues[static_cast<U32>(E_RENDER_QUEUE_GROUP::RQG_EDITOR_ONLY)].Get();

		for (const TMeshDrawEntry& currMeshCommandEntry : mMeshesCommands)
		{
			if (TDrawIndexedCommand* pDrawCommand = pEditorOnlyRenderQueue->SubmitDrawCommand<TDrawIndexedCommand>(currMeshCommandEntry.mDrawGroupKey))
			{
				pDrawCommand->mVertexBufferHandle      = currMeshCommandEntry.mSharedPositionOnlyVertexBufferHandle;
				pDrawCommand->mIndexBufferHandle       = currMeshCommandEntry.mSharedIndexBufferHandle;
				pDrawCommand->mMaterialHandle          = mSelectionMaterialHandle;
				pDrawCommand->mPrimitiveType           = E_PRIMITIVE_TOPOLOGY_TYPE::PTT_TRIANGLE_LIST;
				pDrawCommand->mObjectData.mModelMatrix = currMeshCommandEntry.mModelMat;
				pDrawCommand->mObjectData.mObjectID    = currMeshCommandEntry.mObjectID;
				pDrawCommand->mStartIndex              = currMeshCommandEntry.mStartIndex;
				pDrawCommand->mNumOfIndices            = currMeshCommandEntry.mIndicesCount;
				pDrawCommand->mStartVertex             = 0;
			}
		}

		for (const TSpriteDrawEntry& currSpriteCommandEntry : mSpritesCommands)
		{
			if (TDrawIndexedCommand* pDrawCommand = pEditorOnlyRenderQueue->SubmitDrawCommand<TDrawIndexedCommand>(currSpriteCommandEntry.mDrawGroupKey))
			{
				pDrawCommand->mVertexBufferHandle      = mSpritesVertexBufferHandle;
				pDrawCommand->mIndexBufferHandle       = mSpritesIndexBufferHandle;
				pDrawCommand->mMaterialHandle          = mSelectionMaterialHandle;
				pDrawCommand->mPrimitiveType           = E_PRIMITIVE_TOPOLOGY_TYPE::PTT_TRIANGLE_LIST;
				pDrawCommand->mObjectData.mModelMatrix = currSpriteCommandEntry.mModelMat;
				pDrawCommand->mObjectData.mObjectID    = currSpriteCommandEntry.mObjectID;
				pDrawCommand->mStartIndex              = 0;
				pDrawCommand->mStartVertex             = 0;
				pDrawCommand->mNumOfIndices            = 6;
			}
		}

		TPtr<IBuffer> pUIElementsVertexBuffer = mpGraphicsObjectManager->GetBufferPtr(framePacket.mSelectionSystemData.mUIElementsVertexBufferHandle);
		
		constexpr USIZE MAX_UI_ELEMENTS_VERTICES_COUNT = 1024;
		constexpr USIZE UI_ELEMENTS_VERTEX_BUFFER_SIZE = sizeof(TVector4) * MAX_UI_ELEMENTS_VERTICES_COUNT;

		/// \note If there is no a created vertex buffer or we go out of space extend/create it
		if (!pUIElementsVertexBuffer || (pUIElementsVertexBuffer && pUIElementsVertexBuffer->GetSize() <= mUIElementsContext.mpRenderables.size() * 4))
		{
			if (!pUIElementsVertexBuffer)
			{
				framePacket.mSelectionSystemData.mUIElementsVertexBufferHandle = mpGraphicsObjectManager->CreateBuffer(
					{
						E_BUFFER_USAGE_TYPE::DYNAMIC,
						E_BUFFER_TYPE::STRUCTURED,
						UI_ELEMENTS_VERTEX_BUFFER_SIZE,
						nullptr,
						UI_ELEMENTS_VERTEX_BUFFER_SIZE,
						false,
						sizeof(TVector4),
						E_STRUCTURED_BUFFER_TYPE::DEFAULT
					}).Get();
			}
			else
			{
				framePacket.mSelectionSystemData.mUIElementsVertexBufferHandle = mpGraphicsObjectManager->CreateBuffer(
					{
						E_BUFFER_USAGE_TYPE::DYNAMIC,
						E_BUFFER_TYPE::STRUCTURED,
						pUIElementsVertexBuffer->GetSize() + UI_ELEMENTS_VERTEX_BUFFER_SIZE,
						nullptr,
						UI_ELEMENTS_VERTEX_BUFFER_SIZE,
						false,
						sizeof(TVector4),
						E_STRUCTURED_BUFFER_TYPE::DEFAULT
					}).Get();
			}
		}

		if (pUIElementsVertexBuffer = mpGraphicsObjectManager->GetBufferPtr(framePacket.mSelectionSystemData.mUIElementsVertexBufferHandle))
		{
			E_RESULT_CODE result = pUIElementsVertexBuffer->Map(E_BUFFER_MAP_TYPE::BMT_WRITE_DISCARD);
			TDE2_ASSERT(RC_OK == result);

			pUIElementsVertexBuffer->Write(mUIElementsVertices.data(), sizeof(TVector4) * mUIElementsVertices.size());
			pUIElementsVertexBuffer->Unmap();
		}

		for (const TUIElementDrawEntry& currUiElementCommandEntry : mUIElementsCommands)
		{
			if (TDrawIndexedCommand* pDrawCommand = pEditorOnlyRenderQueue->SubmitDrawCommand<TDrawIndexedCommand>(currUiElementCommandEntry.mDrawGroupKey))
			{
				pDrawCommand->mVertexBufferHandle      = framePacket.mSelectionSystemData.mUIElementsVertexBufferHandle;
				pDrawCommand->mIndexBufferHandle       = mSpritesIndexBufferHandle;
				pDrawCommand->mMaterialHandle          = mSelectionMaterialHandle;
				pDrawCommand->mPrimitiveType           = E_PRIMITIVE_TOPOLOGY_TYPE::PTT_TRIANGLE_LIST;
				pDrawCommand->mObjectData.mModelMatrix = currUiElementCommandEntry.mModelMat;
				pDrawCommand->mObjectData.mObjectID    = currUiElementCommandEntry.mObjectID;
				pDrawCommand->mStartIndex              = 0;
				pDrawCommand->mStartVertex             = currUiElementCommandEntry.mStartVertex;
				pDrawCommand->mNumOfIndices            = 6;
			}
		}

		CRenderQueue* pDebugRenderQueue = framePacket.mpRenderQueues[static_cast<U32>(E_RENDER_QUEUE_GROUP::RQG_DEBUG)].Get();

		for (const TMeshDrawEntry& currMeshCommandEntry : mSelectedMeshesCommands)
		{
			if (TDrawIndexedCommand* pDrawCommand = pDebugRenderQueue->SubmitDrawCommand<TDrawIndexedCommand>(currMeshCommandEntry.mDrawGroupKey))
			{
				pDrawCommand->mVertexBufferHandle      = currMeshCommandEntry.mSharedPositionOnlyVertexBufferHandle;
				pDrawCommand->mIndexBufferHandle       = currMeshCommandEntry.mSharedIndexBufferHandle;
				pDrawCommand->mMaterialHandle          = mSelectionOutlineMaterialHandle;
				pDrawCommand->mPrimitiveType           = E_PRIMITIVE_TOPOLOGY_TYPE::PTT_TRIANGLE_LIST;
				pDrawCommand->mObjectData.mModelMatrix = currMeshCommandEntry.mModelMat;
				pDrawCommand->mObjectData.mObjectID    = currMeshCommandEntry.mObjectID;
				pDrawCommand->mStartIndex              = currMeshCommandEntry.mStartIndex;
				pDrawCommand->mNumOfIndices            = currMeshCommandEntry.mIndicesCount;
				pDrawCommand->mStartVertex             = 0;
			}
		}

		for (const TSpriteDrawEntry& currSpriteCommandEntry : mSelectedSpritesCommands)
		{
			if (TDrawIndexedCommand* pDrawCommand = pEditorOnlyRenderQueue->SubmitDrawCommand<TDrawIndexedCommand>(currSpriteCommandEntry.mDrawGroupKey))
			{
				pDrawCommand->mVertexBufferHandle      = mSpritesVertexBufferHandle;
				pDrawCommand->mIndexBufferHandle       = mSpritesIndexBufferHandle;
				pDrawCommand->mMaterialHandle          = mSelectionOutlineMaterialHandle;
				pDrawCommand->mPrimitiveType           = E_PRIMITIVE_TOPOLOGY_TYPE::PTT_TRIANGLE_LIST;
				pDrawCommand->mObjectData.mModelMatrix = currSpriteCommandEntry.mModelMat;
				pDrawCommand->mObjectData.mObjectID    = currSpriteCommandEntry.mObjectID;
				pDrawCommand->mStartIndex              = 0;
				pDrawCommand->mStartVertex             = 0;
				pDrawCommand->mNumOfIndices            = 6;
			}
		}

		return RC_OK;
	}

	E_RESULT_CODE CObjectsSelectionSystem::_initSpriteBuffers()
	{
		TDE2_PROFILER_SCOPE("CObjectsSelectionSystem::_initSpriteBuffers");

		static const TVector4 quadSpriteVertices[4]
		{
			{ -0.5f, 0.5f, 0.0f, 1.0f },
			{ 0.5f, 0.5f, 0.0f, 1.0f },
			{ -0.5f, -0.5f, 0.0f, 1.0f },
			{ 0.5f, -0.5f, 0.0f, 1.0f }
		};

		auto spriteVertexBufferResult = mpGraphicsObjectManager->CreateBuffer(
			{ 
				E_BUFFER_USAGE_TYPE::STATIC,
				E_BUFFER_TYPE::STRUCTURED, 
				sizeof(TVector4) * 4, 
				quadSpriteVertices,
				sizeof(TVector4) * 4,
				false,
				sizeof(TVector4),
				E_STRUCTURED_BUFFER_TYPE::DEFAULT
			});

		if (spriteVertexBufferResult.HasError())
		{
			return spriteVertexBufferResult.GetError();
		}

		mSpritesVertexBufferHandle = spriteVertexBufferResult.Get();

		static const U32 spriteTriangles[6] { 0, 1, 2, 2, 1, 3 };
		
		auto spriteIndexBufferResult = mpGraphicsObjectManager->CreateBuffer(
			{ 
				E_BUFFER_USAGE_TYPE::STATIC, 
				E_BUFFER_TYPE::STRUCTURED, 
				sizeof(U32) * 6, 
				spriteTriangles,
				sizeof(U32) * 6,
				false,
				sizeof(U32),
				E_STRUCTURED_BUFFER_TYPE::DEFAULT
			});

		if (spriteIndexBufferResult.HasError())
		{
			return spriteIndexBufferResult.GetError();
		}

		mSpritesIndexBufferHandle = spriteIndexBufferResult.Get();

		return RC_OK;
	}


	static TMaterialParameters CreateSelectionMaterialParams(const std::string& materialName)
	{
		const TMaterialParameters selectionMaterialParams
		{
			materialName, false,
			TDepthStencilStateDesc { true, false, E_COMPARISON_FUNC::LESS_EQUAL},
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
		TDE2_PROFILER_SCOPE("CObjectsSelectionSystem::_initSelectionMaterials");

		const static TMaterialParameters selectionMaterialParams        = CreateSelectionMaterialParams("Shaders/Default/Selection.shader");
		const static TMaterialParameters selectionSkinnedMaterialParams = CreateSelectionMaterialParams("Shaders/Default/SelectionSkinned.shader");
		const static TMaterialParameters selectionUIMaterialParams      = CreateSelectionMaterialParams("Shaders/Default/SelectionUI.shader");

		const static TMaterialParameters selectionOutlineMaterialParams        = CreateSelectionOutlineMaterialParams("Shaders/Default/SelectionOutline.shader");
		const static TMaterialParameters selectionSkinnedOutlineMaterialParams = CreateSelectionOutlineMaterialParams("Shaders/Default/SelectionSkinnedOutline.shader");

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
			return pCameraEntity->GetComponent<CCamera>();
		}

		return nullptr;
	}


	TDE2_API ISystem* CreateObjectsSelectionSystem(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem, CObjectsSelectionSystem, result, pRenderer, pGraphicsObjectManager);
	}


	struct TObjectsSelectionSystemAutoInitializer : TSystemAutoInitializer
	{
		virtual ~TObjectsSelectionSystemAutoInitializer() = default;

		E_RESULT_CODE ManageDependencies(IWorld* pWorld) override
		{
			std::array<TSystemId, 5> dependenciesHandles
			{
				pWorld->FindSystem<CCameraSystem>(),
				pWorld->FindSystem<CStaticMeshRendererSystem>(),
				pWorld->FindSystem<CSkinnedMeshRendererSystem>(),
				pWorld->FindSystem<CSpriteRendererSystem>(),
				pWorld->FindSystem<CUIElementsProcessSystem>()
			};

			E_RESULT_CODE result = RC_OK;

			for (const TSystemId& currDependencyHandle : dependenciesHandles)
			{
				result = result | (currDependencyHandle == TSystemId::Invalid ? RC_FAIL : RC_OK);
				result = result | pSystemInstance->AddDependency(pWorld->GetSystem(currDependencyHandle));
			}

			return result;
		}

		ISystem* GetSystem(IWorld* pWorld, const TRequestSubsystemCallback& subsystemsProviderCallback)
		{
			E_RESULT_CODE result = RC_OK;
			pSystemInstance = CreateObjectsSelectionSystem(
				PolymorphicCast<IRenderer*>(subsystemsProviderCallback(E_ENGINE_SUBSYSTEM_TYPE::EST_RENDERER)),
				PolymorphicCast<IGraphicsContext*>(subsystemsProviderCallback(E_ENGINE_SUBSYSTEM_TYPE::EST_GRAPHICS_CONTEXT))->GetGraphicsObjectManager(), result);

			return pSystemInstance;
		}

		ISystem* pSystemInstance = nullptr;
	};


	TDE2_REGISTER_SYSTEM(TObjectsSelectionSystemAutoInitializer);
}

#endif