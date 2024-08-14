/*!
	/file ParticleEmitters.h
	/date 22.03.2021
	/authors Kasimov Ildar
*/

#pragma once


#include "../../utils/Types.h"
#include "../../utils/Utils.h"
#include "../../core/CBaseObject.h"
#include "../../core/Serialization.h"
#include "../../math/TVector3.h"
#include "../../math/TVector4.h"
#include "../../utils/Color.h"
#include <vector>
#include <string>
#include <tuple>


namespace TDEngine2
{
	class IParticleEffect;
	class CTransform;
	struct TParticle;
	struct TParticleColorParameter;
	struct TParticleVelocityParameter;


	struct TEmitterUniformsData
	{
		// box emitter's data
		TVector4 mBoxSizes;

		// sphere/cone emitter's data
		F32      mSphereConeRadius;
		F32      mConeHeight;
		F32      mRandValue;

		// common 
		F32       mDuration;
		TVector4  mPosition;
		TVector4  mShapeOrigin;
		TVector4  mVelocity;
		TVector4  mInitialLifetime;
		TVector4  mInitialSize;
		TVector4  mInitialRotation;
		TColor32F mInitialColor;

		U32      mIs2DEmitter;
		U32      mMaxParticles;
		U32      mEmitRate;
		U32      mEmitterType; // contains TypeId of used emitter
		U32      mEmitterIndex;
		U32      mFlags;

		F32      mGravityModifier;
		F32      mRotationPerFrame;
		TVector3 mForcePerFrame;
	};


	/*!
		\brief The class is a common implementation for all particles emitters that are represented in the engine.
		The class is designed to be stateless and it's more like processor of particles than actual OOP entity
	*/

	class CBaseParticlesEmitter: public CBaseObject, public ISerializable
	{
		public:
			/*!
				\brief The method initializes an internal state of an object

				\param[in, out] pOwnerEffect A pointer to IParticleEffect implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IParticleEffect* pOwnerEffect);

			/*!
				\brief The method deserializes object's state from given reader

				\param[in, out] pReader An input stream of data that contains information about the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API static TResult<CBaseParticlesEmitter*> Deserialize(IArchiveReader* pReader);

			/*!
				\brief The method initializes a state of a given particle as it's used for the first time

				\param[in] pTransform A pointer to transform component that defines a position of current emitter
				\param[out] particleInfo A reference to a particle's object that should be re-initialized 

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE EmitParticle(const CTransform* pTransform, TParticle& particleInfo) = 0;

			TDE2_API E_RESULT_CODE SetOwnerEffect(IParticleEffect* pOwner);

			/*!
				\brief The method determines whether or not the emitter produces particles in 2D or 3D fashion

				\param[in] value True enables 2D emission mode for the emitter, false is default 3D mode
			*/

			TDE2_API void Set2DMode(bool value);

			TDE2_API bool Is2DModeEnabled() const;

			TDE2_API virtual TypeId GetEmitterTypeId() const { return TypeId::Invalid; }

			TDE2_API virtual TEmitterUniformsData GetShaderUniformsData() const;

			TDE2_API static TColor32F GetColorData(const TParticleColorParameter& colorData, F32 time);
			TDE2_API static TVector3 GetVelocityData(const TParticleVelocityParameter& velocityData, F32 time);

			TDE2_API static const std::vector<std::tuple<std::string, TypeId>>& GetEmittersTypes();
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseParticlesEmitter)

			TDE2_API TColor32F _getInitColor() const;
			TDE2_API TVector3 _getInitVelocity() const;
		protected:
			IParticleEffect* mpOwnerEffect;

			bool mIs2DEmitter = false;
	};


	TDE2_API CScopedPtr<CBaseParticlesEmitter> CreateEmitterByTypeId(IParticleEffect* pOwnerEffect, TypeId id);


	/*!
		\brief A factory function for creation objects of CBoxParticlesEmitter's type

		\param[in, out] pOwnerEffect A pointer to IParticleEffect implementation
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CBaseParticlesEmitter's implementation
	*/

	TDE2_API CBaseParticlesEmitter* CreateBoxParticlesEmitter(IParticleEffect* pOwnerEffect, E_RESULT_CODE& result);


	/*!
		class CBoxParticlesEmitter

		\brief The class is an implementation of 3D box emitting particles
	*/

	class CBoxParticlesEmitter : public CBaseParticlesEmitter
	{
		public:
			friend TDE2_API CBaseParticlesEmitter* CreateBoxParticlesEmitter(IParticleEffect*, E_RESULT_CODE&);
		public:
			TDE2_REGISTER_VIRTUAL_TYPE_EX(CBoxParticlesEmitter, GetEmitterTypeId)
			TDE2_REGISTER_TYPE(CBoxParticlesEmitter)

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
				\brief The method initializes a state of a given particle as it's used for the first time

				\param[in] pTransform A pointer to transform component that defines a position of current emitter
				\param[out] particleInfo A reference to a particle's object that should be re-initialized

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE EmitParticle(const CTransform* pTransform, TParticle& particleInfo) override;

			TDE2_API E_RESULT_CODE SetSizes(const TVector3& sizes);
			TDE2_API E_RESULT_CODE SetOrigin(const TVector3& origin);

			TDE2_API const TVector3& GetSizes() const;
			TDE2_API const TVector3& GetOrigin() const;

			TDE2_API TEmitterUniformsData GetShaderUniformsData() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBoxParticlesEmitter)
		protected:
			TVector3 mBoxSizes = ZeroVector3;
			TVector3 mBoxOrigin = ZeroVector3;
	};


	/*!
		\brief A factory function for creation objects of CSphereParticlesEmitter's type

		\param[in, out] pOwnerEffect A pointer to IParticleEffect implementation
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CBaseParticlesEmitter's implementation
	*/

	TDE2_API CBaseParticlesEmitter* CreateSphereParticlesEmitter(IParticleEffect* pOwnerEffect, E_RESULT_CODE& result);


	/*!
		class CSphereParticlesEmitter

		\brief The class is an implementation of 3D sphere emitting particles
	*/

	class CSphereParticlesEmitter : public CBaseParticlesEmitter
	{
		public:
			friend TDE2_API CBaseParticlesEmitter* CreateSphereParticlesEmitter(IParticleEffect*, E_RESULT_CODE&);
		public:
			TDE2_REGISTER_VIRTUAL_TYPE_EX(CSphereParticlesEmitter, GetEmitterTypeId)
			TDE2_REGISTER_TYPE(CSphereParticlesEmitter)

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
				\brief The method initializes a state of a given particle as it's used for the first time

				\param[in] pTransform A pointer to transform component that defines a position of current emitter
				\param[out] particleInfo A reference to a particle's object that should be re-initialized

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE EmitParticle(const CTransform* pTransform, TParticle& particleInfo) override;

			TDE2_API E_RESULT_CODE SetOrigin(const TVector3& origin);
			TDE2_API E_RESULT_CODE SetRadius(F32 radius);

			TDE2_API const TVector3& GetOrigin() const;
			TDE2_API F32 GetRadius() const;

			TDE2_API TEmitterUniformsData GetShaderUniformsData() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CSphereParticlesEmitter)
		protected:
			TVector3 mOrigin = ZeroVector3;
			F32      mRadius = 1.0f;
	};


	/*!
		\brief A factory function for creation objects of CConeParticlesEmitter's type

		\param[in, out] pOwnerEffect A pointer to IParticleEffect implementation
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CBaseParticlesEmitter's implementation
	*/

	TDE2_API CBaseParticlesEmitter* CreateConeParticlesEmitter(IParticleEffect* pOwnerEffect, E_RESULT_CODE& result);


	/*!
		class CConeParticlesEmitter

		\brief The class is an implementation of 3D sphere emitting particles
	*/

	class CConeParticlesEmitter : public CBaseParticlesEmitter
	{
		public:
			friend TDE2_API CBaseParticlesEmitter* CreateConeParticlesEmitter(IParticleEffect*, E_RESULT_CODE&);
		public:
			TDE2_REGISTER_VIRTUAL_TYPE_EX(CConeParticlesEmitter, GetEmitterTypeId)
			TDE2_REGISTER_TYPE(CConeParticlesEmitter)

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
				\brief The method initializes a state of a given particle as it's used for the first time

				\param[in] pTransform A pointer to transform component that defines a position of current emitter
				\param[out] particleInfo A reference to a particle's object that should be re-initialized

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE EmitParticle(const CTransform* pTransform, TParticle& particleInfo) override;

			TDE2_API E_RESULT_CODE SetHeight(F32 height);
			TDE2_API E_RESULT_CODE SetRadius(F32 radius);

			TDE2_API F32 GetHeight() const;
			TDE2_API F32 GetRadius() const;

			TDE2_API TEmitterUniformsData GetShaderUniformsData() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CConeParticlesEmitter)
		protected:
			F32 mRadius = 1.0f;
			F32 mHeight = 1.0f;
	};
}
