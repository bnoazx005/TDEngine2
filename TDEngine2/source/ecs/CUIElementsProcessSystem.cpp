#include "../../include/ecs/CUIElementsProcessSystem.h"
#include "../../include/ecs/IWorld.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/ecs/CTransform.h"
#include "../../include/graphics/UI/CCanvasComponent.h"
#include "../../include/graphics/UI/CLayoutElementComponent.h"
#include "../../include/graphics/UI/CImageComponent.h"
#include "../../include/graphics/UI/C9SliceImageComponent.h"
#include "../../include/graphics/UI/CLabelComponent.h"
#include "../../include/graphics/UI/CUIElementMeshDataComponent.h"
#include "../../include/graphics/UI/GroupLayoutComponents.h"
#include "../../include/graphics/ITexture2D.h"
#include "../../include/graphics/ITexture.h"
#include "../../include/editor/CPerfProfiler.h"
#include "../../include/core/CFont.h"
#include "../../include/core/IResource.h"
#include "../../include/core/IGraphicsContext.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/core/IWindowSystem.h"
#include <algorithm>


namespace TDEngine2
{
	static void UpdateLayoutElementData(IWorld* pWorld, CUIElementsProcessSystem::TLayoutElementsContext& layoutElementsContext, USIZE id)
	{
		TDE2_PROFILER_SCOPE("UpdateLayoutElementData");

		CLayoutElement* pLayoutElement = layoutElementsContext.mpLayoutElements[id];
		CTransform* pTransform = layoutElementsContext.mpTransforms[id];

		if (layoutElementsContext.mChildToParentTable[id] == TComponentsQueryLocalSlice<CTransform>::mInvalidParentIndex)
		{
			return;
		}

		CLayoutElement* pParentLayoutElement = layoutElementsContext.mpLayoutElements[layoutElementsContext.mChildToParentTable[id]];
		if (!pParentLayoutElement)
		{
			TDE2_ASSERT(false);
			return;
		}

		if (!pLayoutElement->IsDirty() && !pParentLayoutElement->IsDirty())
		{
			return;
		}

		const auto& parentWorldRect = pParentLayoutElement->GetWorldRect();

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

		const TVector2 position = worldRect.GetLeftBottom() + originShift;

		pLayoutElement->SetWorldRect(worldRect);
		pLayoutElement->SetAnchorWorldRect({ lbWorldPoint, rtWorldPoint });
		pLayoutElement->SetParentWorldRect(parentWorldRect);

		pTransform->SetPosition(TVector3(position.x, position.y, 0.0f));
	}


	static void UpdateCanvasLayoutElementData(IWorld* pWorld, CUIElementsProcessSystem::TCanvasesContext& canvasesContext, USIZE id)
	{
		TDE2_PROFILER_SCOPE("UpdateLayoutElementData");

		CLayoutElement* pLayoutElement = canvasesContext.mpLayoutElements[id];
		if (!pLayoutElement->IsDirty())
		{
			return;
		}

		CTransform* pTransform = canvasesContext.mpTransforms[id];
		CCanvas* pCanvas = canvasesContext.mpCanvases[id];

		if (!pCanvas)
		{
			TDE2_ASSERT(false);
			return;
		}

		const TVector2 canvasSizes{ static_cast<F32>(pCanvas->GetWidth()), static_cast<F32>(pCanvas->GetHeight()) };

		const TVector2 leftBottom = pLayoutElement->GetMinOffset() + Scale(pLayoutElement->GetMinAnchor(), canvasSizes);
		const TVector2 rightTop = pLayoutElement->GetMaxOffset() + Scale(pLayoutElement->GetMaxAnchor(), canvasSizes);

		const TVector2 rectSizes = rightTop - leftBottom;
		const TVector3 position = pTransform->GetPosition();

		pLayoutElement->SetWorldRect({ position.x - leftBottom.x, position.y - leftBottom.y, rectSizes.x, rectSizes.y });
	}


	static void UpdateGridGroupLayoutElementData(IWorld* pWorld, CUIElementsProcessSystem::TGridGroupsContext& context, USIZE id, CUIElementsProcessSystem::TLayoutElementsContext& layoutElements)
	{
		TDE2_PROFILER_SCOPE("UpdateGridGroupLayoutElementData");

		CLayoutElement* pLayoutElement = context.mpLayoutElements[id];
		CTransform* pTransform = context.mpTransforms[id];
		CGridGroupLayout* pGridGroupLayout = context.mpGridGroupLayouts[id];

		if (!pGridGroupLayout->IsDirty() && !pLayoutElement->IsDirty())
		{
			return;
		}

		U32 index = 0;
		U32 x, y;

		const TVector2& cellSize = pGridGroupLayout->GetCellSize();
		const TVector2& spacing = pGridGroupLayout->GetSpaceBetweenElements();

		const auto& contentRect = pLayoutElement->GetWorldRect();

		/// \todo Implement correct alignment of children
		TVector2 offset = CalcContentRectAlignByType(pLayoutElement->GetParentWorldRect(), contentRect, pGridGroupLayout->GetElementsAlignType());
		//offset.y = contentRect.height + offset.y;

		const U16 columnsCount = static_cast<U16>(std::roundf(contentRect.width / (cellSize.x + spacing.x)));

		auto&& children = pTransform->GetChildren();

		for (TEntityId currChildId : children)
		{
			auto it = std::find(layoutElements.mEntities.cbegin(), layoutElements.mEntities.cend(), currChildId);
			if (it == layoutElements.mEntities.cend())
			{
				TDE2_UNREACHABLE();
				continue;
			}

			CLayoutElement* pChildLayoutElement = layoutElements.mpLayoutElements[std::distance(layoutElements.mEntities.cbegin(), it)];
			if (!pChildLayoutElement || !columnsCount)
			{
				TDE2_UNREACHABLE();
				continue;
			}

			x = index % columnsCount;
			y = index / columnsCount;

			++index;

			pChildLayoutElement->SetMinAnchor(TVector2(0.0f, 1.0f));
			pChildLayoutElement->SetMaxAnchor(TVector2(0.0f, 1.0f));
			pChildLayoutElement->SetMinOffset(offset + TVector2(x * (cellSize.x + spacing.x), -(cellSize.y + spacing.y) * y - cellSize.y));
			pChildLayoutElement->SetMaxOffset(cellSize);

			pChildLayoutElement->SetDirty(true);
		}
	}


	// \note The function returns false if update isn't needed
	static bool UpdateCanvasData(IGraphicsContext* pGraphicsContext, CUIElementsProcessSystem::TCanvasesContext& canvasesContext, USIZE id)
	{
		TDE2_ASSERT(canvasesContext.mpCanvases.size() > id);

		if (CCanvas* pCanvas = canvasesContext.mpCanvases[id])
		{
			if (!pCanvas->IsDirty())
			{
				return false;
			}

			if (pCanvas->DoesInheritSizesFromMainCamera())
			{
				IWindowSystem* pWindowSystem = pGraphicsContext->GetWindowSystem().Get();

				pCanvas->SetWidth(pWindowSystem->GetWidth());
				pCanvas->SetHeight(pWindowSystem->GetHeight());
			}

			/// \note The canvas's origin is a left-bottom corner
			pCanvas->SetProjMatrix(pGraphicsContext->CalcOrthographicMatrix(0.0f, static_cast<F32>(pCanvas->GetHeight()), static_cast<F32>(pCanvas->GetWidth()), 0.0f, 0.0f, 1.0f, true));
		}

		return true;
	}

	static TEntityId FindParentCanvasEntityId(IWorld* pWorld, CEntity* pEntity)
	{
		TDE2_PROFILER_SCOPE("FindParentCanvasEntityId");

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


	static void ComputeImageMeshData(IResourceManager* pResourceManager, const CUIElementsProcessSystem::TUIRenderableElementsContext<CImage>& imagesContext, USIZE index)
	{
		CLayoutElement* pLayoutData = imagesContext.mpLayoutElements[index];
		CImage* pImageData = imagesContext.mpRenderables[index];

		/// \note Load image's asset if it's not done yet
		if (TResourceId::Invalid == pImageData->GetImageResourceId())
		{
			E_RESULT_CODE result = pImageData->SetImageResourceId(pResourceManager->Load<ITexture2D>(pImageData->GetImageId()));
			TDE2_ASSERT(RC_OK == result);
		}

		auto pImageSprite = pResourceManager->GetResource<ITexture>(pImageData->GetImageResourceId());
		if (!pImageSprite)
		{
			return;
		}

		const bool isDirty = pLayoutData->IsDirty(); /// \todo Add dirty flag of ImageComponent
		if (!isDirty && !pImageData->IsDirty())
		{
			return;
		}

		auto pUIElementMeshData = imagesContext.mpUIMeshes[index];
		pUIElementMeshData->ResetMesh();

		auto&& worldRect = pLayoutData->GetWorldRect();

		auto pTransform = imagesContext.mpTransforms[index];
		auto pivot = worldRect.GetLeftBottom() + worldRect.GetSizes() * pLayoutData->GetPivot();
		auto pivotTranslation = TranslationMatrix(TVector3{ -pivot.x, -pivot.y, 0.0f });

		const TMatrix4 localObjectTransform = Inverse(pivotTranslation) * RotationMatrix(pTransform->GetRotation()) * ScaleMatrix(pTransform->GetScale()) * pivotTranslation;

		auto&& lbPoint = localObjectTransform * worldRect.GetLeftBottom();
		auto&& rtPoint = localObjectTransform * worldRect.GetRightTop();

		/// \todo Add support of specifying color data
		pUIElementMeshData->AddVertex({ TVector4(lbPoint.x, lbPoint.y, 0.0f, 1.0f), pImageData->GetColor() });
		pUIElementMeshData->AddVertex({ TVector4(lbPoint.x, rtPoint.y, 0.0f, 0.0f), pImageData->GetColor() });
		pUIElementMeshData->AddVertex({ TVector4(rtPoint.x, lbPoint.y, 1.0f, 1.0f), pImageData->GetColor() });
		pUIElementMeshData->AddVertex({ TVector4(rtPoint.x, rtPoint.y, 1.0f, 0.0f), pImageData->GetColor() });

		static const std::array<U16, 6> indices { 0, 1, 2, 2, 1, 3 }; /// \note standard CW ordered 2 triangles that form a quad

		for (U16 index : indices)
		{
			pUIElementMeshData->AddIndex(index);
		}

		pUIElementMeshData->SetTextureResourceId(pImageData->GetImageResourceId());

		pImageData->SetDirtyFlag(false);
	}


	static void Compute9SliceImageMeshData(IResourceManager* pResourceManager, const CUIElementsProcessSystem::TUIRenderableElementsContext<C9SliceImage>& slicedImagesContext, USIZE index)
	{
		CLayoutElement* pLayoutData    = slicedImagesContext.mpLayoutElements[index];
		C9SliceImage* pSlicedImageData = slicedImagesContext.mpRenderables[index];

		/// \note Load image's asset if it's not done yet
		if (TResourceId::Invalid == pSlicedImageData->GetImageResourceId())
		{
			E_RESULT_CODE result = pSlicedImageData->SetImageResourceId(pResourceManager->Load<ITexture2D>(pSlicedImageData->GetImageId()));
			TDE2_ASSERT(RC_OK == result);
		}

		auto pImageSprite = pResourceManager->GetResource<ITexture>(pSlicedImageData->GetImageResourceId());
		if (!pImageSprite)
		{
			return;
		}

		const bool isDirty = pLayoutData->IsDirty(); /// \todo Add dirty flag of 9SliceImage
		if (!isDirty)
		{
			return;
		}

		auto pUIElementMeshData = slicedImagesContext.mpUIMeshes[index];
		pUIElementMeshData->ResetMesh();

		auto&& worldRect = pLayoutData->GetWorldRect();

		auto pTransform = slicedImagesContext.mpTransforms[index];
		auto pivot = worldRect.GetLeftBottom() + worldRect.GetSizes() * pLayoutData->GetPivot();
		auto pivotTranslation = TranslationMatrix(TVector3{ -pivot.x, -pivot.y, 0.0f });

		const TMatrix4 localObjectTransform = Inverse(pivotTranslation) * RotationMatrix(pTransform->GetRotation()) * ScaleMatrix(pTransform->GetScale()) * pivotTranslation;

		auto&& lbPoint = localObjectTransform * worldRect.GetLeftBottom();
		auto&& rtPoint = localObjectTransform * worldRect.GetRightTop();

		const F32 relativeMargin = pSlicedImageData->GetRelativeBorderSize();

		const std::array<F32, 4> horizontalRectCoords {	lbPoint.x, CMathUtils::Lerp(lbPoint.x, rtPoint.x, relativeMargin), CMathUtils::Lerp(lbPoint.x, rtPoint.x, 1.0f - relativeMargin), rtPoint.x	};
		const std::array<F32, 4> verticalRectCoords	{ lbPoint.y, CMathUtils::Lerp(lbPoint.y, rtPoint.y, relativeMargin), CMathUtils::Lerp(lbPoint.y, rtPoint.y, 1.0f - relativeMargin), rtPoint.y };

		auto&& uvCoordsRect = pImageSprite->GetNormalizedTextureRect();
		auto&& topRightUvCoords = uvCoordsRect.GetRightTop();

		const std::array<F32, 4> horizontalUvCoords
		{
			uvCoordsRect.x, 
			CMathUtils::Lerp(uvCoordsRect.x, topRightUvCoords.x, pSlicedImageData->GetLeftXSlicer()), 
			CMathUtils::Lerp(uvCoordsRect.x, topRightUvCoords.x, pSlicedImageData->GetRightXSlicer()),
			topRightUvCoords.x
		};

		/// \note v texture coordinates are stored in inverted order
		const std::array<F32, 4> verticalUvCoords
		{
			topRightUvCoords.y,
			CMathUtils::Lerp(uvCoordsRect.y, topRightUvCoords.y, pSlicedImageData->GetTopYSlicer()),
			CMathUtils::Lerp(uvCoordsRect.y, topRightUvCoords.y, pSlicedImageData->GetBottomYSlicer()),
			uvCoordsRect.y,
		};

		/// \todo Add support of specifying color data
		for (USIZE i = 0; i < 4; ++i)
		{
			for (USIZE j = 0; j < 4; ++j)
			{
				pUIElementMeshData->AddVertex({ TVector4(horizontalRectCoords[j], verticalRectCoords[i], horizontalUvCoords[j], verticalUvCoords[i]), TColorUtils::mWhite });
			}
		}

		/// \note Generate indices data
		for (U16 k = 0; k < 9; ++k)
		{
			pUIElementMeshData->AddIndex((k / 3) + k);
			pUIElementMeshData->AddIndex((k / 3) + k + 5);
			pUIElementMeshData->AddIndex((k / 3) + k + 1);

			pUIElementMeshData->AddIndex((k / 3) + k);
			pUIElementMeshData->AddIndex((k / 3) + k + 4);
			pUIElementMeshData->AddIndex((k / 3) + k + 5);
		}

		pUIElementMeshData->SetTextureResourceId(pSlicedImageData->GetImageResourceId());
	}


	static inline void ComputeTextMeshData(IResourceManager* pResourceManager, const CUIElementsProcessSystem::TUIRenderableElementsContext<CLabel>& labelsContext, USIZE id)
	{
		CLayoutElement* pLayoutData = labelsContext.mpLayoutElements[id];
		CLabel* pLabelData = labelsContext.mpRenderables[id];

		/// \note Load font data if it's not loaded yet
		if (TResourceId::Invalid == pLabelData->GetFontResourceHandle())
		{
			E_RESULT_CODE result = pLabelData->SetFontResourceHandle(pResourceManager->Load<IFont>(pLabelData->GetFontId()));
			TDE2_ASSERT(RC_OK == result);
		}

		auto pFont = pResourceManager->GetResource<IFont>(pLabelData->GetFontResourceHandle());

		const bool isDirty = pLayoutData->IsDirty() || pLabelData->IsDirty() || (pLabelData->GetFontDataVersionId() != pFont->GetDataVersionNumber());
		if (!isDirty)
		{
			return;
		}

		auto pUIElementMeshData = labelsContext.mpUIMeshes[id];
		pUIElementMeshData->ResetMesh();
		
		/// \note Transfer vertices from pFont->GenerateMesh into UIMeshData component
		auto&& textMeshVertsData = pFont->GenerateMesh({ pLayoutData->GetWorldRect(), 1.0f, pLabelData->GetOverflowPolicyType(), pLabelData->GetAlignType() }, pLabelData->GetText());

		for (const TVector4& currVertex : textMeshVertsData.mVerts)
		{
			pUIElementMeshData->AddVertex({ currVertex, TColorUtils::mWhite });
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

		pLabelData->SetFontDataVersionId(pFont->GetDataVersionNumber());
		pLabelData->ResetDirtyFlag();
		pLayoutData->SetDirty(false);
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


	static void SortLayoutElementEntities(IWorld* pWorld, CUIElementsProcessSystem::TLayoutElementsContext& layoutElementsContext)
	{
		layoutElementsContext.mChildToParentTable.clear();
		layoutElementsContext.mEntities.clear();
		layoutElementsContext.mpTransforms.clear();
		layoutElementsContext.mpLayoutElements.clear();

		std::vector<TEntityId> entities = pWorld->FindEntitiesWithComponents<CTransform, CLayoutElement>();
		if (entities.empty())
		{
			return;
		}

		/// \note There are could be entities which are canvases. So firstly we should remove them from the array
		entities.erase(std::remove_if(entities.begin(), entities.end(), [pWorld](TEntityId entityId)
		{
			if (CEntity* pEntity = pWorld->FindEntity(entityId))
			{
				return pEntity->HasComponent<CCanvas>();
			}

			return false;
		}), entities.end());

		/// \note For CTransform we should sort all entities that parents should preceede their children
		/// \note Fill up relationships table to sort entities based on their dependencies 
		std::unordered_map<TEntityId, std::vector<TEntityId>> parentToChildRelations;

		std::vector<std::tuple<TEntityId, TEntityId, USIZE>> parentEntities;

		for (TEntityId currEntityId : entities)
		{
			if (CEntity* pEntity = pWorld->FindEntity(currEntityId))
			{
				const TEntityId parentId = pEntity->GetComponent<CTransform>()->GetParent();
				if (TEntityId::Invalid == parentId)
				{
					continue;
				}

				if (std::find(entities.cbegin(), entities.cend(), parentId) == entities.cend()) /// \note Mark this entity as root
				{
					parentEntities.push_back({ currEntityId, parentId, parentEntities.size() });					
				}

				parentToChildRelations[parentId].push_back(pEntity->GetId());
			}
		}

		entities.clear();

		TEntityId currEntityId;
		USIZE currParentElementIndex = 0;

		std::stack<std::tuple<TEntityId, USIZE>> entitiesToProcess;

		for (auto currEntityPair : parentEntities)
		{
			entitiesToProcess.push({ std::get<0>(currEntityPair), entities.size() });
			entitiesToProcess.push({ std::get<1>(currEntityPair), TComponentsQueryLocalSlice<CTransform>::mInvalidParentIndex });

			while (!entitiesToProcess.empty())
			{
				std::tie(currEntityId, currParentElementIndex) = entitiesToProcess.top();
				entitiesToProcess.pop();

				layoutElementsContext.mChildToParentTable.push_back(currParentElementIndex);
				layoutElementsContext.mEntities.push_back(currEntityId);
				entities.push_back(currEntityId);

				if (currParentElementIndex == TComponentsQueryLocalSlice<CTransform>::mInvalidParentIndex)
				{
					continue;
				}

				const USIZE parentIndex = entities.size() - 1;

				for (TEntityId currEntityId : parentToChildRelations[currEntityId])
				{
					entitiesToProcess.push({ currEntityId, parentIndex });
				}
			}
		}

		auto& transforms     = layoutElementsContext.mpTransforms;
		auto& layoutElements = layoutElementsContext.mpLayoutElements;

		transforms.clear();
		layoutElements.clear();

		for (auto currEntityId : entities)
		{
			if (auto pCurrEntity = pWorld->FindEntity(currEntityId))
			{
				transforms.push_back(pCurrEntity->GetComponent<CTransform>());
				layoutElements.push_back(pCurrEntity->GetComponent<CLayoutElement>());
			}
		}
	}


	template <typename T>
	static CUIElementsProcessSystem::TUIRenderableElementsContext<T> CreateUIRenderableContext(IWorld* pWorld)
	{
		auto&& entities = pWorld->FindEntitiesWithComponents<CLayoutElement, T>();

		CUIElementsProcessSystem::TUIRenderableElementsContext<T> result;

		for (auto&& currEntityId : entities)
		{
			if (auto pEntity = pWorld->FindEntity(currEntityId))
			{
				result.mpTransforms.push_back(pEntity->template GetComponent<CTransform>());
				result.mpRenderables.push_back(pEntity->template GetComponent<T>());
				result.mpLayoutElements.push_back(pEntity->template GetComponent<CLayoutElement>());
				result.mpUIMeshes.push_back(pEntity->template HasComponent<CUIElementMeshData>() ? pEntity->template GetComponent<CUIElementMeshData>() : pEntity->template AddComponent<CUIElementMeshData>());
			}
		}

		return std::move(result);
	}


	template <typename TFunc>
	static CUIElementsProcessSystem::TCanvasesContext CreateCanvasesContext(IWorld* pWorld, TFunc&& inserter)
	{
		CUIElementsProcessSystem::TCanvasesContext context;

		auto&& transforms = context.mpTransforms;
		auto&& layoutElements = context.mpLayoutElements;
		auto&& canvases = context.mpCanvases;

		auto&& canvasEntities = pWorld->FindEntitiesWithComponents<CTransform, CCanvas>();

		/// \note Add LayoutElement for each Canvas
		for (TEntityId currEntity : canvasEntities)
		{
			CEntity* pEntity = pWorld->FindEntity(currEntity);

			transforms.push_back(pEntity->template GetComponent<CTransform>());
			canvases.push_back(pEntity->template GetComponent<CCanvas>());

			if (pEntity->HasComponent<CLayoutElement>())
			{
				layoutElements.push_back(pEntity->template GetComponent<CLayoutElement>());
				inserter(currEntity, pEntity->template GetComponent<CLayoutElement>());

				continue;
			}

			CLayoutElement* pLayoutElement = pEntity->template AddComponent<CLayoutElement>(); /// \note use both anchors that're stretched
			pLayoutElement->SetMinAnchor(ZeroVector2);
			pLayoutElement->SetMaxAnchor(TVector2(1.0f));
			pLayoutElement->SetMinOffset(ZeroVector2);
			pLayoutElement->SetMaxOffset(ZeroVector2);

			layoutElements.push_back(pLayoutElement);

			inserter(currEntity, pLayoutElement);
		}

		return std::move(context);
	}


	template <typename TFunc>
	static CUIElementsProcessSystem::TGridGroupsContext CreateGridGroupLayoutsContext(IWorld* pWorld, TFunc&& inserter)
	{
		CUIElementsProcessSystem::TGridGroupsContext context;

		auto&& transforms       = context.mpTransforms;
		auto&& layoutElements   = context.mpLayoutElements;
		auto&& gridGroupLayouts = context.mpGridGroupLayouts;

		auto&& entities = pWorld->FindEntitiesWithComponents<CTransform, CGridGroupLayout>();

		/// \note Add LayoutElement for each Canvas
		for (TEntityId currEntity : entities)
		{
			CEntity* pEntity = pWorld->FindEntity(currEntity);

			transforms.push_back(pEntity->template GetComponent<CTransform>());
			gridGroupLayouts.push_back(pEntity->template GetComponent<CGridGroupLayout>());

			if (pEntity->HasComponent<CLayoutElement>())
			{
				layoutElements.push_back(pEntity->template GetComponent<CLayoutElement>());
				inserter(currEntity, pEntity->template GetComponent<CLayoutElement>());

				continue;
			}

			CLayoutElement* pLayoutElement = pEntity->template AddComponent<CLayoutElement>(); /// \note use both anchors that're stretched
			pLayoutElement->SetMinAnchor(ZeroVector2);
			pLayoutElement->SetMaxAnchor(TVector2(1.0f));
			pLayoutElement->SetMinOffset(ZeroVector2);
			pLayoutElement->SetMaxOffset(ZeroVector2);

			layoutElements.push_back(pLayoutElement);

			inserter(currEntity, pLayoutElement);
		}

		return std::move(context);
	}


	void CUIElementsProcessSystem::InjectBindings(IWorld* pWorld)
	{
		SortLayoutElementEntities(pWorld, mLayoutElementsContext);

		auto checkAndAssignLayoutElements = [this](TEntityId id, CLayoutElement* pLayoutElement)
		{
			auto it = std::find(mLayoutElementsContext.mEntities.begin(), mLayoutElementsContext.mEntities.end(), id);
			if (it == mLayoutElementsContext.mEntities.end())
			{
				return;
			}

			/// \note We've found an entity try to assign pLayoutElement instance into its corresponding slot
			mLayoutElementsContext.mpLayoutElements[std::distance(mLayoutElementsContext.mEntities.begin(), it)] = pLayoutElement;
		};

		mCanvasesContext         = CreateCanvasesContext(pWorld, checkAndAssignLayoutElements);
		mGridGroupLayoutsContext = CreateGridGroupLayoutsContext(pWorld, checkAndAssignLayoutElements);

		mImagesContext       = CreateUIRenderableContext<CImage>(pWorld);
		mSlicedImagesContext = CreateUIRenderableContext<C9SliceImage>(pWorld);
		mLabelsContext       = CreateUIRenderableContext<CLabel>(pWorld);
	}


	static inline void UpdateLayoutElements(CUIElementsProcessSystem::TLayoutElementsContext& layoutElementsContext, IWorld* pWorld)
	{
		TDE2_PROFILER_SCOPE("UpdateLayoutElements");

		for (USIZE i = 0; i < layoutElementsContext.mpTransforms.size(); ++i)
		{
			UpdateLayoutElementData(pWorld, layoutElementsContext, i);

			if (auto pLayoutElement = layoutElementsContext.mpLayoutElements[i])
			{
				if (TEntityId::Invalid != pLayoutElement->GetOwnerCanvasId())
				{
					continue;
				}

				pLayoutElement->SetOwnerCanvasId(FindParentCanvasEntityId(pWorld, pWorld->FindEntity(layoutElementsContext.mEntities[i])));
			}
		}
	}


	static inline void UpdateGridGroupLayoutElements(CUIElementsProcessSystem::TGridGroupsContext& gridGroupsContext, CUIElementsProcessSystem::TLayoutElementsContext& layoutElements, IWorld* pWorld)
	{
		TDE2_PROFILER_SCOPE("UpdateGridGroupLayoutElements");

		for (USIZE i = 0; i < gridGroupsContext.mpTransforms.size(); ++i)
		{
			UpdateGridGroupLayoutElementData(pWorld, gridGroupsContext, i, layoutElements);
		}
	}


	template <typename T>
	static inline void DiscardDirtyFlagForElements(T& context)
	{
		TDE2_PROFILER_SCOPE("DiscardDirtyFlagForElements");

		for (USIZE i = 0; i < context.mpLayoutElements.size(); ++i)
		{
			if (auto pLayoutElement = context.mpLayoutElements[i])
			{
				pLayoutElement->SetDirty(false);
			}
		}
	}


	template <>
	void DiscardDirtyFlagForElements<CUIElementsProcessSystem::TGridGroupsContext>(CUIElementsProcessSystem::TGridGroupsContext& context)
	{
		TDE2_PROFILER_SCOPE("DiscardDirtyFlagForElements");

		for (USIZE i = 0; i < context.mpGridGroupLayouts.size(); ++i)
		{
			if (auto pGridLayout = context.mpGridGroupLayouts[i])
			{
				pGridLayout->SetDirty(false);
			}
		}
	}


	static inline void ComputeImagesMeshes(const CUIElementsProcessSystem::TUIRenderableElementsContext<CImage>& imagesContext, IResourceManager* pResourceManager)
	{
		TDE2_PROFILER_SCOPE("ComputeImagesMeshes");

		for (USIZE i = 0; i < imagesContext.mpTransforms.size(); ++i)
		{
			ComputeImageMeshData(pResourceManager, imagesContext, i);
		}
	}


	static inline void ComputeSlicedImagesMeshes(const CUIElementsProcessSystem::TUIRenderableElementsContext<C9SliceImage>& slicedImagesContext, IResourceManager* pResourceManager)
	{
		TDE2_PROFILER_SCOPE("ComputeSlicedImagesMeshes");

		for (USIZE i = 0; i < slicedImagesContext.mpTransforms.size(); ++i)
		{
			Compute9SliceImageMeshData(pResourceManager, slicedImagesContext, i);
		}
	}

	
	static inline void ComputeLabelsMeshes(const CUIElementsProcessSystem::TUIRenderableElementsContext<CLabel>& labelsContext, IResourceManager* pResourceManager)
	{
		TDE2_PROFILER_SCOPE("ComputeLabelsMeshes");

		for (USIZE i = 0; i < labelsContext.mpTransforms.size(); ++i)
		{
			ComputeTextMeshData(pResourceManager, labelsContext, i);
		}
	}


	void CUIElementsProcessSystem::Update(IWorld* pWorld, F32 dt)
	{
		TDE2_PROFILER_SCOPE("CUIElementsProcessSystem::Update");

		if (mLayoutElementsContext.mpLayoutElements.empty())
		{
			return;
		}

		/// \note Update canvas entities
		for (USIZE i = 0; i < mCanvasesContext.mpTransforms.size(); ++i)
		{
			if (UpdateCanvasData(mpGraphicsContext, mCanvasesContext, i))
			{
				UpdateCanvasLayoutElementData(pWorld, mCanvasesContext, i);
			}
		}

		UpdateLayoutElements(mLayoutElementsContext, pWorld); /// \note Process LayoutElement entities

		ComputeImagesMeshes(mImagesContext, mpResourceManager);
		ComputeSlicedImagesMeshes(mSlicedImagesContext, mpResourceManager);
		ComputeLabelsMeshes(mLabelsContext, mpResourceManager);

		DiscardDirtyFlagForElements(mLayoutElementsContext);
		DiscardDirtyFlagForElements(mCanvasesContext);

		UpdateGridGroupLayoutElements(mGridGroupLayoutsContext, mLayoutElementsContext, pWorld);
		DiscardDirtyFlagForElements(mGridGroupLayoutsContext);
	}


	TDE2_API ISystem* CreateUIElementsProcessSystem(IGraphicsContext* pGraphicsContext, IResourceManager* pResourceManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem, CUIElementsProcessSystem, result, pGraphicsContext, pResourceManager);
	}
}