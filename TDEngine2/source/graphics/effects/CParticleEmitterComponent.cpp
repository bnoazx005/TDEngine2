#include "../../../include/graphics/effects/CParticleEmitterComponent.h"
#define META_EXPORT_ECS_SECTION
#include "../../include/metadata.h"


namespace TDEngine2
{
	TDE2_REGISTER_COMPONENT_FACTORY(CreateParticleEmitterFactory)
	TDE2_DEFINE_COMPONENT_META(TParticleEmitterComponentData)


	CParticleEmitter::CParticleEmitter() :
		CBaseComponentT() 
	{
	}

	E_RESULT_CODE CParticleEmitter::SetParticleEffect(const std::string& id)
	{
		if (id.empty())
		{
			return RC_INVALID_ARGS;
		}

		mData.mParticleEffectName = id;

		return RC_OK;
	}

	void CParticleEmitter::SetParticleEffectHandle(TResourceId handle)
	{
		mData.mParticleEffectId = handle;
	}

	void CParticleEmitter::SetPlayingFlag(bool value)
	{
		mData.mIsPlaying = value;
	}

	bool CParticleEmitter::IsPlaying() const
	{
		return mData.mIsPlaying;
	}

	const std::string& CParticleEmitter::GetParticleEffectId() const
	{
		return mData.mParticleEffectName;
	}

	TResourceId CParticleEmitter::GetParticleEffectHandle() const
	{
		return mData.mParticleEffectId;
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