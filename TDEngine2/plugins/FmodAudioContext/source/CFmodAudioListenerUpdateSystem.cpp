#include "../include/CFmodAudioListenerUpdateSystem.h"
#include <scene/components/AudioComponents.h>
#include <core/IAudioContext.h>
#include <ecs/CTransform.h>
#include <ecs/IWorld.h>
#include <ecs/CEntity.h>
#include <utils/CFileLogger.h>
#include <stringUtils.hpp>


namespace TDEngine2
{
	CAudioListenerUpdateSystem::CAudioListenerUpdateSystem() :
		CBaseSystem()
	{
	}

	E_RESULT_CODE CAudioListenerUpdateSystem::Init(IAudioContext* pAudioContext)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pAudioContext)
		{
			return RC_INVALID_ARGS;
		}

		mpAudioContext = pAudioContext;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CAudioListenerUpdateSystem::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	void CAudioListenerUpdateSystem::InjectBindings(IWorld* pWorld)
	{
		auto&& entities = pWorld->FindEntitiesWithComponents<CTransform, CAudioListenerComponent>();
		if (entities.empty())
		{
			TDE2_ASSERT(false);
			return;
		}

		mMainListenerEntityId = entities.front();
	}

	void CAudioListenerUpdateSystem::Update(IWorld* pWorld, F32 dt)
	{
		TDE2_ASSERT(mpAudioContext && TEntityId::Invalid != mMainListenerEntityId);

		CEntity* pListenerEntity = pWorld->FindEntity(mMainListenerEntityId);
		if (!pListenerEntity)
		{
			LOG_ERROR(Wrench::StringUtils::Format("[AudioListenerUpdateSystem] Missing entity, entity_id: {0}", static_cast<U32>(mMainListenerEntityId)));
			return;
		}

		if (auto pTransform = pListenerEntity->GetComponent<CTransform>())
		{
			mpAudioContext->SetListenerPosition(pTransform->GetPosition());			
		}
	}


	TDE2_API ISystem* CreateAudioListenerUpdateSystem(IAudioContext* pAudioContext, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem, CAudioListenerUpdateSystem, result, pAudioContext);
	}
}