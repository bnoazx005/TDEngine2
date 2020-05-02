#include "./../../include/ecs/CTransformSystem.h"
#include "./../../include/ecs/IWorld.h"
#include "./../../include/ecs/CTransform.h"
#include "./../../include/ecs/CEntity.h"
#include "./../../include/ecs/components/CBoundsComponent.h"


namespace TDEngine2
{
	CTransformSystem::CTransformSystem():
		CBaseSystem()
	{
	}

	E_RESULT_CODE CTransformSystem::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;
		
		return RC_OK;
	}

	E_RESULT_CODE CTransformSystem::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	void CTransformSystem::InjectBindings(IWorld* pWorld)
	{
		mTransforms.clear();

		std::vector<TEntityId> entities = pWorld->FindEntitiesWithComponents<CTransform>();

		CEntity* pCurrEntity = nullptr;

		for (auto iter = entities.begin(); iter != entities.end(); ++iter)
		{
			pCurrEntity = pWorld->FindEntity(*iter);

			if (!pCurrEntity)
			{
				continue;
			}

			mTransforms.push_back(pCurrEntity->GetComponent<CTransform>());
		}
	}

	void CTransformSystem::Update(IWorld* pWorld, F32 dt)
	{
		pWorld->ForEach<CTransform>([pWorld](TEntityId entityId, IComponent* pComponent)
		{
			auto pTransform = dynamic_cast<CTransform*>(pComponent);

			if (!pTransform->HasChanged())
			{
				return;
			}
			
			const TMatrix4& localToWorldMatrix = (TranslationMatrix(pTransform->GetPosition()) * 
												  RotationMatrix(pTransform->GetRotation())) * 
												  ScaleMatrix(pTransform->GetScale());

			pTransform->SetTransform(localToWorldMatrix);

			if (CEntity* pEntity = pWorld->FindEntity(entityId))
			{
				if (auto pBounds = pEntity->GetComponent<CBoundsComponent>())
				{
					pBounds->SetDirty(true);
				}
			}

			/// \todo Implement parent-to-child relationship's update
		});
	}


	TDE2_API ISystem* CreateTransformSystem(E_RESULT_CODE& result)
	{
		CTransformSystem* pSystemInstance = new (std::nothrow) CTransformSystem();

		if (!pSystemInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pSystemInstance->Init();

		if (result != RC_OK)
		{
			delete pSystemInstance;

			pSystemInstance = nullptr;
		}

		return dynamic_cast<ISystem*>(pSystemInstance);
	}
}