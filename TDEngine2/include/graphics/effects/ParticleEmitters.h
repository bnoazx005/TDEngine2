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
#include "../../utils/Color.h"


namespace TDEngine2
{
	class IParticleEffect;
	class CTransform;
	struct TParticle;
	struct TParticleColorParameter;


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
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method deserializes object's state from given reader

				\param[in, out] pReader An input stream of data that contains information about the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API static TResult<CBaseParticlesEmitter*> Load(IArchiveReader* pReader);

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

			TDE2_API static TColor32F GetColorData(const TParticleColorParameter& colorData, F32 time);
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseParticlesEmitter)

			TDE2_API TColor32F _getInitColor() const;
			TDE2_API TVector3 _getInitVelocity() const;
		protected:
			IParticleEffect* mpOwnerEffect;

			bool mIs2DEmitter = false;
	};


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
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBoxParticlesEmitter)
		protected:
			TVector3 mBoxSizes;
			TVector3 mBoxOrigin;
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
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CSphereParticlesEmitter)
		protected:
			TVector3 mOrigin;
			F32      mRadius;
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
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CConeParticlesEmitter)
		protected:
			F32 mRadius;
			F32 mHeight;
	};
}
