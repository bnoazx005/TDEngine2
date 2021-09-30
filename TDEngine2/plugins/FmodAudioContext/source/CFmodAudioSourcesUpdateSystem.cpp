#include "../include/CFmodAudioSourcesUpdateSystem.h"
#include "../include/CFmodAudioContext.h"
#include "../include/CFmodAudioClip.h"
#include <scene/components/AudioComponents.h>
#include <core/IAudioContext.h>
#include <core/IResourceManager.h>
#include <ecs/CTransform.h>
#include <ecs/IWorld.h>
#include <ecs/CEntity.h>
#include <utils/CFileLogger.h>
#include <stringUtils.hpp>
#include <fmod.hpp>
#include <fmod_common.h>
#include <fmod_errors.h>


namespace TDEngine2
{
	CAudioSourcesUpdateSystem::CAudioSourcesUpdateSystem() :
		CBaseSystem()
	{
	}

	E_RESULT_CODE CAudioSourcesUpdateSystem::Init(IAudioContext* pAudioContext, IResourceManager* pResourceManager)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pAudioContext || !pResourceManager)
		{
			return RC_INVALID_ARGS;
		}

		mpAudioContext = pAudioContext;
		mpResourceManager = pResourceManager;

		mIsInitialized = true;

		return RC_OK;
	}

	void CAudioSourcesUpdateSystem::InjectBindings(IWorld* pWorld)
	{
		mAudioSources = pWorld->FindEntitiesWithComponents<CAudioSourceComponent>();
	}

	void CAudioSourcesUpdateSystem::Update(IWorld* pWorld, F32 dt)
	{
		TDE2_ASSERT(mpAudioContext);

		FMOD::System* pCoreSystem = dynamic_cast<CFMODAudioContext*>(mpAudioContext)->GetInternalContext();

		auto updateParameters = [](CAudioSourceComponent* pAudioSource, const TVector3& position, FMOD::Channel*& pChannel)
		{
			pChannel->setVolume(pAudioSource->GetVolume());
			pChannel->setPan(pAudioSource->GetPanning());
			pChannel->setMute(pAudioSource->IsMuted());

			const FMOD_VECTOR pos{ position.x, position.y, position.z };
			pChannel->set3DAttributes(&pos, nullptr);

			if (pAudioSource->IsLooped())
			{
				pChannel->setLoopCount((std::numeric_limits<I32>::max)());
			}
		};

		for (U32 i = 0; i < mAudioSources.size(); ++i)
		{
			const TEntityId currEntityId = mAudioSources[i];

			CEntity* pEntity = pWorld->FindEntity(currEntityId);
			if (!pEntity)
			{
				continue;
			}

			const TVector3& position = pEntity->GetComponent<CTransform>()->GetPosition();

			if (CAudioSourceComponent* pAudioSource = pEntity->GetComponent<CAudioSourceComponent>())
			{
				if (pAudioSource->IsPlaying())
				{
					updateParameters(pAudioSource, position, mpActiveChannels.at(currEntityId));
					continue;
				}

				FMOD::Channel* pNewChannel = nullptr;

				// \todo Reimplement this later
				const TResourceId audioResourceId = mpResourceManager->Load<IAudioSource>(pAudioSource->GetAudioClipId());
				if (TResourceId::Invalid == audioResourceId)
				{
					continue;
				}

				if (auto pClipResource = mpResourceManager->GetResource<CFMODAudioClip>(audioResourceId))
				{
					FMOD_RESULT result = pCoreSystem->playSound(pClipResource->GetInternalHandle(), nullptr, pAudioSource->IsPaused(), &pNewChannel);
					TDE2_ASSERT(FMOD_OK == result);

					mpActiveChannels.insert({ currEntityId, pNewChannel });

					updateParameters(pAudioSource, position, pNewChannel);
					pAudioSource->SetPlaying(true);
				}
			}
		}
	}


	TDE2_API ISystem* CreateAudioSourcesUpdateSystem(IAudioContext* pAudioContext, IResourceManager* pResourceManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem, CAudioSourcesUpdateSystem, result, pAudioContext, pResourceManager);
	}
}