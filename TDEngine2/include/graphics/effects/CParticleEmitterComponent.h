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

	class CParticleEmitter : public CBaseComponent
	{
		public:
			friend TDE2_API IComponent* CreateParticleEmitter(E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CParticleEmitter)

			/*!
				\brief The method initializes an internal state of a quad sprite

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init();

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

			TDE2_API E_RESULT_CODE SetParticleEffect(const std::string& id);

			TDE2_API void SetParticleEffectHandle(TResourceId handle);

			TDE2_API void SetPlayingFlag(bool value);

			TDE2_API bool IsPlaying() const;

			TDE2_API const std::string& GetParticleEffectId() const;

			TDE2_API TResourceId GetParticleEffectHandle() const;
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


	/*!
		\brief A factory function for creation objects of CParticleEmitterFactory's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CParticleEmitterFactory's implementation
	*/

	TDE2_API IComponentFactory* CreateParticleEmitterFactory(E_RESULT_CODE& result);


	/*!
		class CParticleEmitterFactory

		\brief The class is factory facility to create a new objects of CParticleEmitter type
	*/

	class CParticleEmitterFactory : public IComponentFactory, public CBaseObject
	{
		public:
			friend TDE2_API IComponentFactory* CreateParticleEmitterFactory(E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an internal state of a factory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init();

			/*!
				\brief The method creates a new instance of a component based on passed parameters

				\param[in] pParams An object that contains parameters that are needed for the component's creation

				\return A pointer to a new instance of IComponent type
			*/

			TDE2_API IComponent* Create(const TBaseComponentParameters* pParams) const override;

			/*!
				\brief The method creates a new instance of a component based on passed parameters

				\param[in] pParams An object that contains parameters that are needed for the component's creation

				\return A pointer to a new instance of IComponent type
			*/

			TDE2_API IComponent* CreateDefault(const TBaseComponentParameters& params) const override;

			/*!
				\brief The method returns an identifier of a component's type, which
				the factory serves

				\return The method returns an identifier of a component's type, which
				the factory serves
			*/

			TDE2_API TypeId GetComponentTypeId() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CParticleEmitterFactory)
	};
}