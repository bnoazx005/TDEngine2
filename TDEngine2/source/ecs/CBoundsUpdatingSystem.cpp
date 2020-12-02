#include "../../include/ecs/CBoundsUpdatingSystem.h"
#include "../../include/ecs/IWorld.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/ecs/CTransform.h"
#include "../../include/ecs/components/CBoundsComponent.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/graphics/CStaticMesh.h"
#include "../../include/graphics/CStaticMeshContainer.h"
#include "../../include/graphics/CQuadSprite.h"
#include "../../include/graphics/IDebugUtility.h"
#include "../../include/utils/CFileLogger.h"
#include "../../include/math/TVector4.h"


namespace TDEngine2
{
	CBoundsUpdatingSystem::CBoundsUpdatingSystem() :
		CBaseSystem()
	{
	}

	E_RESULT_CODE CBoundsUpdatingSystem::Init(IResourceManager* pResourceManager, IDebugUtility* pDebugUtility)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pResourceManager)
		{
			return RC_INVALID_ARGS;
		}

		mpResourceManager = pResourceManager;
		mpDebugUtility    = pDebugUtility;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CBoundsUpdatingSystem::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	void CBoundsUpdatingSystem::InjectBindings(IWorld* pWorld)
	{
		mStaticMeshesEntities = pWorld->FindEntitiesWithComponents<CStaticMeshContainer>();
		mSpritesEntities = pWorld->FindEntitiesWithComponents<CQuadSprite>();
	}

	void CBoundsUpdatingSystem::Update(IWorld* pWorld, F32 dt)
	{
		_processEntities(pWorld, mStaticMeshesEntities, std::bind(&CBoundsUpdatingSystem::_computeStaticMeshBounds, this, std::placeholders::_1));
		_processEntities(pWorld, mSpritesEntities, std::bind(&CBoundsUpdatingSystem::_computeSpritesBounds, this, std::placeholders::_1));
	}

	void CBoundsUpdatingSystem::_processEntities(IWorld* pWorld, const std::vector<TEntityId>& entities, const std::function<void(CEntity*)>& processCallback)
	{
		CEntity* pEntity = nullptr;

		for (TEntityId currEntity : entities)
		{
			if (!(pEntity = pWorld->FindEntity(currEntity)))
			{
				continue;
			}

			if (!pEntity->HasComponent<CBoundsComponent>())
			{
				pEntity->AddComponent<CBoundsComponent>();
				return;
			}

			if (CBoundsComponent* pBounds = pEntity->GetComponent<CBoundsComponent>())
			{
				if (mpDebugUtility)
				{
					mpDebugUtility->DrawAABB(pBounds->GetBounds(), { 1.0f, 1.0f, 1.0f, 1.0f });
				}

				if (!pBounds->IsDirty())
				{
					continue;
				}

				if (processCallback) // \note Compute bounds for the entity
				{
					processCallback(pEntity);
				}

				pBounds->SetDirty(false);
			}
		}
	}

	void CBoundsUpdatingSystem::_computeStaticMeshBounds(CEntity* pEntity)
	{
		CBoundsComponent* pBounds = pEntity->GetComponent<CBoundsComponent>();

		if (CStaticMeshContainer* pStaticMeshContainer = pEntity->GetComponent<CStaticMeshContainer>())
		{
			if (IStaticMesh* pStaticMesh = mpResourceManager->GetResource<IStaticMesh>(mpResourceManager->Load<CStaticMesh>(pStaticMeshContainer->GetMeshName())))
			{
				auto&& vertices = pStaticMesh->GetPositionsArray();

				if (CTransform* pTransform = pEntity->GetComponent<CTransform>())
				{
					const TMatrix4& worldMatrix = pTransform->GetTransform();

					TVector4 min{ (std::numeric_limits<F32>::max)() };
					TVector4 max{ -(std::numeric_limits<F32>::max)() };

					for (auto&& v : vertices)
					{
						TVector4 transformedVertex = worldMatrix * v;

						min = Min(min, transformedVertex);
						max = Max(max, transformedVertex);
					}

					pBounds->SetBounds(TAABB{ min, max });
				}
			}
		}
	}

	void CBoundsUpdatingSystem::_computeSpritesBounds(CEntity* pEntity)
	{
		CBoundsComponent* pBounds = pEntity->GetComponent<CBoundsComponent>();
		
		static const std::array<TVector4, 4> spriteVerts
		{
			TVector4 { -0.5f, 0.5f, 0.0f, 1.0f },
			TVector4 { 0.5f, 0.5f, 0.0f, 1.0f },
			TVector4 { -0.5f, -0.5f, 0.0f, 1.0f },
			TVector4 { 0.5f, -0.5f, 0.0f, 1.0f },
		};

		if (CTransform* pSpriteTransform = pEntity->GetComponent<CTransform>())
		{
			const TMatrix4& worldMatrix = pSpriteTransform->GetTransform();

			TVector4 min { (std::numeric_limits<F32>::max)() };
			TVector4 max { -(std::numeric_limits<F32>::max)() };

			for (auto&& v : spriteVerts)
			{
				TVector4 transformedVertex = worldMatrix * v;

				min = Min(min, transformedVertex);
				max = Max(max, transformedVertex);
			}

			pBounds->SetBounds(TAABB{ min, max });
		}
	}
	

	TDE2_API ISystem* CreateBoundsUpdatingSystem(IResourceManager* pResourceManager, IDebugUtility* pDebugUtility, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem, CBoundsUpdatingSystem, result, pResourceManager, pDebugUtility);
	}
}