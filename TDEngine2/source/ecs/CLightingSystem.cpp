#include "./../../include/ecs/CLightingSystem.h"
#include "./../../include/ecs/IWorld.h"
#include "./../../include/ecs/CEntity.h"
#include "./../../include/graphics/IRenderer.h"
#include "./../../include/graphics/InternalShaderData.h"
#include "./../../include/scene/components/CDirectionalLight.h"
#include "./../../include/utils/CFileLogger.h"
#include "./../../include/utils/CResult.h"


namespace TDEngine2
{
	CLightingSystem::CLightingSystem() :
		CBaseSystem()
	{
	}

	E_RESULT_CODE CLightingSystem::Init(IRenderer* pRenderer)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pRenderer)
		{
			return RC_INVALID_ARGS;
		}

		mpRenderer = pRenderer;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CLightingSystem::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	void CLightingSystem::InjectBindings(IWorld* pWorld)
	{
		mDirectionalLightsEntities = pWorld->FindEntitiesWithComponents<CDirectionalLight>();
	}

	void CLightingSystem::Update(IWorld* pWorld, F32 dt)
	{
		TDE2_ASSERT(mDirectionalLightsEntities.size() == 1); // \note For now only single sun light source is supported
		TDE2_ASSERT(mpRenderer);

		TLightingShaderData lightingData;

		for (TEntityId currEntity : mDirectionalLightsEntities)
		{
			if (auto pEntity = pWorld->FindEntity(currEntity))
			{
				if (auto pSunLight = pEntity->GetComponent<CDirectionalLight>())
				{
					lightingData.mSunLightDirection = TVector4(Normalize(pSunLight->GetDirection()), 0.0f);
					lightingData.mSunLightColor     = pSunLight->GetColor();
				}
			}
		}

		if (mpRenderer)
		{
			PANIC_ON_FAILURE(mpRenderer->SetLightingData(lightingData));
		}
	}


	TDE2_API ISystem* CreateLightingSystem(IRenderer* pRenderer, E_RESULT_CODE& result)
	{
		CLightingSystem* pSystemInstance = new (std::nothrow) CLightingSystem();

		if (!pSystemInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pSystemInstance->Init(pRenderer);

		if (result != RC_OK)
		{
			delete pSystemInstance;

			pSystemInstance = nullptr;
		}

		return dynamic_cast<ISystem*>(pSystemInstance);
	}
}