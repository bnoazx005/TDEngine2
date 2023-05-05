#include "../../include/ecs/CUIElementsRenderSystem.h"
#include "../../include/ecs/IWorld.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/ecs/CTransform.h"
#include "../../include/ecs/CUIEventsSystem.h"
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
#include "../../include/editor/CPerfProfiler.h"
#include <stack>
#include <tuple>


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

	void CUIElementsRenderSystem::InjectBindings(IWorld* pWorld)
	{
		auto& transforms     = mUIElementsContext.mpTransforms;
		auto& layoutElements = mUIElementsContext.mpLayoutElements;
		auto& uiMeshData     = mUIElementsContext.mpUIMeshData;

		transforms.clear();
		layoutElements.clear();
		uiMeshData.clear();

		/// \note Find main canvas which has no parent or its parent has no CLayoutElement component attached
		for (TEntityId currCanvasEntity : FindMainCanvases(pWorld))
		{
			CTransform* pTransform = pWorld->FindEntity(currCanvasEntity)->GetComponent<CTransform>();

			/// \note Sort all entities based on their priority (children're first)
			std::stack<std::tuple<TEntityId, bool>> entitiesToVisit;

			entitiesToVisit.emplace(currCanvasEntity, false);

			CEntity* pEntity = nullptr;
			
			TEntityId currEntityId = TEntityId::Invalid;
			bool hasUIMaskApplied = false;

			while (!entitiesToVisit.empty())
			{
				std::tie(currEntityId, hasUIMaskApplied) = entitiesToVisit.top();
				entitiesToVisit.pop();

				pEntity = pWorld->FindEntity(currEntityId);
				if (pEntity->HasComponent<CDeactivatedComponent>() || pEntity->HasComponent<CDeactivatedGroupComponent>())
				{
					continue;
				}

				if (pEntity->HasComponent<CUIElementMeshData>())
				{
					transforms.insert(transforms.begin(), pEntity->GetComponent<CTransform>());
					layoutElements.insert(layoutElements.begin(), pEntity->GetComponent<CLayoutElement>());

					auto pUIMeshData = pEntity->GetComponent<CUIElementMeshData>();
					uiMeshData.insert(uiMeshData.begin(), pUIMeshData);

					pUIMeshData->SetMaterialType(pEntity->HasComponent<CUIMaskComponent>() ? E_UI_MATERIAL_TYPE::MASK_EMITTER :
						(hasUIMaskApplied ? E_UI_MATERIAL_TYPE::MASK_USER : E_UI_MATERIAL_TYPE::DEFAULT));

					if (!hasUIMaskApplied)
					{
						hasUIMaskApplied = pEntity->HasComponent<CUIMaskComponent>();
					}
				}

				if (pTransform = pEntity->GetComponent<CTransform>())
				{
					for (TEntityId id : pTransform->GetChildren())
					{
						entitiesToVisit.emplace(id, hasUIMaskApplied);
					}
				}
			}
		}
	}


	static U32 ComputeCommandKey(TResourceId materialId, TMaterialInstanceId instanceId, U16 graphicsLayerId)
	{
		return ((static_cast<U32>(materialId) << 24) | (static_cast<U32>(instanceId) << 16)) | graphicsLayerId;
	}


	static U64 ComputeTextureMaterialHash(TResourceId materialId, TResourceId textureId)
	{
		return static_cast<U32>(textureId) | (static_cast<U32>(materialId) << (sizeof(U32) * 8 - 1));
	}


	void CUIElementsRenderSystem::Update(IWorld* pWorld, F32 dt)
	{
		TDE2_PROFILER_SCOPE("CUIElementsRenderSystem::Update");

		CEntity* pCanvasEntity = nullptr;

		CCanvas* pCurrCanvas = nullptr;
		CCanvas* pPrevCanvas = nullptr;

		mVertices.clear();
		mIndices.clear();

		U32 index = 0;

		TResourceId prevMaterialId = TResourceId::Invalid;
		TMaterialInstanceId prevMaterialInstanceId = DefaultMaterialInstanceId;

		auto& transforms     = mUIElementsContext.mpTransforms;
		auto& layoutElements = mUIElementsContext.mpLayoutElements;
		auto& uiMeshData     = mUIElementsContext.mpUIMeshData;

		auto shouldBatchBeFlushed = [this, pWorld, &uiMeshData](const TResourceId currMaterialId, const TMaterialInstanceId currMaterialInstanceId, USIZE index)
		{
			if (index + 1 >= uiMeshData.size())
			{
				return true;
			}

			if (CUIElementMeshData* pMeshData = uiMeshData[index + 1])
			{
				const TResourceId nextMaterialId = pMeshData->IsTextMesh() ? 
					mDefaultFontMaterialId[static_cast<USIZE>(pMeshData->GetMaterialType())] : mDefaultUIMaterialId[static_cast<USIZE>(pMeshData->GetMaterialType())];

				if (nextMaterialId != currMaterialId)
				{
					return true;
				}

				const TResourceId nextTextureId = pMeshData->GetTextureResourceId();

				auto it = mUsingMaterials.find(ComputeTextureMaterialHash(nextMaterialId, nextTextureId));
				if (it == mUsingMaterials.cend())
				{
					return true;
				}

				return (currMaterialInstanceId != it->second);
			}

			return false;
		};

		for (USIZE i = 0; i < layoutElements.size(); ++i)
		{
			CUIElementMeshData* pMeshData = uiMeshData[i];
			CLayoutElement* pLayoutElement = layoutElements[i];
			CTransform* pTransform = transforms[i];

			/// \note Get entity that represents a canvas
			if (!pCanvasEntity || (pCanvasEntity->GetId() != pLayoutElement->GetOwnerCanvasId()))
			{
				pCanvasEntity = pWorld->FindEntity(pLayoutElement->GetOwnerCanvasId());
				if (!pCanvasEntity)
				{
					return;
				}

				pPrevCanvas = pCurrCanvas;
				pCurrCanvas = pCanvasEntity->GetComponent<CCanvas>();
			}

			TDE2_ASSERT(pCurrCanvas && pCanvasEntity);

			const bool isTextMesh = pMeshData->IsTextMesh();

			const TResourceId currMaterialId = isTextMesh ? mDefaultFontMaterialId[static_cast<USIZE>(pMeshData->GetMaterialType())] : mDefaultUIMaterialId[static_cast<USIZE>(pMeshData->GetMaterialType())];
			const TResourceId currTextureId = pMeshData->GetTextureResourceId();

			auto pMaterial = mpResourceManager->GetResource<IMaterial>(currMaterialId);

			const U64 textureMaterialHash = ComputeTextureMaterialHash(currMaterialId, currTextureId);

			if (mUsingMaterials.find(textureMaterialHash) == mUsingMaterials.cend()) // \note create a new instance if it doesn't exist yet
			{
				mUsingMaterials.emplace(textureMaterialHash, pMaterial->CreateInstance()->GetInstanceId());
			}

			auto pTexture = mpResourceManager->GetResource<ITexture>(currTextureId);

			const TMaterialInstanceId currMaterialInstance = mUsingMaterials[textureMaterialHash];

			pMaterial->SetTextureResource("Texture", pTexture.Get(), currMaterialInstance);
			
			if (!isTextMesh)
			{
				pMaterial->SetVariableForInstance<I32>(currMaterialInstance, "mIsAlphaClipEnabled", E_UI_MATERIAL_TYPE::MASK_EMITTER == pMeshData->GetMaterialType());
			}

			auto&& vertices = pMeshData->GetVertices();
			auto&& indices = pMeshData->GetIndices();

			if (vertices.empty() || indices.empty())
			{
				continue;
			}

			for (U16 index : indices)
			{
				mIntermediateIndexBuffer.push_back(static_cast<U16>(mIntermediateVertsBuffer.empty() ? 0 : mIntermediateVertsBuffer.size()) + index);
			}

			std::copy(vertices.cbegin(), vertices.cend(), std::back_inserter(mIntermediateVertsBuffer));
			
			/// \note Flush current buffers when the batch is splitted
			if (shouldBatchBeFlushed(currMaterialId, currMaterialInstance, i) || (pCurrCanvas && pPrevCanvas != pCurrCanvas))
			{
				auto pCurrCommand = mpUIElementsRenderGroup->SubmitDrawCommand<TDrawIndexedCommand>(
					((0xFFFF - (pCurrCanvas->GetPriority() + (0xFFFF >> 1))) << 16) | index);

				pCurrCommand->mpVertexBuffer = mpVertexBuffer;
				pCurrCommand->mpIndexBuffer = mpIndexBuffer;
				pCurrCommand->mPrimitiveType = E_PRIMITIVE_TOPOLOGY_TYPE::PTT_TRIANGLE_LIST;
				pCurrCommand->mMaterialHandle = currMaterialId;
				pCurrCommand->mMaterialInstanceId = currMaterialInstance;
				pCurrCommand->mpVertexDeclaration = isTextMesh ? mpDefaultFontVertexDecl : mpDefaultUIVertexDecl;
				pCurrCommand->mStartIndex = static_cast<U32>(mIndices.size());
				pCurrCommand->mStartVertex = static_cast<U32>(mVertices.size());
				pCurrCommand->mNumOfIndices = static_cast<U32>(mIntermediateIndexBuffer.size());

				std::copy(mIntermediateVertsBuffer.cbegin(), mIntermediateVertsBuffer.cend(), std::back_inserter(mVertices));
				std::copy(mIntermediateIndexBuffer.cbegin(), mIntermediateIndexBuffer.cend(), std::back_inserter(mIndices));

				/// \todo Implement this
				auto&& uvRect = pTexture ? pTexture->GetNormalizedTextureRect() : TRectF32{ 0.0f, 0.0f, 1.0f, 1.0f };

				/// \todo Add computation of a projection matrix
				auto&& rect = pLayoutElement->GetWorldRect();
				auto pivot = rect.GetLeftBottom() + pLayoutElement->GetPivot() * rect.GetSizes();

				auto pivotTranslation = TranslationMatrix(TVector3{ -pivot.x, -pivot.y, 0.0f });
				TMatrix4 localObjectTransform = Inverse(pivotTranslation) * RotationMatrix(pTransform->GetRotation()) * ScaleMatrix(pTransform->GetScale()) * pivotTranslation;

				pCurrCommand->mObjectData.mModelMatrix = Transpose(pCurrCanvas->GetProjMatrix() * localObjectTransform);
				pCurrCommand->mObjectData.mTextureTransformDesc = { uvRect.x, uvRect.y, uvRect.width, uvRect.height };

				mIntermediateVertsBuffer.clear();
				mIntermediateIndexBuffer.clear();

				++index;

				continue;
			}		
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
		{
			TMaterialParameters editorUIMaterialParams{ "DefaultEditorUI", true, { false, false }, { E_CULL_MODE::BACK } };

			auto& blendingParams = editorUIMaterialParams.mBlendingParams;
			blendingParams.mScrValue = E_BLEND_FACTOR_VALUE::SOURCE_ALPHA;
			blendingParams.mDestValue = E_BLEND_FACTOR_VALUE::ONE_MINUS_SOURCE_ALPHA;
			blendingParams.mScrAlphaValue = E_BLEND_FACTOR_VALUE::ONE_MINUS_SOURCE_ALPHA;
			blendingParams.mDestAlphaValue = E_BLEND_FACTOR_VALUE::ZERO;

			mDefaultUIMaterialId[static_cast<USIZE>(E_UI_MATERIAL_TYPE::DEFAULT)] = mpResourceManager->Create<IMaterial>("DefaultInGameUI.material", editorUIMaterialParams);

			blendingParams.mWriteMask = 0x8;

			auto& depthStencilParams = editorUIMaterialParams.mDepthStencilParams;
			depthStencilParams.mIsStencilTestEnabled = true;
			depthStencilParams.mStencilReadMaskValue = 0xFF;
			depthStencilParams.mStencilWriteMaskValue = 0xFF;
			depthStencilParams.mStencilFrontFaceOp.mFunc = E_COMPARISON_FUNC::ALWAYS;
			depthStencilParams.mStencilFrontFaceOp.mFailOp = E_STENCIL_OP::KEEP;
			depthStencilParams.mStencilFrontFaceOp.mPassOp = E_STENCIL_OP::REPLACE;
			depthStencilParams.mStencilRefValue = 0x1;

			mDefaultUIMaterialId[static_cast<USIZE>(E_UI_MATERIAL_TYPE::MASK_EMITTER)] = mpResourceManager->Create<IMaterial>("DefaultInGameUI_Mask.material", editorUIMaterialParams);

			depthStencilParams.mStencilFrontFaceOp.mFunc = E_COMPARISON_FUNC::EQUAL;
			depthStencilParams.mStencilFrontFaceOp.mFailOp = E_STENCIL_OP::KEEP;
			depthStencilParams.mStencilFrontFaceOp.mPassOp = E_STENCIL_OP::KEEP;
			depthStencilParams.mStencilReadMaskValue = 0x1;
			depthStencilParams.mStencilWriteMaskValue = 0x0; /// Maskable elements are allowed only to read values

			mDefaultUIMaterialId[static_cast<USIZE>(E_UI_MATERIAL_TYPE::MASK_USER)] = mpResourceManager->Create<IMaterial>("DefaultInGameUI_Maskable.material", editorUIMaterialParams);
		}

		mDefaultFontMaterialId[static_cast<USIZE>(E_UI_MATERIAL_TYPE::DEFAULT)] = mpResourceManager->Load<IMaterial>("DefaultResources/Materials/UI/DefaultTextMaterial.material");

		if (auto pDefaultTextMaterial = mpResourceManager->GetResource<IMaterial>(mDefaultFontMaterialId[static_cast<USIZE>(E_UI_MATERIAL_TYPE::DEFAULT)]))
		{
			/// \note Text can't be a mask but can be a maskable item
			mDefaultFontMaterialId[static_cast<USIZE>(E_UI_MATERIAL_TYPE::MASK_EMITTER)] = mDefaultFontMaterialId[static_cast<USIZE>(E_UI_MATERIAL_TYPE::DEFAULT)];

			if (auto pTextMaskableMaterial = pDefaultTextMaterial->Clone())
			{
				pTextMaskableMaterial->SetStencilBufferEnabled(true);
				pTextMaskableMaterial->SetStencilReadMask(0x1);
				pTextMaskableMaterial->SetStencilWriteMask(0x0);
				pTextMaskableMaterial->SetStencilRefValue(0x1);
				pTextMaskableMaterial->SetStencilFrontOp({ E_COMPARISON_FUNC::EQUAL, E_STENCIL_OP::KEEP, E_STENCIL_OP::KEEP });

				mDefaultFontMaterialId[static_cast<USIZE>(E_UI_MATERIAL_TYPE::MASK_USER)] = DynamicPtrCast<IResource>(pTextMaskableMaterial)->GetId();
			}
		}

		/// \note Create a default vertex declaration for fonts
		auto createFontVertDeclResult = mpGraphicsObjectManager->CreateVertexDeclaration();
		if (createFontVertDeclResult.HasError())
		{
			return createFontVertDeclResult.GetError();
		}
		
		mpDefaultFontVertexDecl = createFontVertDeclResult.Get();

		mpDefaultFontVertexDecl->AddElement({ TDEngine2::FT_FLOAT4, 0, TDEngine2::VEST_POSITION });
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