#include "../../include/ecs/CUIElementsProcessSystem.h"
#include "../../include/ecs/IWorld.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/ecs/CTransform.h"
#include "../../include/graphics/UI/CCanvasComponent.h"
#include "../../include/graphics/UI/CLayoutElementComponent.h"
#include "../../include/graphics/UI/CImageComponent.h"
#include "../../include/graphics/UI/CLabelComponent.h"
#include "../../include/graphics/UI/CUIElementMeshDataComponent.h"
#include "../../include/graphics/ITexture2D.h"
#include "../../include/graphics/ITexture.h"
#include "../../include/core/IFont.h"
#include "../../include/core/IResource.h"
#include "../../include/core/IGraphicsContext.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/core/IWindowSystem.h"


namespace TDEngine2
{
	static void UpdateLayoutElementData(IWorld* pWorld, CEntity* pEntity)
	{
		CLayoutElement* pLayoutElement = pEntity->GetComponent<CLayoutElement>();
		CTransform* pTransform = pEntity->GetComponent<CTransform>();

		if (pEntity->HasComponent<CCanvas>())
		{
			CCanvas* pCanvas = pEntity->GetComponent<CCanvas>();
			const TVector2 canvasSizes{ static_cast<F32>(pCanvas->GetWidth()), static_cast<F32>(pCanvas->GetHeight()) };

			const TVector2 leftBottom = pLayoutElement->GetMinOffset() + Scale(pLayoutElement->GetMinAnchor(), canvasSizes);
			const TVector2 rightTop = pLayoutElement->GetMaxOffset() + Scale(pLayoutElement->GetMaxAnchor(), canvasSizes);

			const TVector2 rectSizes = rightTop - leftBottom;
			const TVector3 position = pTransform->GetPosition();

			pLayoutElement->SetWorldRect({ position.x - leftBottom.x, position.y - leftBottom.y, rectSizes.x, rectSizes.y });

			return;
		}

		if (TEntityId::Invalid == pTransform->GetParent())
		{
			TDE2_ASSERT(false);
			return;
		}

		CEntity* pParentEntity = pWorld->FindEntity(pTransform->GetParent());
		if (!pParentEntity)
		{
			TDE2_ASSERT(false);
			return;
		}

		CLayoutElement* pParentLayoutElement = pParentEntity->GetComponent<CLayoutElement>();
		if (!pParentLayoutElement)
		{
			TDE2_ASSERT(false);
			return;
		}

		auto parentWorldRect = pParentLayoutElement->GetWorldRect();

		const TVector2 parentLBRect = parentWorldRect.GetLeftBottom();
		const TVector2 parentRectSize = parentWorldRect.GetSizes();

		const TVector2 minAnchor = pLayoutElement->GetMinAnchor();
		const TVector2 maxAnchor = pLayoutElement->GetMaxAnchor();

		const TVector2 lbWorldPoint = parentLBRect + parentRectSize * minAnchor;
		const TVector2 rtWorldPoint = parentLBRect + parentRectSize * maxAnchor;

		const F32 maxOffsetSign = Length(maxAnchor - minAnchor) < 1e-3f ? 1.0f : -1.0f;

		TRectF32 worldRect
		{
			lbWorldPoint + pLayoutElement->GetMinOffset(),
			rtWorldPoint + maxOffsetSign * pLayoutElement->GetMaxOffset() /// \todo Is this a correct way to implement that?
		};

		if (Length(maxAnchor - minAnchor) < 1e-3f)
		{
			auto&& sizes = pLayoutElement->GetMaxOffset();

			worldRect.width = sizes.x;
			worldRect.height = sizes.y;
		}

		const TVector2 originShift = worldRect.GetSizes() * pLayoutElement->GetPivot();

		pLayoutElement->SetWorldRect(worldRect);
		pLayoutElement->SetAnchorWorldRect({ lbWorldPoint, rtWorldPoint });
		pLayoutElement->SetParentWorldRect(parentWorldRect);

		const TVector2 position = worldRect.GetLeftBottom() + originShift;
		pTransform->SetPosition(TVector3(position.x, position.y, 0.0f));
	}


	static void UpdateCanvasData(IGraphicsContext* pGraphicsContext, IWorld* pWorld, CEntity* pEntity)
	{
		TDE2_ASSERT(pEntity->HasComponent<CCanvas>());

		if (CCanvas* pCanvas = pEntity->GetComponent<CCanvas>())
		{
			if (!pCanvas->IsDirty())
			{
				return;
			}

			if (pCanvas->DoesInheritSizesFromMainCamera())
			{
				IWindowSystem* pWindowSystem = pGraphicsContext->GetWindowSystem();

				pCanvas->SetWidth(pWindowSystem->GetWidth());
				pCanvas->SetHeight(pWindowSystem->GetHeight());
			}

			/// \note The canvas's origin is a left-bottom corner
			pCanvas->SetProjMatrix(pGraphicsContext->CalcOrthographicMatrix(0.0f, static_cast<F32>(pCanvas->GetHeight()), static_cast<F32>(pCanvas->GetWidth()), 0.0f, 0.0f, 1.0f, true));
		}
	}

	static TEntityId FindParentCanvasEntityId(IWorld* pWorld, CEntity* pEntity)
	{
		CEntity* pCurrEntity = pEntity;

		CTransform* pTransform = pEntity->GetComponent<CTransform>();
		TEntityId currParentId = pTransform->GetParent();

		while ((TEntityId::Invalid != currParentId) && !pCurrEntity->HasComponent<CCanvas>())
		{
			pCurrEntity = pWorld->FindEntity(currParentId);

			pTransform = pCurrEntity->GetComponent<CTransform>();
			currParentId = pTransform->GetParent();
		}

		return (pCurrEntity == pEntity) ? TEntityId::Invalid : pCurrEntity->GetId();
	}


	static CUIElementMeshData* GetUIElementsMeshData(CEntity* pEntity)
	{
		if (!pEntity->HasComponent<CUIElementMeshData>())
		{
			pEntity->AddComponent<CUIElementMeshData>();
		}

		auto pUIElementMeshData = pEntity->GetComponent<CUIElementMeshData>();
		pUIElementMeshData->ResetMesh();

		return pUIElementMeshData;
	}


	static void ComputeImageMeshData(IResourceManager* pResourceManager, IWorld* pWorld, TEntityId id)
	{
		CEntity* pEntity = pWorld->FindEntity(id);
		if (!pEntity)
		{
			return;
		}

		CLayoutElement* pLayoutData = pEntity->GetComponent<CLayoutElement>();
		CImage* pImageData = pEntity->GetComponent<CImage>();

		/// \note Load image's asset if it's not done yet
		if (TResourceId::Invalid == pImageData->GetImageResourceId())
		{
			E_RESULT_CODE result = pImageData->SetImageResourceId(pResourceManager->Load<ITexture2D>(pImageData->GetImageId()));
			TDE2_ASSERT(RC_OK == result);
		}

		ITexture* pImageSprite = pResourceManager->GetResource<ITexture>(pImageData->GetImageResourceId());
		if (!pImageSprite)
		{
			return;
		}

		auto pUIElementMeshData = GetUIElementsMeshData(pEntity);

		auto&& worldRect = pLayoutData->GetWorldRect();

		auto&& lbPoint = worldRect.GetLeftBottom();
		auto&& rtPoint = worldRect.GetRightTop();

		/// \todo Add support of specifying color data
		pUIElementMeshData->AddVertex({ TVector4(lbPoint.x, lbPoint.y, 0.0f, 1.0f), TColorUtils::mWhite });
		pUIElementMeshData->AddVertex({ TVector4(lbPoint.x, rtPoint.y, 0.0f, 0.0f), TColorUtils::mWhite });
		pUIElementMeshData->AddVertex({ TVector4(rtPoint.x, lbPoint.y, 1.0f, 1.0f), TColorUtils::mWhite });
		pUIElementMeshData->AddVertex({ TVector4(rtPoint.x, rtPoint.y, 1.0f, 0.0f), TColorUtils::mWhite });

		static const std::array<U16, 6> indices { 0, 1, 2, 2, 1, 3 }; /// \note standard CW ordered 2 triangles that form a quad

		for (U16 index : indices)
		{
			pUIElementMeshData->AddIndex(index);
		}

		pUIElementMeshData->SetTextureResourceId(pImageData->GetImageResourceId());
	}


	static void ComputeTextMeshData(IResourceManager* pResourceManager, IWorld* pWorld, TEntityId id)
	{
		CEntity* pEntity = pWorld->FindEntity(id);
		if (!pEntity)
		{
			return;
		}

		CLayoutElement* pLayoutData = pEntity->GetComponent<CLayoutElement>();
		CLabel* pLabelData = pEntity->GetComponent<CLabel>();

		/// \note Load font data if it's not loaded yet
		if (TResourceId::Invalid == pLabelData->GetFontResourceHandle())
		{
			E_RESULT_CODE result = pLabelData->SetFontResourceHandle(pResourceManager->Load<IFont>(pLabelData->GetFontId()));
			TDE2_ASSERT(RC_OK == result);
		}

		IFont* pFont = pResourceManager->GetResource<IFont>(pLabelData->GetFontResourceHandle());

		auto pUIElementMeshData = GetUIElementsMeshData(pEntity);

		E_FONT_ALIGN_POLICY alignType = pLabelData->GetAlignType();

		const TRectF32 elementRect = pLayoutData->GetWorldRect();
		TVector2 textOffsetPosition = elementRect.GetLeftBottom() + CLabel::GetPositionFromAlighType(alignType) * elementRect.GetSizes();

		/// \note Transfer vertices from pFont->GenerateMesh into UIMeshData component
		auto&& textMeshVertsData = pFont->GenerateMesh(ZeroVector2, 1.0f, CU8String(pLabelData->GetText()));

		if (CLabel::IsCenterizeAlignPolicy(alignType))
		{
			textOffsetPosition.x -= textMeshVertsData.mTextRectSizes.x * 0.5f;
		}

		if (CLabel::IsRightsidedAlignPolicy(alignType))
		{
			textOffsetPosition.x -= textMeshVertsData.mTextRectSizes.x;
		}

		for (const TVector4& currVertex : textMeshVertsData.mVerts)
		{
			pUIElementMeshData->AddVertex({ { currVertex.x + textOffsetPosition.x, currVertex.y + textOffsetPosition.y, currVertex.z, currVertex.w }, TColorUtils::mWhite });
		}

		U16 index = 0;

		for (U16 i = 0; i < textMeshVertsData.mNeededIndicesCount; i += 6)
		{
			pUIElementMeshData->AddIndex(index); pUIElementMeshData->AddIndex(index + 1); pUIElementMeshData->AddIndex(index + 2);
			pUIElementMeshData->AddIndex(index + 2); pUIElementMeshData->AddIndex(index + 1); pUIElementMeshData->AddIndex(index + 3);

			index += 4;
		}

		pUIElementMeshData->SetTextureResourceId(dynamic_cast<IResource*>(pFont->GetTexture())->GetId()); /// \todo Replace dynamic_cast with proper method in IFont
		pUIElementMeshData->SetTextMeshFlag(true);
	}


	CUIElementsProcessSystem::CUIElementsProcessSystem() :
		CBaseSystem()
	{
	}

	E_RESULT_CODE CUIElementsProcessSystem::Init(IGraphicsContext* pGraphicsContext, IResourceManager* pResourceManager)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pGraphicsContext || !pResourceManager)
		{
			return RC_INVALID_ARGS;
		}

		mpGraphicsContext = pGraphicsContext;
		mpResourceManager = pResourceManager;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CUIElementsProcessSystem::Free()
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


	static void SortLayoutElementEntities(IWorld* pWorld, std::vector<TEntityId>& entities)
	{
		auto&& layoutElementsEntities = pWorld->FindEntitiesWithComponents<CTransform, CLayoutElement>();
		if (layoutElementsEntities.empty())
		{
			return;
		}

		entities.clear();

		std::unordered_map<TEntityId, U32> parentEntitiesTable;

		// \note Sort all entities in the following order that every parent should precede its children

		for (TEntityId currEntityId : layoutElementsEntities)
		{
			if (CEntity* pEntity = pWorld->FindEntity(currEntityId))
			{
				if (pEntity->HasComponent<CCanvas>())
				{
					continue; // skip entities with CCanvas component their will be processed before any LayoutElement ones
				}

				CTransform* pTransform = pEntity->GetComponent<CTransform>();

				if (TEntityId::Invalid == pTransform->GetParent())
				{
					entities.push_back(currEntityId);
					parentEntitiesTable[currEntityId] = static_cast<U32>(entities.size());
					continue;
				}

				const TEntityId parentId = pTransform->GetParent();

				auto it = parentEntitiesTable.find(parentId);
				if (it == parentEntitiesTable.cend())
				{
					entities.push_back(currEntityId);
					parentEntitiesTable[currEntityId] = static_cast<U32>(entities.size());

					continue;
				}

				entities.insert(entities.cbegin() + it->second, currEntityId);
			}
		}
	}


	void CUIElementsProcessSystem::InjectBindings(IWorld* pWorld)
	{
		SortLayoutElementEntities(pWorld, mLayoutElementsEntities);

		mCanvasEntities = pWorld->FindEntitiesWithComponents<CTransform, CCanvas>();

		/// \note Add LayoutElement for each Canvas
		for (TEntityId currEntity : mCanvasEntities)
		{
			CEntity* pEntity = pWorld->FindEntity(currEntity);
			if (pEntity->HasComponent<CLayoutElement>())
			{
				continue;
			}

			CLayoutElement* pLayoutElement = pEntity->AddComponent<CLayoutElement>(); /// \note use both anchors that're stretched
			pLayoutElement->SetMinAnchor(ZeroVector2);
			pLayoutElement->SetMaxAnchor(TVector2(1.0f));
			pLayoutElement->SetMinOffset(ZeroVector2);
			pLayoutElement->SetMaxOffset(ZeroVector2);
		}

		mImagesEntities = pWorld->FindEntitiesWithComponents<CLayoutElement, CImage>();
		mLabelsEntities = pWorld->FindEntitiesWithComponents<CLayoutElement, CLabel>();
	}

	void CUIElementsProcessSystem::Update(IWorld* pWorld, F32 dt)
	{
		CEntity* pEntity = nullptr;

		/// \note Update canvas entities
		for (TEntityId currEntity : mCanvasEntities)
		{
			pEntity = pWorld->FindEntity(currEntity);

			UpdateCanvasData(mpGraphicsContext, pWorld, pEntity);
			UpdateLayoutElementData(pWorld, pEntity); 
		}

		/// \note Process LayoutElement entities
		for (TEntityId currEntity : mLayoutElementsEntities)
		{
			pEntity = pWorld->FindEntity(currEntity);
			UpdateLayoutElementData(pWorld, pEntity);
			
			if (auto pLayoutElement = pEntity->GetComponent<CLayoutElement>())
			{
				pLayoutElement->SetOwnerCanvasId(FindParentCanvasEntityId(pWorld, pEntity));
			}
		}

		/// \note Compute meshes for Images
		for (TEntityId currEntity : mImagesEntities)
		{
			ComputeImageMeshData(mpResourceManager, pWorld, currEntity);
		}

		/// \note Compute meshes for Labels
		for (TEntityId currEntity : mLabelsEntities)
		{
			ComputeTextMeshData(mpResourceManager, pWorld, currEntity);
		}
	}


	TDE2_API ISystem* CreateUIElementsProcessSystem(IGraphicsContext* pGraphicsContext, IResourceManager* pResourceManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem, CUIElementsProcessSystem, result, pGraphicsContext, pResourceManager);
	}
}