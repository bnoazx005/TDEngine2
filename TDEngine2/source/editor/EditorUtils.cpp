#include "../../include/editor/EditorUtils.h"
#include "../../include/graphics/IDebugUtility.h"
#include "../../include/ecs/CWorld.h"
#include "../../include/platform/IOStreams.h"
#include "../../include/platform/CYAMLFile.h"
#include "../../include/scene/IPrefabsRegistry.h"
#include "../../include/scene/IScene.h"
#include "../../include/core/IImGUIContext.h"
#include <clip.h>


#if TDE2_EDITORS_ENABLED

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


	E_RESULT_CODE CEntitiesCommands::CopyEntitiesHierarchy(TPtr<IPrefabsRegistry> pPrefabsRegistry, TPtr<IWorld> pWorld, TEntityId entityId)
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

	TResult<TEntityId> CEntitiesCommands::PasteEntitiesHierarchy(TPtr<IPrefabsRegistry> pPrefabsRegistry, TPtr<IWorld> pWorld, IScene* pCurrScene, TEntityId parentEntityId)
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

		auto&& duplicateRootEntityInfo = pPrefabsRegistry->LoadPrefabHierarchy(pFileReader, pWorld->GetEntityManager(), 
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

		return Wrench::TOkValue<TEntityId>(duplicateRootEntityInfo.mRootEntityId);
	}


	/*!
		\brief CImGUIExtensions's definition
	*/

	E_RESULT_CODE CImGUIExtensions::EntityRefField(TPtr<IImGUIContext> pImGUIContext, TPtr<IWorld> pWorld, const std::string& text, TEntityId& entityRef, const std::function<void()>& onValueChanged)
	{
		if (!pImGUIContext || !pWorld)
		{
			return RC_INVALID_ARGS;
		}

		static const std::string InvalidEntityValue = "None";

		CEntity* pEntity = pWorld->FindEntity(entityRef);
		std::string value = pEntity ? pEntity->GetName() : InvalidEntityValue;

		pImGUIContext->TextField(text, value, nullptr, nullptr, false, true);

		pImGUIContext->RegisterDragAndDropTarget([pImGUIContext, pWorld, &entityRef, onValueChanged]
		{
			if (auto pChildEntityId = pImGUIContext->GetDragAndDropData<TEntityId>("DRAGGED_ENTITY_ID"))
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
}

#endif