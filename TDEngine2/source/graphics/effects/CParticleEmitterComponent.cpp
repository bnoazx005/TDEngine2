#include "../../../include/graphics/effects/CParticleEmitterComponent.h"


namespace TDEngine2
{
	struct TParticleEmitterArchiveKeys
	{
		static const std::string mParticleEffectKeyId;
	};

	const std::string TParticleEmitterArchiveKeys::mParticleEffectKeyId = "particle-effect-id";


	CParticleEmitter::CParticleEmitter() :
		CBaseComponent()
	{
	}

	E_RESULT_CODE CParticleEmitter::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsPlaying = false;

		mParticleEffectId = TResourceId::Invalid;

		mIsInitialized = true;

		return RC_OK;
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


	CParticleEmitterFactory::CParticleEmitterFactory() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CParticleEmitterFactory::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CParticleEmitterFactory::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	IComponent* CParticleEmitterFactory::Create(const TBaseComponentParameters* pParams) const
	{
		if (!pParams)
		{
			return nullptr;
		}

		const TParticleEmitterParameters* params = static_cast<const TParticleEmitterParameters*>(pParams);

		E_RESULT_CODE result = RC_OK;

		return CreateParticleEmitter(result);
	}

	IComponent* CParticleEmitterFactory::CreateDefault(const TBaseComponentParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		return CreateParticleEmitter(result);
	}

	TypeId CParticleEmitterFactory::GetComponentTypeId() const
	{
		return CParticleEmitter::GetTypeId();
	}


	IComponentFactory* CreateParticleEmitterFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CParticleEmitterFactory, result);
	}
}