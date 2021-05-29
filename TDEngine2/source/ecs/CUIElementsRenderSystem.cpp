#include "../../include/ecs/CUIElementsRenderSystem.h"
#include "../../include/ecs/IWorld.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/ecs/CTransform.h"
#include "../../include/graphics/UI/CCanvasComponent.h"
#include "../../include/graphics/UI/CLayoutElementComponent.h"
#include "../../include/graphics/IGraphicsObjectManager.h"
#include "../../include/graphics/IRenderer.h"
#include "../../include/graphics/IVertexDeclaration.h"
#include "../../include/graphics/IMaterial.h"
#include "../../include/graphics/CGraphicsLayersInfo.h"
#include "../../include/graphics/UI/CUIElementMeshDataComponent.h"
#include "../../include/graphics/CRenderQueue.h"
#include "../../include/graphics/IVertexBuffer.h"
#include "../../include/graphics/IIndexBuffer.h"
#include "../../include/graphics/IMaterial.h"
#include "../../include/graphics/ITexture.h"
#include "../../include/core/IResource.h"
#include "../../include/core/IResourceManager.h"


namespace TDEngine2
{
	CUIElementsRenderSystem::CUIElementsRenderSystem() :
		CBaseSystem()
	{
	}

	E_RESULT_CODE CUIElementsRenderSystem::Init(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pRenderer || !pGraphicsObjectManager)
		{
			return RC_INVALID_ARGS;
		}

		mpGraphicsObjectManager = pGraphicsObjectManager;
		mpUIElementsRenderGroup = pRenderer->GetRenderQueue(E_RENDER_QUEUE_GROUP::RQG_OVERLAY);
		mpResourceManager = pRenderer->GetResourceManager();

		E_RESULT_CODE result = _initDefaultResources();
		if (RC_OK != result)
		{
			return result;
		}

		mpGraphicsLayers = CreateGraphicsLayersInfo(result);
		if (RC_OK != result)
		{
			return result;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CUIElementsRenderSystem::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		--mRefCounter;

		if (!mRefCounter)
		{
			mIsInitialized = false;
			delete this;
		}

		return RC_OK;
	}

	void CUIElementsRenderSystem::InjectBindings(IWorld* pWorld)
	{
		mUIElementsEntities = pWorld->FindEntitiesWithAny<CUIElementMeshData>();
	}


	static U32 ComputeCommandKey(TResourceId materialId, TMaterialInstanceId instanceId, U16 graphicsLayerId)
	{
		return ((static_cast<U32>(materialId) << 24) | (static_cast<U32>(instanceId) << 16)) | graphicsLayerId;
	}


	void CUIElementsRenderSystem::Update(IWorld* pWorld, F32 dt)
	{
		CEntity* pEntity = nullptr;
		CEntity* pCanvasEntity = nullptr;

		CCanvas* pCurrCanvas = nullptr;

		mVertices.clear();
		mIndices.clear();

		for (TEntityId currEntityId : mUIElementsEntities)
		{
			pEntity = pWorld->FindEntity(currEntityId);
			if (!pEntity)
			{
				continue;
			}

			CUIElementMeshData* pMeshData = pEntity->GetComponent<CUIElementMeshData>();
			CTransform* pTransform = pEntity->GetComponent<CTransform>();
			CLayoutElement* pLayoutElement = pEntity->GetComponent<CLayoutElement>();

			/// \note Get entity that represents a canvas
			if (!pCanvasEntity || (pCanvasEntity->GetId() != pLayoutElement->GetOwnerCanvasId()))
			{
				pCanvasEntity = pWorld->FindEntity(pLayoutElement->GetOwnerCanvasId());
				pCurrCanvas = pCanvasEntity->GetComponent<CCanvas>();
			}

			TDE2_ASSERT(pCurrCanvas && pCanvasEntity);

			const bool isTextMesh = pMeshData->IsTextMesh();

			const TResourceId currMaterialId = isTextMesh ? mDefaultFontMaterialId : mDefaultUIMaterialId;
			const TResourceId currTextureId = pMeshData->GetTextureResourceId();

			IMaterial* pMaterial = mpResourceManager->GetResource<IMaterial>(currMaterialId);

			if (mUsingMaterials.find(currTextureId) == mUsingMaterials.cend()) // \note create a new instance if it doesn't exist yet
			{
				mUsingMaterials.emplace(currTextureId, pMaterial->CreateInstance()->GetInstanceId());
			}

			ITexture* pTexture = mpResourceManager->GetResource<ITexture>(currTextureId);

			const TMaterialInstanceId currMaterialInstance = mUsingMaterials[currTextureId];

			pMaterial->SetTextureResource("Texture", pTexture, currMaterialInstance);

			const U16 layerIndex = static_cast<U16>(std::abs(pTransform->GetPosition().z)); /// \todo Reimplement this 

			auto pCurrCommand = mpUIElementsRenderGroup->SubmitDrawCommand<TDrawIndexedCommand>(ComputeCommandKey(currMaterialId, currMaterialInstance, layerIndex));

			auto&& vertices = pMeshData->GetVertices();
			auto&& indices = pMeshData->GetIndices();

			pCurrCommand->mpVertexBuffer      = mpVertexBuffer;
			pCurrCommand->mpIndexBuffer       = mpIndexBuffer;
			pCurrCommand->mPrimitiveType      = E_PRIMITIVE_TOPOLOGY_TYPE::PTT_TRIANGLE_LIST;
			pCurrCommand->mMaterialHandle     = currMaterialId;
			pCurrCommand->mMaterialInstanceId = currMaterialInstance;
			pCurrCommand->mpVertexDeclaration = isTextMesh ? mpDefaultFontVertexDecl : mpDefaultUIVertexDecl;
			pCurrCommand->mStartIndex         = static_cast<U32>(mIndices.size());
			pCurrCommand->mStartVertex        = static_cast<U32>(mVertices.size());
			pCurrCommand->mNumOfIndices       = static_cast<U32>(indices.size());

			std::copy(vertices.cbegin(), vertices.cend(), std::back_inserter(mVertices));
			std::copy(indices.cbegin(), indices.cend(), std::back_inserter(mIndices));

			/// \todo Implement this
			auto&& uvRect = pTexture ? pTexture->GetNormalizedTextureRect() : TRectF32{ 0.0f, 0.0f, 1.0f, 1.0f };

			/// \todo Add computation of a projection matrix
			pCurrCommand->mObjectData.mModelMatrix = Transpose(pCurrCanvas->GetProjMatrix());
			pCurrCommand->mObjectData.mTextureTransformDesc = { uvRect.x, uvRect.y, uvRect.width, uvRect.height };
		}

		if (mVertices.empty())
		{
			return;
		}

		/// \note Write data into GPU buffers
		E_RESULT_CODE result = _updateGPUBuffers();
		TDE2_ASSERT(RC_OK == result);
	}

	E_RESULT_CODE CUIElementsRenderSystem::_initDefaultResources()
	{
		auto createUIVertDeclResult = mpGraphicsObjectManager->CreateVertexDeclaration();
		if (createUIVertDeclResult.HasError())
		{
			return createUIVertDeclResult.GetError();
		}

		mpDefaultUIVertexDecl = createUIVertDeclResult.Get();

		mpDefaultUIVertexDecl->AddElement({ TDEngine2::FT_FLOAT4, 0, TDEngine2::VEST_POSITION });
		mpDefaultUIVertexDecl->AddElement({ TDEngine2::FT_FLOAT4, 0, TDEngine2::VEST_COLOR });

		// \note load default editor's material (depth test and writing to the depth buffer are disabled)
		TMaterialParameters editorUIMaterialParams{ "DefaultEditorUI", true, { false, false }, { E_CULL_MODE::BACK } };

		auto& blendingParams = editorUIMaterialParams.mBlendingParams;
		blendingParams.mScrValue = E_BLEND_FACTOR_VALUE::SOURCE_ALPHA;
		blendingParams.mDestValue = E_BLEND_FACTOR_VALUE::ONE_MINUS_SOURCE_ALPHA;
		blendingParams.mScrAlphaValue = E_BLEND_FACTOR_VALUE::ONE_MINUS_SOURCE_ALPHA;
		blendingParams.mDestAlphaValue = E_BLEND_FACTOR_VALUE::ZERO;

		mDefaultUIMaterialId = mpResourceManager->Create<IMaterial>("DefaultInGameUI.material", editorUIMaterialParams);
		mDefaultFontMaterialId = mpResourceManager->Load<IMaterial>("DefaultMaterials/UI/DefaultTextMaterial.material");

		/// \note Create a default vertex declaration for fonts
		auto createFontVertDeclResult = mpGraphicsObjectManager->CreateVertexDeclaration();
		if (createFontVertDeclResult.HasError())
		{
			return createFontVertDeclResult.GetError();
		}
		
		mpDefaultFontVertexDecl = createFontVertDeclResult.Get();

		mpDefaultFontVertexDecl->AddElement({ TDEngine2::FT_FLOAT4, 0, TDEngine2::VEST_POSITION });
		//mpDefaultFontVertexDecl->AddElement({ TDEngine2::FT_FLOAT2, 0, TDEngine2::VEST_TEXCOORDS });
		mpDefaultFontVertexDecl->AddElement({ TDEngine2::FT_FLOAT4, 0, TDEngine2::VEST_COLOR });

		/// \note Create vertex buffer and index one
		auto createVertBufferResult = mpGraphicsObjectManager->CreateVertexBuffer(E_BUFFER_USAGE_TYPE::BUT_DYNAMIC, sizeof(TUIElementsVertex) * mMaxVerticesCount, nullptr);
		if (createVertBufferResult.HasError())
		{
			return createVertBufferResult.GetError();
		}

		mpVertexBuffer = createVertBufferResult.Get();

		auto createIndexBufferResult = mpGraphicsObjectManager->CreateIndexBuffer(E_BUFFER_USAGE_TYPE::BUT_DYNAMIC, E_INDEX_FORMAT_TYPE::IFT_INDEX16, sizeof(U16) * mMaxVerticesCount * 3, nullptr);
		if (createIndexBufferResult.HasError())
		{
			return createIndexBufferResult.GetError();
		}

		mpIndexBuffer = createIndexBufferResult.Get();

		return RC_OK;
	}

	E_RESULT_CODE CUIElementsRenderSystem::_updateGPUBuffers()
	{
		E_RESULT_CODE result = mpVertexBuffer->Map(E_BUFFER_MAP_TYPE::BMT_WRITE_DISCARD);
		if (RC_OK != result)
		{
			return result;
		}

		result = mpVertexBuffer->Write(&mVertices.front(), static_cast<U32>(sizeof(TUIElementsVertex) * mVertices.size()));
		if (RC_OK != result)
		{
			return result;
		}

		mpVertexBuffer->Unmap();

		/// \note Index buffer
		result = mpIndexBuffer->Map(E_BUFFER_MAP_TYPE::BMT_WRITE_DISCARD);
		if (RC_OK != result)
		{
			return result;
		}

		result = mpIndexBuffer->Write(&mIndices.front(), static_cast<U32>(sizeof(U16) * mIndices.size()));
		if (RC_OK != result)
		{
			return result;
		}

		mpIndexBuffer->Unmap();

		return RC_OK;
	}


	TDE2_API ISystem* CreateUIElementsRenderSystem(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem, CUIElementsRenderSystem, result, pRenderer, pGraphicsObjectManager);
	}
}