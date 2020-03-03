#include "./../../include/ecs/CObjectsSelectionSystem.h"
#include "./../../include/graphics/IRenderer.h"
#include "./../../include/graphics/IGraphicsObjectManager.h"
#include "./../../include/graphics/CRenderQueue.h"
#include "./../../include/graphics/COrthoCamera.h"
#include "./../../include/graphics/CPerspectiveCamera.h"
#include "./../../include/graphics/CStaticMeshContainer.h"
#include "./../../include/graphics/CQuadSprite.h"
#include "./../../include/core/IResourceManager.h"
#include "./../../include/ecs/CTransform.h"
#include "./../../include/ecs/IWorld.h"


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

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CObjectsSelectionSystem::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
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
		TDE2_ASSERT(mCameraEntityId != InvalidEntityId);

		// get camera and test all entities for their visibility
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