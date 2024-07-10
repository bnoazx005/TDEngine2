#include "../../../include/graphics/effects/CParticleEmitterComponent.h"


namespace TDEngine2
{
	TDE2_REGISTER_COMPONENT_FACTORY(CreateParticleEmitterFactory)


	struct TParticleEmitterArchiveKeys
	{
		static const std::string mParticleEffectKeyId;
	};

	const std::string TParticleEmitterArchiveKeys::mParticleEffectKeyId = "particle-effect-id";


	CParticleEmitter::CParticleEmitter() :
		CBaseComponent(), mIsPlaying(false), mParticleEffectId(TResourceId::Invalid)
	{
	}

	E_RESULT_CODE CParticleEmitter::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}

		mParticleEffectName = pReader->GetString(TParticleEmitterArchiveKeys::mParticleEffectKeyId);
		TDE2_ASSERT(!mParticleEffectName.empty());

		return RC_OK;
	}

	E_RESULT_CODE CParticleEmitter::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		pWriter->BeginGroup("component");
		{
			pWriter->SetUInt32("type_id", static_cast<U32>(CParticleEmitter::GetTypeId()));

			pWriter->SetString(TParticleEmitterArchiveKeys::mParticleEffectKeyId, mParticleEffectName);
		}
		pWriter->EndGroup();

		return RC_OK;
	}

	E_RESULT_CODE CParticleEmitter::Clone(IComponent*& pDestObject) const
	{
		if (auto pComponent = dynamic_cast<CParticleEmitter*>(pDestObject))
		{
			pComponent->mIsPlaying = mIsPlaying;
			pComponent->mParticleEffectId = mParticleEffectId;
			pComponent->mParticleEffectName = mParticleEffectName;

#if TDE2_EDITORS_ENABLED
			pComponent->mResetStateOnNextFrame = mResetStateOnNextFrame;
#endif

			return RC_OK;
		}

		return RC_FAIL;
	}

	E_RESULT_CODE CParticleEmitter::SetParticleEffect(const std::string& id)
	{
		if (id.empty())
		{
			return RC_INVALID_ARGS;
		}

		mParticleEffectName = id;

		return RC_OK;
	}

	void CParticleEmitter::SetParticleEffectHandle(TResourceId handle)
	{
		mParticleEffectId = handle;
	}

	void CParticleEmitter::SetPlayingFlag(bool value)
	{
		mIsPlaying = value;
	}

	bool CParticleEmitter::IsPlaying() const
	{
		return mIsPlaying;
	}

	const std::string& CParticleEmitter::GetParticleEffectId() const
	{
		return mParticleEffectName;
	}

	TResourceId CParticleEmitter::GetParticleEffectHandle() const
	{
		return mParticleEffectId;
	}
	

	IComponent* CreateParticleEmitter(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CParticleEmitter, result);
	}


	/*!
		\brief CParticleEmitterFactory's definition
	*/

	CParticleEmitterFactory::CParticleEmitterFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* CParticleEmitterFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return CreateParticleEmitter(result);
	}

	E_RESULT_CODE CParticleEmitterFactory::SetupComponent(CParticleEmitter* pComponent, const TParticleEmitterParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}

		return RC_OK;
	}


	IComponentFactory* CreateParticleEmitterFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CParticleEmitterFactory, result);
	}
}