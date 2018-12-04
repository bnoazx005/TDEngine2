#include "./../../include/ecs/CSpriteRendererSystem.h"
#include "./../../include/ecs/IWorld.h"


namespace TDEngine2
{
	CSpriteRendererSystem::CSpriteRendererSystem() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CSpriteRendererSystem::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CSpriteRendererSystem::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	void CSpriteRendererSystem::Update(IWorld* pWorld, F32 dt)
	{
		/// \todo implement this method
	}


	TDE2_API ISystem* CreateSpriteRendererSystem(E_RESULT_CODE& result)
	{
		CSpriteRendererSystem* pSystemInstance = new (std::nothrow) CSpriteRendererSystem();

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