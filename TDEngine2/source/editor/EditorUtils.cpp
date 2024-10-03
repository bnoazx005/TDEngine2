#include "../../include/editor/EditorUtils.h"

#if TDE2_EDITORS_ENABLED

#include "../../include/graphics/IDebugUtility.h"
#include "../../include/ecs/CWorld.h"
#include "../../include/ecs/CUIEventsSystem.h"
#include "../../include/platform/IOStreams.h"
#include "../../include/platform/CYAMLFile.h"
#include "../../include/scene/IPrefabsRegistry.h"
#include "../../include/scene/IScene.h"
#include "../../include/graphics/UI/CLayoutElementComponent.h"
#include "../../include/graphics/UI/CInputReceiverComponent.h"
#include "../../include/graphics/UI/CToggleComponent.h"
#include "../../include/graphics/UI/CImageComponent.h"
#include "../../include/graphics/UI/C9SliceImageComponent.h"
#include "../../include/graphics/UI/CUISliderComponent.h"
#include "../../include/graphics/UI/CInputFieldComponent.h"
#include "../../include/graphics/UI/CCanvasComponent.h"
#include "../../include/graphics/UI/CLabelComponent.h"
#include "../../include/graphics/UI/CScrollableUIAreaComponent.h"
#include "../../include/graphics/UI/CDropDownComponent.h"
#include "../../include/graphics/UI/GroupLayoutComponents.h"
#include "../../include/core/IFont.h"
#include "../../include/core/IImGUIContext.h"
#include "../../include/editor/ecs/EditorComponents.h"
#include "../../include/editor/CPerfProfiler.h"
#include <clip.h>


namespace TDEngine2
{
	void DrawEditorGrid(IDebugUtility* pDebugUtility, I32 rows, I32 cols, F32 cellSize)
	{
		const F32 halfWidth = 0.5f * cols * cellSize;
		const F32 halfHeight = 0.5f * rows * cellSize;

		for (I32 i = rows / 2; i >= -rows / 2; --i)
		{
			if (!i)
			{
				continue;
			}

			pDebugUtility->DrawLine(TVector3(-halfWidth, 0.0f, i * cellSize), TVector3(halfWidth, 0.0f, i * cellSize), TColorUtils::mWhite);
		}

		for (I32 i = cols / 2; i >= -cols / 2; --i)
		{
			if (!i)
			{
				continue;
			}

			pDebugUtility->DrawLine(TVector3(i * cellSize, 0.0f, halfHeight), TVector3(i * cellSize, 0.0f, -halfHeight), TColorUtils::mWhite);
		}

		/// \note Center lines

		pDebugUtility->DrawLine(TVector3(-halfWidth, 0.0f, 0.0f), TVector3(halfWidth, 0.0f, 0.0f), TColorUtils::mRed);
		pDebugUtility->DrawLine(TVector3(0.0f, 0.0f, -halfHeight), TVector3(0.0f, 0.0f, halfHeight), TColorUtils::mRed);
	}


	static const std::string DefaultSpritePathId = "ProjectResources/Textures/DefaultSprite.tga";


	E_RESULT_CODE CSceneHierarchyUtils::CopyEntitiesHierarchy(TPtr<IPrefabsRegistry> pPrefabsRegistry, TPtr<IWorld> pWorld, TEntityId entityId)
	{
		E_RESULT_CODE result = RC_OK;

		if (!pPrefabsRegistry || !pWorld || TEntityId::Invalid == entityId)
		{
			return RC_INVALID_ARGS;
		}

		auto pMemoryMappedStream = TPtr<IStream>(CreateMemoryIOStream(Wrench::StringUtils::GetEmptyStr(), {}, result));
		if (!pMemoryMappedStream)
		{
			return result;
		}

		IYAMLFileWriter* pFileWriter = dynamic_cast<IYAMLFileWriter*>(CreateYAMLFileWriter(nullptr, pMemoryMappedStream, result));
		if (!pFileWriter)
		{
			return result;
		}

		result = result | pPrefabsRegistry->SavePrefabHierarchy(pFileWriter, pWorld.Get(), pWorld->FindEntity(entityId));
		result = result | pFileWriter->Close();

		if (auto pInputSream = DynamicPtrCast<IInputStream>(pMemoryMappedStream))
		{
			if (!clip::set_text(pInputSream->ReadToEnd()))
			{
				return RC_FAIL;
			}

			return result;
		}
		
		return RC_FAIL;
	}

	TResult<TEntityId> CSceneHierarchyUtils::PasteEntitiesHierarchy(TPtr<IPrefabsRegistry> pPrefabsRegistry, TPtr<IWorld> pWorld, IScene* pCurrScene, TEntityId parentEntityId, const TEntityOperation& op)
	{
		if (!pPrefabsRegistry || !pWorld)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}

		if (!clip::has(clip::text_format())) /// \note Nothing to paste because of the empty clipboard
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		E_RESULT_CODE result = RC_OK;

		std::string clipboardData;
		
		if (!clip::get_text(clipboardData))
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		std::vector<U8> serializedBuffer;
		std::transform(clipboardData.begin(), clipboardData.end(), std::back_inserter(serializedBuffer), [](const C8 ch) { return static_cast<U8>(ch); });

		auto pMemoryMappedStream = TPtr<IStream>(CreateMemoryIOStream(Wrench::StringUtils::GetEmptyStr(), serializedBuffer, result));
		if (!pMemoryMappedStream)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		IYAMLFileReader* pFileReader = dynamic_cast<IYAMLFileReader*>(CreateYAMLFileReader(nullptr, pMemoryMappedStream, result));
		if (!pFileReader)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		auto&& duplicateRootEntityInfo = pPrefabsRegistry->LoadPrefabHierarchy(pFileReader, pWorld.Get(), pWorld->GetEntityManager(),
			[pCurrScene](TEntityId id)
			{ 
				return pCurrScene->CreateEntity(Wrench::StringUtils::GetEmptyStr()); 
			},
			[pCurrScene](TEntityId linkId, const std::string& prefabId, CEntity* pParentEntity) 
			{
				return pCurrScene->Spawn(prefabId, pParentEntity, TEntityId::Invalid);
			});

		result = result | GroupEntities(pWorld.Get(), parentEntityId, duplicateRootEntityInfo.mRootEntityId);
		result = result | pFileReader->Close();

		if (RC_OK != result)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		if (op)
		{
			op(duplicateRootEntityInfo.mRootEntityId);
		}

		return Wrench::TOkValue<TEntityId>(duplicateRootEntityInfo.mRootEntityId);
	}


	static TResult<TEntityId> CreateNewEntityInternal(const std::string& id, TPtr<IWorld> pWorld, IScene* pCurrScene, TEntityId parentEntityId, const CSceneHierarchyUtils::TEntityOperation& op)
	{
		if (!pWorld || !pCurrScene)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}

		/// \note Also a new created entity becomes selected at the same time
		if (CEntity* pEntity = pCurrScene->CreateEntity(id))
		{
			if (TEntityId::Invalid != parentEntityId)
			{
				GroupEntities(pWorld.Get(), parentEntityId, pEntity->GetId());
			}

			op(pEntity->GetId());

			return Wrench::TOkValue<TEntityId>(pEntity->GetId());
		}

		return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
	}


	TResult<TEntityId> CSceneHierarchyUtils::CreateNewEntity(TPtr<IWorld> pWorld, IScene* pCurrScene, TEntityId parentEntityId, const TEntityOperation& op)
	{
		return CreateNewEntityInternal("NewEntity", pWorld, pCurrScene, parentEntityId, op);
	}

	
	static void Setup9ImageSliceComponent(CEntity* pEntity, const std::string& imageId, const TColor32F& color = TColorUtils::mWhite)
	{
		auto pImage = pEntity->AddComponent<C9SliceImage>();
		if (!pImage)
		{
			return;
		}

		pImage->SetImageId(imageId);
		pImage->SetLeftXSlicer(0.25f);
		pImage->SetRightXSlicer(0.75f);
		pImage->SetBottomYSlicer(0.25f);
		pImage->SetTopYSlicer(0.75f);
		pImage->SetColor(color);
	}


	static void SetupImageComponent(CEntity* pEntity, const std::string& imageId, const TColor32F& color = TColorUtils::mWhite)
	{
		auto pImage = pEntity->AddComponent<CImage>();
		if (!pImage)
		{
			return;
		}

		pImage->SetImageId(imageId);
		pImage->SetColor(color);
	}


	TResult<TEntityId> CSceneHierarchyUtils::CreateToggleUIElement(TPtr<IWorld> pWorld, IScene* pCurrScene, TEntityId parentEntityId, const TEntityOperation& op)
	{
		auto rootEntityResult = CreateNewEntityInternal("Toggle", pWorld, pCurrScene, parentEntityId, op);
		if (rootEntityResult.HasError())
		{
			return rootEntityResult;
		}

		if (auto pToggleEntity = pWorld->FindEntity(rootEntityResult.Get()))
		{
			pToggleEntity->AddComponent<CInputReceiver>();
			
			if (auto pLayoutElement = pToggleEntity->AddComponent<CLayoutElement>())
			{
				pLayoutElement->SetMinAnchor(ZeroVector2);
				pLayoutElement->SetMaxAnchor(ZeroVector2);
				pLayoutElement->SetMinOffset(ZeroVector2);
				pLayoutElement->SetMaxOffset(TVector2(35.0f));
			}
			
			Setup9ImageSliceComponent(pToggleEntity, DefaultSpritePathId);

			auto pMarkerEntity = pCurrScene->CreateEntity("Marker");
			{
				if (auto pLayoutElement = pMarkerEntity->AddComponent<CLayoutElement>())
				{
					pLayoutElement->SetMinAnchor(ZeroVector2);
					pLayoutElement->SetMaxAnchor(TVector2(1.0f));
					pLayoutElement->SetMinOffset(TVector2(5.0f));
					pLayoutElement->SetMaxOffset(TVector2(5.0f));
				}

				Setup9ImageSliceComponent(pMarkerEntity, DefaultSpritePathId, TColorUtils::mBlack);
			}

			if (auto pToggle = pToggleEntity->AddComponent<CToggle>())
			{
				pToggle->SetMarkerEntityId(pMarkerEntity->GetId());
			}

			GroupEntities(pWorld.Get(), pToggleEntity->GetId(), pMarkerEntity->GetId());
		}

		return rootEntityResult;
	}

	TResult<TEntityId> CSceneHierarchyUtils::CreateLabelElement(TPtr<IWorld> pWorld, IScene* pCurrScene, TEntityId parentEntityId, const TEntityOperation& op)
	{
		auto rootEntityResult = CreateNewEntityInternal("Text", pWorld, pCurrScene, parentEntityId, op);
		if (rootEntityResult.HasError())
		{
			return rootEntityResult;
		}

		if (auto pLabelEntity = pWorld->FindEntity(rootEntityResult.Get()))
		{
			if (auto pLayoutElement = pLabelEntity->AddComponent<CLayoutElement>())
			{
				pLayoutElement->SetMinAnchor(ZeroVector2);
				pLayoutElement->SetMaxAnchor(TVector2(1.0f));
				pLayoutElement->SetMinOffset(ZeroVector2);
				pLayoutElement->SetMaxOffset(ZeroVector2);
			}

			if (auto pLabel = pLabelEntity->AddComponent<CLabel>())
			{
				pLabel->SetFontId("OpenSans.font");
				pLabel->SetTextHeight(24);
				pLabel->SetColor(TColorUtils::mBlack);
				pLabel->SetAlignType(E_FONT_ALIGN_POLICY::LEFT_CENTER);
				pLabel->SetText(Wrench::StringUtils::GetEmptyStr());
			}
		}

		return rootEntityResult;
	}

	TResult<TEntityId> CSceneHierarchyUtils::CreateImageUIElement(TPtr<IWorld> pWorld, IScene* pCurrScene, TEntityId parentEntityId, const TEntityOperation& op)
	{
		auto rootEntityResult = CreateNewEntityInternal("Image", pWorld, pCurrScene, parentEntityId, op);
		if (rootEntityResult.HasError())
		{
			return rootEntityResult;
		}

		if (auto pImageEntity = pWorld->FindEntity(rootEntityResult.Get()))
		{
			if (auto pLayoutElement = pImageEntity->AddComponent<CLayoutElement>())
			{
				pLayoutElement->SetMinAnchor(ZeroVector2);
				pLayoutElement->SetMaxAnchor(ZeroVector2);
				pLayoutElement->SetMinOffset(ZeroVector2);
				pLayoutElement->SetMaxOffset(TVector2(100.0f));
			}

			SetupImageComponent(pImageEntity, DefaultSpritePathId, TColorUtils::mWhite);
		}

		return rootEntityResult;
	}

	TResult<TEntityId> CSceneHierarchyUtils::Create9SliceImageUIElement(TPtr<IWorld> pWorld, IScene* pCurrScene, TEntityId parentEntityId, const TEntityOperation& op)
	{
		auto rootEntityResult = CreateNewEntityInternal("9SlicedImage", pWorld, pCurrScene, parentEntityId, op);
		if (rootEntityResult.HasError())
		{
			return rootEntityResult;
		}

		if (auto pImageEntity = pWorld->FindEntity(rootEntityResult.Get()))
		{
			if (auto pLayoutElement = pImageEntity->AddComponent<CLayoutElement>())
			{
				pLayoutElement->SetMinAnchor(ZeroVector2);
				pLayoutElement->SetMaxAnchor(ZeroVector2);
				pLayoutElement->SetMinOffset(ZeroVector2);
				pLayoutElement->SetMaxOffset(TVector2(100.0f));
			}

			Setup9ImageSliceComponent(pImageEntity, DefaultSpritePathId, TColorUtils::mWhite);
		}

		return rootEntityResult;
	}

	TResult<TEntityId> CSceneHierarchyUtils::CreateCanvasUIElement(TPtr<IWorld> pWorld, IScene* pCurrScene, TEntityId parentEntityId, const TEntityOperation& op)
	{
		auto rootEntityResult = CreateNewEntityInternal("Canvas", pWorld, pCurrScene, parentEntityId, op);
		if (rootEntityResult.HasError())
		{
			return rootEntityResult;
		}

		if (auto pCanvasEntity = pWorld->FindEntity(rootEntityResult.Get()))
		{
			if (auto pCanvas = pCanvasEntity->AddComponent<CCanvas>())
			{
				pCanvas->SetInheritSizesFromMainCamera(true);
			}
		}

		return rootEntityResult;
	}

	TResult<TEntityId> CSceneHierarchyUtils::CreateSliderUIElement(TPtr<IWorld> pWorld, IScene* pCurrScene, TEntityId parentEntityId, const TEntityOperation& op)
	{
		static constexpr F32 sliderHeight = 15.0f;

		auto rootEntityResult = CreateNewEntityInternal("Slider", pWorld, pCurrScene, parentEntityId, op);
		if (rootEntityResult.HasError())
		{
			return rootEntityResult;
		}

		if (auto pSliderEntity = pWorld->FindEntity(rootEntityResult.Get()))
		{
			pSliderEntity->AddComponent<CInputReceiver>();

			if (auto pLayoutElement = pSliderEntity->AddComponent<CLayoutElement>())
			{
				pLayoutElement->SetMinAnchor(ZeroVector2);
				pLayoutElement->SetMaxAnchor(ZeroVector2);
				pLayoutElement->SetMinOffset(ZeroVector2);
				pLayoutElement->SetMaxOffset(TVector2(100.0f, sliderHeight));
			}

			Setup9ImageSliceComponent(pSliderEntity, DefaultSpritePathId, TColor32F(0.75f));

			auto pMarkerEntity = pCurrScene->CreateEntity("Marker");
			{
				if (auto pLayoutElement = pMarkerEntity->AddComponent<CLayoutElement>())
				{
					pLayoutElement->SetMinAnchor(ZeroVector2);
					pLayoutElement->SetMaxAnchor(ZeroVector2);
					pLayoutElement->SetMinOffset(ZeroVector2);
					pLayoutElement->SetMaxOffset(TVector2(sliderHeight + 4.0f));
				}

				Setup9ImageSliceComponent(pMarkerEntity, DefaultSpritePathId, TColorUtils::mWhite);
			}

			if (auto pSlider = pSliderEntity->AddComponent<CUISlider>())
			{
				pSlider->SetMarkerEntityId(pMarkerEntity->GetId());
				pSlider->SetValue(0.5f);
			}

			GroupEntities(pWorld.Get(), pSliderEntity->GetId(), pMarkerEntity->GetId());
		}

		return rootEntityResult;
	}

	TResult<TEntityId> CSceneHierarchyUtils::CreateInputFieldUIElement(TPtr<IWorld> pWorld, IScene* pCurrScene, TEntityId parentEntityId, const TEntityOperation& op)
	{
		static constexpr F32 sliderHeight = 32.0f;

		auto rootEntityResult = CreateNewEntityInternal("InputField", pWorld, pCurrScene, parentEntityId, op);
		if (rootEntityResult.HasError())
		{
			return rootEntityResult;
		}

		if (auto pInputFieldEntity = pWorld->FindEntity(rootEntityResult.Get()))
		{
			pInputFieldEntity->AddComponent<CInputReceiver>();

			if (auto pLayoutElement = pInputFieldEntity->AddComponent<CLayoutElement>())
			{
				pLayoutElement->SetMinAnchor(ZeroVector2);
				pLayoutElement->SetMaxAnchor(ZeroVector2);
				pLayoutElement->SetMinOffset(ZeroVector2);
				pLayoutElement->SetMaxOffset(TVector2(250.0f, sliderHeight));
			}

			Setup9ImageSliceComponent(pInputFieldEntity, DefaultSpritePathId, TColor32F(0.75f));

			auto pCursorEntity = pCurrScene->CreateEntity("Cursor");
			{
				if (auto pLayoutElement = pCursorEntity->AddComponent<CLayoutElement>())
				{
					pLayoutElement->SetMinAnchor(ZeroVector2);
					pLayoutElement->SetMaxAnchor(TVector2(0.0f, 1.0f));
					pLayoutElement->SetMinOffset(ZeroVector2);
					pLayoutElement->SetMaxOffset(TVector2(2.0f, 0.0f));
				}

				SetupImageComponent(pCursorEntity, DefaultSpritePathId, TColorUtils::mBlack);
			}

			auto&& labelResult = CreateLabelElement(pWorld, pCurrScene, pInputFieldEntity->GetId(), [](auto) {});

			if (auto pInputField = pInputFieldEntity->AddComponent<CInputField>())
			{
				pInputField->SetCursorEntityId(pCursorEntity->GetId());
				pInputField->SetLabelEntityId(labelResult.Get());
			}

			GroupEntities(pWorld.Get(), pInputFieldEntity->GetId(), pCursorEntity->GetId());
		}

		return rootEntityResult;
	}

	TResult<TEntityId> CSceneHierarchyUtils::CreateScrollUIArea(TPtr<IWorld> pWorld, IScene* pCurrScene, TEntityId parentEntityId, const TEntityOperation& op)
	{
		auto rootEntityResult = CreateNewEntityInternal("ScrollArea", pWorld, pCurrScene, parentEntityId, op);
		if (rootEntityResult.HasError())
		{
			return rootEntityResult;
		}

		if (auto pScrollableEntity = pWorld->FindEntity(rootEntityResult.Get()))
		{
			pScrollableEntity->AddComponent<CInputReceiver>();

			if (auto pLayoutElement = pScrollableEntity->AddComponent<CLayoutElement>())
			{
				pLayoutElement->SetMinAnchor(ZeroVector2);
				pLayoutElement->SetMaxAnchor(ZeroVector2);
				pLayoutElement->SetMinOffset(ZeroVector2);
				pLayoutElement->SetMaxOffset(TVector2(400.0f));
			}

			auto pContentEntity = pCurrScene->CreateEntity("Content");
			{
				if (auto pLayoutElement = pContentEntity->AddComponent<CLayoutElement>())
				{
					pLayoutElement->SetMinAnchor(ZeroVector2);
					pLayoutElement->SetMaxAnchor(ZeroVector2);
					pLayoutElement->SetMinOffset(ZeroVector2);
					pLayoutElement->SetMaxOffset(TVector2(400.0f));
				}
			}

			if (auto pScrollArea = pScrollableEntity->AddComponent<CScrollableUIArea>())
			{
				pScrollArea->SetContentEntityId(pContentEntity->GetId());
			}

			pScrollableEntity->AddComponent<CUIMaskComponent>();

			Setup9ImageSliceComponent(pScrollableEntity, DefaultSpritePathId, TColor32F(0.0f));

			GroupEntities(pWorld.Get(), pScrollableEntity->GetId(), pContentEntity->GetId());
		}

		return rootEntityResult;
	}

	TResult<TEntityId> CSceneHierarchyUtils::CreateDropDownUIElement(TPtr<IWorld> pWorld, IScene* pCurrScene, TEntityId parentEntityId, const TEntityOperation& op)
	{
		static constexpr F32 elementHeight = 32.0f;

		auto rootEntityResult = CreateNewEntityInternal("DropDown", pWorld, pCurrScene, parentEntityId, op);
		if (rootEntityResult.HasError())
		{
			return rootEntityResult;
		}

		if (auto pDropDownEntity = pWorld->FindEntity(rootEntityResult.Get()))
		{
			pDropDownEntity->AddComponent<CInputReceiver>();

			if (auto pLayoutElement = pDropDownEntity->AddComponent<CLayoutElement>())
			{
				pLayoutElement->SetMinAnchor(ZeroVector2);
				pLayoutElement->SetMaxAnchor(ZeroVector2);
				pLayoutElement->SetMinOffset(ZeroVector2);
				pLayoutElement->SetMaxOffset(TVector2(250.0f, elementHeight));
			}

			Setup9ImageSliceComponent(pDropDownEntity, DefaultSpritePathId, TColor32F(0.75f));

			TEntityId contentId = TEntityId::Invalid;

			auto pContentEntity = pCurrScene->CreateEntity("Content");
			{
				if (auto pLayoutElement = pContentEntity->AddComponent<CLayoutElement>())
				{
					pLayoutElement->SetMinAnchor(ZeroVector2);
					pLayoutElement->SetMaxAnchor(TVector2(1.0f, 0.0f));
					pLayoutElement->SetMinOffset(TVector2(0.0f, -120.0f));
					pLayoutElement->SetMaxOffset(TVector2(0.0f, 10.0f));
				}

				Setup9ImageSliceComponent(pContentEntity, DefaultSpritePathId, TColor32F(0.75f));

				auto&& scrollerResult = CreateScrollUIArea(pWorld, pCurrScene, pContentEntity->GetId(), [](auto) {});

				if (auto pScrollerEntity = pWorld->FindEntity(scrollerResult.Get()))
				{
					if (auto pScroller = pScrollerEntity->GetComponent<CScrollableUIArea>())
					{
						pScroller->SetNormalizedScrollPosition(TVector2(0.0f, 1.0f));
					}

					if (auto pScrollerLayoutElement = pScrollerEntity->GetComponent<CLayoutElement>())
					{
						pScrollerLayoutElement->SetMinAnchor(ZeroVector2);
						pScrollerLayoutElement->SetMaxAnchor(TVector2(1.0f));
						pScrollerLayoutElement->SetMinOffset(ZeroVector2);
						pScrollerLayoutElement->SetMaxOffset(ZeroVector2);
					}

					if (auto pInputReceiver = pScrollerEntity->GetComponent<CInputReceiver>())
					{
						pInputReceiver->mIsInputBypassEnabled = true;
					}

					if (auto pScrollerContentEntity = pWorld->FindEntity(pScrollerEntity->GetComponent<CTransform>()->GetChildren().front()))
					{
						contentId = pScrollerContentEntity->GetId();

						if (auto pGridGroupLayout = pScrollerContentEntity->AddComponent<CGridGroupLayout>())
						{
							pGridGroupLayout->SetElementsAlignType(E_UI_ELEMENT_ALIGNMENT_TYPE::LEFT_TOP);
							pGridGroupLayout->SetCellSize(TVector2(256.0f, elementHeight)); /// \todo Replace the value later
						}

						if (auto pGridLayoutElement = pScrollerContentEntity->GetComponent<CLayoutElement>())
						{
							pGridLayoutElement->SetMinAnchor(TVector2(0.0f, 1.0f));
							pGridLayoutElement->SetMaxAnchor(TVector2(1.0f));
							pGridLayoutElement->SetMinOffset(ZeroVector2);
							pGridLayoutElement->SetMaxOffset(TVector2(256.0f, 300.0f));
						}
					}
				}
			}

			/// \note Option's item template
			auto&& itemPrefabResult = CreateLabelElement(pWorld, pCurrScene, pDropDownEntity->GetId(), [](auto) {});
			if (auto pItemPrefabEntity = pWorld->FindEntity(itemPrefabResult.Get()))
			{
				pItemPrefabEntity->AddComponent<CInputReceiver>();

				SetEntityActive(pWorld.Get(), pItemPrefabEntity->GetId(), false);
			}

			GroupEntities(pWorld.Get(), pDropDownEntity->GetId(), pContentEntity->GetId());

			if (auto pDropDown = pDropDownEntity->AddComponent<CDropDown>())
			{
				auto&& labelResult = CreateLabelElement(pWorld, pCurrScene, pDropDownEntity->GetId(), [](auto) {});
				pDropDown->SetLabelEntityId(labelResult.Get());

				pDropDown->SetPopupRootEntityId(pContentEntity->GetId());
				pDropDown->SetContentEntityId(contentId);
				pDropDown->SetItemPrefabEntityId(itemPrefabResult.Get());
			}
		}

		return rootEntityResult;
	}

	TResult<TEntityId> CSceneHierarchyUtils::CreateButtonUIElement(TPtr<IWorld> pWorld, IScene* pCurrScene, TEntityId parentEntityId, const TEntityOperation& op)
	{
		static constexpr F32 elementHeight = 32.0f;

		auto rootEntityResult = CreateNewEntityInternal("Button", pWorld, pCurrScene, parentEntityId, op);
		if (rootEntityResult.HasError())
		{
			return rootEntityResult;
		}

		if (auto pButtonEntity = pWorld->FindEntity(rootEntityResult.Get()))
		{
			pButtonEntity->AddComponent<CInputReceiver>();

			if (auto pLayoutElement = pButtonEntity->AddComponent<CLayoutElement>())
			{
				pLayoutElement->SetMinAnchor(ZeroVector2);
				pLayoutElement->SetMaxAnchor(ZeroVector2);
				pLayoutElement->SetMinOffset(ZeroVector2);
				pLayoutElement->SetMaxOffset(TVector2(250.0f, elementHeight));
			}

			Setup9ImageSliceComponent(pButtonEntity, DefaultSpritePathId, TColor32F(0.75f));

			auto&& labelResult = CreateLabelElement(pWorld, pCurrScene, pButtonEntity->GetId(), [](auto) {});
			if (auto pLabelEntity = pWorld->FindEntity(labelResult.Get()))
			{
				if (auto pLabel = pLabelEntity->GetComponent<CLabel>())
				{
					pLabel->SetText("New Button");
					pLabel->SetAlignType(E_FONT_ALIGN_POLICY::CENTER);
				}
			}
		}

		return rootEntityResult;
	}


	/*!
		\brief CImGUIExtensions's definition
	*/

	E_RESULT_CODE CImGUIExtensions::EntityRefField(IImGUIContext& imGUIContext, IWorld& world, const std::string& text, TEntityId& entityRef, const std::function<void()>& onValueChanged)
	{
		static const std::string InvalidEntityValue = "None";

		CEntity* pEntity = world.FindEntity(entityRef);
		std::string value = pEntity ? pEntity->GetName() : InvalidEntityValue;

		imGUIContext.TextField(text, value, nullptr, nullptr, false, true);

		imGUIContext.RegisterDragAndDropTarget([&imGUIContext, &world, &entityRef, onValueChanged]
		{
			if (auto pChildEntityId = imGUIContext.GetDragAndDropData<TEntityId>("DRAGGED_ENTITY_ID"))
			{
				entityRef = *pChildEntityId;

				if (onValueChanged)
				{
					onValueChanged();
				}
			}
		});

		return RC_OK;
	}

	E_RESULT_CODE CImGUIExtensions::EntityRefField(IImGUIContext& imGUIContext, IWorld& world, const std::string& text, CEntityRef& entityRef, const std::function<void()>& onValueChanged)
	{
		TEntityId resolvedRefId = entityRef.Get();

		E_RESULT_CODE result = EntityRefField(imGUIContext, world, text, resolvedRefId, [onValueChanged, &entityRef, &resolvedRefId]
		{
			if (onValueChanged)
			{
				onValueChanged();
			}

			entityRef.Set(resolvedRefId);
		});

		return result;
	}

	E_RESULT_CODE CImGUIExtensions::EntityRefField(TPtr<IImGUIContext> pImGUIContext, TPtr<IWorld> pWorld, const std::string& text, TEntityId& entityRef, const std::function<void()>& onValueChanged)
	{
		if (!pImGUIContext || !pWorld)
		{
			return RC_INVALID_ARGS;
		}

		return EntityRefField(*pImGUIContext.Get(), *pWorld.Get(), text, entityRef, onValueChanged);
	}

	E_RESULT_CODE CImGUIExtensions::EntityRefField(TPtr<IImGUIContext> pImGUIContext, TPtr<IWorld> pWorld, const std::string& text, CEntityRef& entityRef, const std::function<void()>& onValueChanged)
	{
		if (!pImGUIContext || !pWorld)
		{
			return RC_INVALID_ARGS;
		}

		return EntityRefField(*pImGUIContext.Get(), *pWorld.Get(), text, entityRef, onValueChanged);
	}


	bool CSnapGuidesContainer::TSnapGuideline::IsHorizontal() const
	{
		return CMathUtils::Abs(Dot(mEnd - mStart, RightVector2)) > 0.0f;
	}

	bool CSnapGuidesContainer::TSnapGuideline::IsVertical() const
	{
		return CMathUtils::Abs(Dot(mEnd - mStart, UpVector2)) > 0.0f;
	}


	/*!
		\brief CSnapGuidesContainer's definition
	*/

	void CSnapGuidesContainer::UpdateGuidelines(TPtr<IWorld> pWorld)
	{
		TDE2_PROFILER_SCOPE("CSnapGuidesContainer::UpdateGuidelines");

		if (!pWorld)
		{
			return;
		}

		mSnapGuides.clear();

		for (TEntityId currCanvasEntity : FindMainCanvases(pWorld.Get()))
		{
			CTransform* pTransform = pWorld->FindEntity(currCanvasEntity)->GetComponent<CTransform>();

			std::stack<TEntityId> entitiesToVisit;

			entitiesToVisit.emplace(currCanvasEntity);

			CEntity* pEntity = nullptr;
			TEntityId currEntityId = TEntityId::Invalid;
			
			while (!entitiesToVisit.empty())
			{
				currEntityId = entitiesToVisit.top();
				entitiesToVisit.pop();

				pEntity = pWorld->FindEntity(currEntityId);
				if (pEntity->HasComponent<CSelectedEntityComponent>())
				{
					continue;
				}

				{
					CLayoutElement* pLayoutElement = pEntity->GetComponent<CLayoutElement>();
					if (!pLayoutElement)
					{
						continue;
					}

					const auto& worldRect = pLayoutElement->GetWorldRect();
					if (Length(worldRect.GetSizes()) < FloatEpsilon)
					{
						continue;
					}

					const auto& rectPoints = worldRect.GetPoints();

					for (USIZE i = 0; i < rectPoints.size(); i++)
					{
						mSnapGuides.push_back({ rectPoints[i], rectPoints[(i + 1) % rectPoints.size()] });
					}
				}

				if (pTransform = pEntity->GetComponent<CTransform>())
				{
					for (TEntityId id : pTransform->GetChildren())
					{
						entitiesToVisit.emplace(id);
					}
				}
			}
		}
	}

	std::vector<CSnapGuidesContainer::TSnapGuideline> CSnapGuidesContainer::GetNearestSnapGuides(const TVector2& point, F32 threshold) const
	{
		std::vector<TSnapGuideline> result;

		bool isHorizontalFound = false;
		bool isVerticalFound = false;

		for (auto&& currSnapGuideline : mSnapGuides)
		{
			if (currSnapGuideline.mStart == point || currSnapGuideline.mEnd == point)
			{
				continue;
			}

			const TVector2 dir = Normalize(currSnapGuideline.mEnd - currSnapGuideline.mStart);
			const TVector2 normal(dir.y, dir.x);

			const F32 dist = Length(Dot(normal, (point - currSnapGuideline.mStart)) * normal);
			
			if (dist < threshold && (!isHorizontalFound || !isVerticalFound))
			{
				if ((currSnapGuideline.IsHorizontal() && isHorizontalFound) || (currSnapGuideline.IsVertical() && isVerticalFound))
				{
					continue;
				}

				result.push_back(currSnapGuideline);

				if (currSnapGuideline.IsHorizontal())
				{
					isHorizontalFound = true;
				}
				else
				{
					isVerticalFound = true;
				}
			}
		}

		return std::move(result);
	}

	const std::vector<CSnapGuidesContainer::TSnapGuideline>& CSnapGuidesContainer::GetSnapGuides() const
	{
		return mSnapGuides;
	}
}

#endif