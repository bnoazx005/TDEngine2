/*!
	\file CParticleEmitter.h
	\date 20.03.2021
	\authors Kasimov Ildar
*/

#pragma once


#include "../../ecs/CBaseComponent.h"


namespace TDEngine2
{
	enum class TResourceId : U32;


	CLASS_META(SECTION = ecs, flags = SERIALIZE_MARKED_ONLY_FIELDS)
	struct TParticleEmitterComponentData
	{
		FIELD_META(name = particle_effect_id) std::string mParticleEffectName = "";

		TResourceId                                       mParticleEffectId = TResourceId::Invalid;
		bool                                              mIsPlaying = false;

//#if TDE2_EDITORS_ENABLED
		bool                                              mResetStateOnNextFrame = true;
//#endif

		TDE2_DECLARE_COMPONENT_META(TParticleEmitterComponentData);
	};


	/*!
		\brief A factory function for creation objects of CParticleEmitter's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CParticleEmitter's implementation
	*/

	TDE2_API IComponent* CreateParticleEmitter(E_RESULT_CODE& result);


	/*!
		class CParticleEmitter

		\brief The interface describes a functionality of a particle system component
	*/

	class CParticleEmitter : public CBaseComponentT<CParticleEmitter, TParticleEmitterComponentData>
	{
		public:
			friend TDE2_API IComponent* CreateParticleEmitter(E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CParticleEmitter)

			TDE2_API E_RESULT_CODE SetParticleEffect(const std::string& id);

			TDE2_API void SetParticleEffectHandle(TResourceId handle);

			TDE2_API void SetPlayingFlag(bool value);

			TDE2_API bool IsPlaying() const;

			TDE2_API const std::string& GetParticleEffectId() const;

			TDE2_API TResourceId GetParticleEffectHandle() const;

		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CParticleEmitter)
	};


	/*!
		struct TParticleEmitterParameters

		\brief The structure contains parameters for creation of CParticleEmitter
	*/

	typedef struct TParticleEmitterParameters : public TBaseComponentParameters
	{
	} TParticleEmitterParameters;


	TDE2_DECLARE_COMPONENT_FACTORY(ParticleEmitter, TParticleEmitterParameters);
}