#include "./../../include/ecs/CTransformSystem.h"
#include "./../../include/ecs/IWorld.h"
#include "./../../include/ecs/CTransform.h"


namespace TDEngine2
{
	CTransformSystem::CTransformSystem():
		CBaseObject()
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

	void CTransformSystem::Update(IWorld* pWorld, F32 dt)
	{
		pWorld->ForEach<CTransform>([](TEntityId entityId, IComponent* pComponent)
		{
			auto pTransform = dynamic_cast<CTransform*>(pComponent);
			
			/// \todo implement logic of CTranform update
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