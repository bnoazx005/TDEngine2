/*!
	\file IParticleEffect.h
	\date 09.03.2021
	\authors Kasimov Ildar
*/

#pragma once


#include "../../utils/Types.h"
#include "../../utils/Utils.h"
#include "../../math/MathUtils.h"
#include "../../core/IResource.h"
#include "../../core/IResourceFactory.h"
#include "../../core/IResourceLoader.h"
#include "../../core/Serialization.h"
#include "../../graphics/animation/CAnimationCurve.h"
#include "../../utils/CGradientColor.h"
#include "ParticleEmitters.h"


namespace TDEngine2
{
	enum class E_PARTICLE_SIMULATION_SPACE : U16
	{
		WORLD,
		LOCAL,
	};


	/*!
		\brief The enumeration contains values for compact storage of information about enabled parameter modifiers
	*/

	enum class E_PARTICLE_EFFECT_INFO_FLAGS : U32
	{
		DEFAULT = 0,
		E_SIZE_OVER_LIFETIME_ENABLED = 1,
		E_COLOR_OVER_LIFETIME_ENABLED = 1 << 1,
		E_VELOCITY_OVER_LIFETIME_ENABLED = 1 << 2,
		E_ROTATION_OVER_LIFETIME_ENABLED = 1 << 3,
		E_GRAVITY_FORCE_ENABLED = 1 << 4,
		E_FORCE_OVER_LIFETIME_ENABLED = 1 << 5,
	};

	TDE2_DECLARE_BITMASK_OPERATORS_INTERNAL(E_PARTICLE_EFFECT_INFO_FLAGS);


	enum class E_PARTICLE_COLOR_PARAMETER_TYPE : U16
	{
		SINGLE_COLOR = 0,
		TWEEN_RANDOM,
		GRADIENT_LERP,
		GRADIENT_RANDOM,
	};


	enum class E_PARTICLE_VELOCITY_PARAMETER_TYPE : U16
	{
		CONSTANTS,
		CURVES
	};


	/*!
		\brief The structure contains information about how the color should be applied to particles
	*/

	typedef struct TParticleColorParameter
	{
		E_PARTICLE_COLOR_PARAMETER_TYPE mType = E_PARTICLE_COLOR_PARAMETER_TYPE::SINGLE_COLOR;

		TColor32F mFirstColor;
		TColor32F mSecondColor;

		CScopedPtr<CGradientColor> mGradientColor;
	} TParticleColorParameter, *TParticleColorParameterPtr;


	typedef struct TParticleVelocityParameter
	{
		E_PARTICLE_VELOCITY_PARAMETER_TYPE mType = E_PARTICLE_VELOCITY_PARAMETER_TYPE::CONSTANTS;

		CScopedPtr<CAnimationCurve> mXCurve;
		CScopedPtr<CAnimationCurve> mYCurve;
		CScopedPtr<CAnimationCurve> mZCurve;

		CScopedPtr<CAnimationCurve> mSpeedFactorCurve;

		TVector3 mVelocityConst;
		F32 mSpeedFactorConst;
	} TParticleVelocityParameter, *TParticleVelocityParameterPtr;


	/*!
		struct TParticleEffect2DParameters

		\brief The stucture contains fields for creation IParticleEffect and its derived classes
	*/

	typedef struct TParticleEffectParameters : TBaseResourceParameters
	{
		/// \note Common settings
		F32  mDuration;
		bool mIsLooped;
		U16  mMaxParticlesCount;
		std::string mMaterialName;

		/// \note Particle's settings
		TRangeF32 mLifeTime;
		TRangeF32 mInitialSize;
		TRangeF32 mInitialRotation;

		E_PARTICLE_EFFECT_INFO_FLAGS mFlags;
	} TParticleEffectParameters, *TParticleEffectParametersPtr;


	/*!
		interface IParticleEffect

		\brief The interface describes a functionality of a particle effect's settings
	*/

	class IParticleEffect: public ISerializable
	{
		public:
			/*!
				\brief The method initializes an internal state of a material

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation
				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation
				\param[in] name A resource's name

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name) = 0;

			/*!
				\brief The method sets up a duration of the effect

				\param[in] duration A positive value which determines a length of the effect's playback

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE SetDuration(F32 duration) = 0;

			/*!
				\brief The method enables/disables loop mode of the playback

				\param[in] value If the flag equals to true the loop mode is enabled
			*/

			TDE2_API virtual void SetLoopMode(bool value) = 0;

			/*!
				\brief The method sets a upper bound for amount of emitted particles

				\param[in] A maximal limit of particles
			*/

			TDE2_API virtual void SetMaxParticlesCount(U16 particlesCount) = 0;

			/*!
				\brief The method sets up a name of used material. Note that it isn't verify is material already loaded or exist
			
				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE SetMaterialName(const std::string& value) = 0;

			TDE2_API virtual E_RESULT_CODE SetLifetime(const TRangeF32& value) = 0;

			TDE2_API virtual E_RESULT_CODE SetInitialSize(const TRangeF32& value) = 0;

			TDE2_API virtual void SetInitialRotation(const TRangeF32& value) = 0;

			/*!
				\brief The method defines initial color of the particles
			*/

			TDE2_API virtual void SetInitialColor(const TParticleColorParameter& colorData) = 0;

			TDE2_API virtual void SetInitialVelocityData(const TVector3& direction, F32 speedFactor) = 0;

			TDE2_API virtual void SetGravityModifier(F32 value) = 0;

			/*!
				\brief The method defines how much particles are spawned per frame
			*/

			TDE2_API virtual void SetEmissionRate(U32 value) = 0;

			TDE2_API virtual void SetSimulationSpaceType(E_PARTICLE_SIMULATION_SPACE spaceType) = 0;

			/*!
				\brief The method initializes internal state of a shared emitter use given pointer to the object.
				
				\param[in] pEmitter A pointer to shared emitter (for now it could be box, sphere or cone shapes)

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE SetSharedEmitter(const CScopedPtr<CBaseParticlesEmitter>& pEmitter) = 0;

			/*!
				\brief The method defines a curve that controls particles size over their lifetime

				\param[in] pCurve A pointer to a curve

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE SetSizeCurve(const CScopedPtr<CAnimationCurve>& pCurve) = 0;

			/*!
				\brief The method defines how particles' color behaves itself during lifetime of the ones

				\param[in] colorData Could be simple color, tween between colors or gradient sampler

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE SetColorOverLifeTime(const TParticleColorParameter& colorData) = 0;

			/*!
				\brief The method defines how particles' velocity are being changing over time

				\param[in] velocityData An object that contains a bunch of curves that determines behviour of a velocity

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE SetVelocityOverTime(const TParticleVelocityParameter& velocityData) = 0;

			TDE2_API virtual E_RESULT_CODE SetRotationOverTime(F32 anglePerFrame) = 0;

			TDE2_API virtual void SetForceOverTime(const TVector3& force) = 0;

			/*!
				\brief The method updates information about enabled/disables modifiers of particles parameters.
				Note that a new value overwrites previous one.
				
				\param[in] value A disjunction of flags that determines which modifiers are enabled

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE SetModifiersFlags(E_PARTICLE_EFFECT_INFO_FLAGS value) = 0;

			/*!
				\return The method returns a duration of the effect
			*/

			TDE2_API virtual F32 GetDuration() const = 0;

			/*!
				\return The flag is true when loop mode is enabled on the effect
			*/

			TDE2_API virtual bool IsLoopModeActive() const = 0;

			/*!
				\return The method returns a maximal number of particles
			*/

			TDE2_API virtual U16 GetMaxParticlesCount() const = 0;

			/*!
				\return The method returns an identifier of a material
			*/

			TDE2_API virtual const std::string& GetMaterialName() const = 0;

			TDE2_API virtual const TRangeF32& GetLifetime() const = 0;

			TDE2_API virtual const TRangeF32& GetInitialSize() const = 0;

			TDE2_API virtual const TRangeF32& GetInitialRotation() const = 0;

			TDE2_API virtual const TParticleColorParameter& GetInitialColor() const = 0;

			TDE2_API virtual const TVector3& GetInitialMoveDirection() const = 0;

			TDE2_API virtual F32 GetInitialSpeedFactor() const = 0;

			TDE2_API virtual F32 GetGravityModifier() const = 0;

			TDE2_API virtual U32 GetEmissionRate() const = 0;

			TDE2_API virtual E_PARTICLE_SIMULATION_SPACE GetSimulationSpaceType() const = 0;

			TDE2_API virtual CScopedPtr<CBaseParticlesEmitter> GetSharedEmitter() const = 0;

			TDE2_API virtual CScopedPtr<CAnimationCurve> GetSizeCurve() const = 0;

			TDE2_API virtual const TParticleColorParameter& GetColorOverLifeTime() const = 0;

			TDE2_API virtual const TParticleVelocityParameter& GetVelocityOverTime() const = 0;

			TDE2_API virtual const TVector3& GetForceOverTime() const = 0;

			TDE2_API virtual F32 GetRotationOverTime() const = 0;

			TDE2_API virtual E_PARTICLE_EFFECT_INFO_FLAGS GetEnabledModifiersFlags() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IParticleEffect)
	};


	/*!
		interface IParticleEffectLoader

		\brief The interface describes a functionality of a mesh loader
	*/

	class IParticleEffectLoader : public IResourceLoader
	{
		public:
			/*!
				\brief The method initializes an inner state of an object

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation
				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation
				\param[in, out] pFileSystem A pointer to IFileSystem's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IParticleEffectLoader)
	};


	/*!
		interface IStaticParticleEffectFactory

		\brief The interface represents a functionality of a factory of IParticleEffect objects
	*/

	class IParticleEffectFactory : public IResourceFactory
	{
		public:
			/*!
				\brief The method initializes an internal state of a shader factory

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation
				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IParticleEffectFactory)
	};
}