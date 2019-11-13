#include "./../../include/ecs/CStaticMeshRendererSystem.h"
#include "./../../include/graphics/IGraphicsObjectManager.h"
#include "./../../include/graphics/IRenderer.h"
#include "./../../include/graphics/CStaticMeshContainer.h"
#include "./../../include/graphics/CRenderQueue.h"
#include "./../../include/graphics/CBaseMaterial.h"
#include "./../../include/graphics/IVertexDeclaration.h"
#include "./../../include/graphics/CStaticMesh.h"
#include "./../../include/ecs/CTransform.h"
#include "./../../include/ecs/IWorld.h"
#include "./../../include/ecs/CEntity.h"
#include "./../../include/core/IResourceManager.h"
#include <algorithm>


namespace TDEngine2
{
	CStaticMeshRendererSystem::CStaticMeshRendererSystem() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CStaticMeshRendererSystem::Init(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pGraphicsObjectManager || !pRenderer)
		{
			return RC_INVALID_ARGS;
		}

		mpOpaqueRenderGroup      = pRenderer->GetRenderQueue(E_RENDER_QUEUE_GROUP::RQG_OPAQUE_GEOMETRY);
		mpTransparentRenderGroup = pRenderer->GetRenderQueue(E_RENDER_QUEUE_GROUP::RQG_TRANSPARENT_GEOMETRY);

		mpGraphicsObjectManager = pGraphicsObjectManager;

		mpResourceManager = pRenderer->GetResourceManager();

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CStaticMeshRendererSystem::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	void CStaticMeshRendererSystem::InjectBindings(IWorld* pWorld)
	{
		std::vector<TEntityId> entities = pWorld->FindEntitiesWithComponents<CTransform, CStaticMeshContainer>();

		mProcessingEntities.clear();

		CEntity* pCurrEntity = nullptr;

		for (auto iter = entities.begin(); iter != entities.end(); ++iter)
		{
			pCurrEntity = pWorld->FindEntity(*iter);

			if (!pCurrEntity)
			{
				continue;
			}

			mProcessingEntities.push_back({ pCurrEntity->GetComponent<CTransform>(), pCurrEntity->GetComponent<CStaticMeshContainer>() });
		}
	}

	void CStaticMeshRendererSystem::Update(IWorld* pWorld, F32 dt)
	{
		// \note first pass (construct an array of materials)
		// \note Materials: | {opaque_material_group1}, ..., {opaque_material_groupN} | {transp_material_group1}, ..., {transp_material_groupM} |
		_collectUsedMaterials(mProcessingEntities, mpResourceManager, mCurrMaterialsArray);

		auto firstTransparentMatIter = std::find_if(mCurrMaterialsArray.cbegin(), mCurrMaterialsArray.cend(), [](const IMaterial* pCurrMaterial)
		{
			return pCurrMaterial->IsTransparent();
		});
		
		// \note construct commands for opaque geometry
		std::for_each(mCurrMaterialsArray.cbegin(), firstTransparentMatIter, [this](const IMaterial* pCurrMaterial)
		{
			auto iter = mProcessingEntities.begin();

			// \note iterate over all entities with pCurrMaterial attached as main material
			while ((iter = std::find_if(iter, mProcessingEntities.end(), [pCurrMaterial](auto&& entity)
					{
						return std::get<CStaticMeshContainer*>(entity)->GetMaterialName() == dynamic_cast<const CBaseMaterial*>(pCurrMaterial)->GetName();
					})) != mProcessingEntities.end())
			{
				auto pStaticMeshContainer = std::get<CStaticMeshContainer*>(*iter);

				auto pSharedMeshResource = mpResourceManager->Load<CStaticMesh>(pStaticMeshContainer->GetMeshName())->Get<IStaticMesh>(RAT_BLOCKING);

				// \note we need to create vertex and index buffers for the object
				if (pStaticMeshContainer->GetSystemBuffersHandle() == static_cast<U32>(-1))
				{
					pStaticMeshContainer->SetSystemBuffersHandle(mMeshBuffersMap.size());

					auto pVertexDecl = mpGraphicsObjectManager->CreateVertexDeclaration().Get();

					auto&& vertexData = pSharedMeshResource->ToArrayOfStructsDataLayout();
					std::vector<U16> indices(pSharedMeshResource->GetIndices().begin(), pSharedMeshResource->GetIndices().end());

					mMeshBuffersMap.push_back(
					{ 
						mpGraphicsObjectManager->CreateVertexBuffer(BUT_STATIC, vertexData.size(), &vertexData[0]).Get(),
						mpGraphicsObjectManager->CreateIndexBuffer(BUT_STATIC, IFT_INDEX16, indices.size() * sizeof(U16), &indices[0]).Get(),
						pVertexDecl
					});

					// \note form the vertex declaration for the mesh
					pVertexDecl->AddElement({ TDEngine2::FT_FLOAT4, 0, TDEngine2::VEST_POSITION });
					pVertexDecl->AddElement({ TDEngine2::FT_FLOAT4, 0, TDEngine2::VEST_COLOR });

					if (pSharedMeshResource->HasTexCoords0())
					{
						pVertexDecl->AddElement({ TDEngine2::FT_FLOAT4, 0, TDEngine2::VEST_TEXCOORDS });
					}

					if (pSharedMeshResource->HasNormals())
					{
						pVertexDecl->AddElement({ TDEngine2::FT_FLOAT4, 0, TDEngine2::VEST_NORMAL });
					}

					if (pSharedMeshResource->HasTangents())
					{
						pVertexDecl->AddElement({ TDEngine2::FT_FLOAT4, 0, TDEngine2::VEST_TANGENT });
					}
				}

				auto meshBuffersEntry = mMeshBuffersMap[pStaticMeshContainer->GetSystemBuffersHandle()];

				// create a command for the renderer
				auto pCommand = mpOpaqueRenderGroup->SubmitDrawCommand<TDrawIndexedCommand>(pStaticMeshContainer->GetSystemBuffersHandle());

				pCommand->mpVertexBuffer      = meshBuffersEntry.mpVertexBuffer;
				pCommand->mpIndexBuffer       = meshBuffersEntry.mpIndexBuffer;
				pCommand->mpMaterialHandler   = mpResourceManager->Load<CBaseMaterial>(pStaticMeshContainer->GetMaterialName());
				pCommand->mpVertexDeclaration = meshBuffersEntry.mpVertexDecl;
				pCommand->mNumOfIndices       = pSharedMeshResource->GetIndices().size();
				pCommand->mPrimitiveType      = E_PRIMITIVE_TOPOLOGY_TYPE::PTT_TRIANGLE_LIST;
				pCommand->mObjectData.mUnused = std::get<CTransform*>(*iter)->GetTransform();

				++iter;
			}
		});

		// \note construct commands for transparent geometry
		std::for_each(firstTransparentMatIter, mCurrMaterialsArray.cend(), [](const IMaterial* pCurrMaterial)
		{
			// \todo
		});
	}

	void CStaticMeshRendererSystem::_collectUsedMaterials(const TEntitiesArray& entities, IResourceManager* pResourceManager, TMaterialsArray& usedMaterials)
	{
		usedMaterials.clear();

		IStaticMeshContainer* pCurrStaticMeshContainer = nullptr;

		IMaterial* pCurrMaterial = nullptr;

		for (auto& iter : entities)
		{
			pCurrStaticMeshContainer = std::get<CStaticMeshContainer*>(iter);

			pCurrMaterial = mpResourceManager->Load<CBaseMaterial>(pCurrStaticMeshContainer->GetMaterialName())->Get<CBaseMaterial>(RAT_BLOCKING);

			// \note skip duplicates
			if (std::find(usedMaterials.cbegin(), usedMaterials.cend(), pCurrMaterial) != usedMaterials.cend())
			{
				continue;
			}

			usedMaterials.push_back(pCurrMaterial);
		}

		// sort all materials
		std::sort(usedMaterials.begin(), usedMaterials.end(), CBaseMaterial::AlphaBasedMaterialComparator);
	}


	TDE2_API ISystem* CreateStaticMeshRendererSystem(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager, E_RESULT_CODE& result)
	{
		CStaticMeshRendererSystem* pSystemInstance = new (std::nothrow) CStaticMeshRendererSystem();

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