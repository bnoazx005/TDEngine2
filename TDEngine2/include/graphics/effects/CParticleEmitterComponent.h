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

	class CParticleEmitter : public CBaseComponent, public CPoolMemoryAllocPolicy<CParticleEmitter, 1 << 20>
	{
		public:
			friend TDE2_API IComponent* CreateParticleEmitter(E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CParticleEmitter)

			/*!
				\brief The method deserializes object's state from given reader

				\param[in, out] pReader An input stream of data that contains information about the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Load(IArchiveReader* pReader) override;

			/*!
				\brief The method serializes object's state into given stream

				\param[in, out] pWriter An output stream of data that writes information about the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Save(IArchiveWriter* pWriter) override;

			/*!
				\brief The method creates a new deep copy of the instance and returns a smart pointer to it.
				The original state of the object stays the same

				\param[in] pDestObject A valid pointer to an object which the properties will be assigned into
			*/

			TDE2_API E_RESULT_CODE Clone(IComponent*& pDestObject) const override;

			TDE2_API E_RESULT_CODE SetParticleEffect(const std::string& id);

			TDE2_API void SetParticleEffectHandle(TResourceId handle);

			TDE2_API void SetPlayingFlag(bool value);

			TDE2_API bool IsPlaying() const;

			TDE2_API const std::string& GetParticleEffectId() const;

			TDE2_API TResourceId GetParticleEffectHandle() const;

			bool mResetStateOnNextFrame = true;

		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CParticleEmitter)
		protected:
			std::string mParticleEffectName;

			TResourceId mParticleEffectId;

			bool mIsPlaying;
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