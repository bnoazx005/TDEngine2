/*!
	\file CParticleEffect.h
	\date 09.03.2021
	\authors Kasimov Ildar
*/

#pragma once


#include "../IMesh.h"
#include "../../core/CBaseResource.h"
#include "IParticleEffect.h"


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CParticleEffect's type

		\param[in, out] pResourceManager A pointer to IGraphicsContext's implementation
		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation
		\param[in] name A resource's name
		\param[in] id An identifier of a resource
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CParticleEffect's implementation
	*/

	TDE2_API IParticleEffect* CreateParticleEffect(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, E_RESULT_CODE& result);


	/*!
		class CParticleEffect

		\brief The class describes a functionality of a particle effect
	*/

	class CParticleEffect : public IParticleEffect, public CBaseResource
	{
		public:
			friend TDE2_API IParticleEffect* CreateParticleEffect(IResourceManager*, IGraphicsContext*, const std::string&, E_RESULT_CODE&);
		public:
			TDE2_REGISTER_RESOURCE_TYPE(CParticleEffect)
			TDE2_REGISTER_TYPE(CParticleEffect)

			/*!
				\brief The method initializes an internal state of a mesh object

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation
				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation
				\param[in] name A resource's name

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name) override;

			/*!
				\brief The method resets current internal data of a resource

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Reset() override;

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
				\brief The method sets up a duration of the effect

				\param[in] duration A positive value which determines a length of the effect's playback

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE SetDuration(F32 duration) override;

			/*!
				\brief The method enables/disables loop mode of the playback

				\param[in] value If the flag equals to true the loop mode is enabled
			*/

			TDE2_API void SetLoopMode(bool value) override;

			/*!
				\brief The method sets a upper bound for amount of emitted particles

				\param[in] A maximal limit of particles
			*/

			TDE2_API void SetMaxParticlesCount(U16 particlesCount) override;

			/*!
				\brief The method sets up a name of used material. Note that it isn't verify is material already loaded or exist

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE SetMaterialName(const std::string& value) override;

			TDE2_API E_RESULT_CODE SetLifetime(const TRangeF32& value) override;

			TDE2_API E_RESULT_CODE SetInitialSize(const TRangeF32& value) override;

			TDE2_API void SetInitialRotation(const TRangeF32& value) override;

			/*!
				\brief The method defines initial color of the particles
			*/

			TDE2_API void SetInitialColor(const TParticleColorParameter& colorData) override;

			/*!
				\brief The method defines how much particles are spawned per frame
			*/

			TDE2_API void SetEmissionRate(U32 value) override;

			/*!
				\brief The method defines a curve that controls particles size over their lifetime

				\param[in] pCurve A pointer to a curve

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE SetSizeCurve(const CScopedPtr<CAnimationCurve>& pCurve) override;

			/*!
				\brief The method updates information about enabled/disables modifiers of particles parameters

				\param[in] value A disjunction of flags that determines which modifiers are enabled

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE SetModifiersFlags(E_PARTICLE_EFFECT_INFO_FLAGS value) override;

			/*!
				\return The method returns a duration of the effect
			*/

			TDE2_API F32 GetDuration() const override;

			/*!
				\return The flag is true when loop mode is enabled on the effect
			*/

			TDE2_API bool IsLoopModeActive() const override;

			/*!
				\return The method returns a maximal number of particles
			*/

			TDE2_API U16 GetMaxParticlesCount() const override;

			/*!
				\return The method returns an identifier of a material
			*/

			TDE2_API const std::string& GetMaterialName() const override;

			TDE2_API const TRangeF32& GetLifetime() const override;

			TDE2_API const TRangeF32& GetInitialSize() const override;

			TDE2_API const TRangeF32& GetInitialRotation() const override;

			TDE2_API const TParticleColorParameter& GetInitialColor() const override;

			TDE2_API U32 GetEmissionRate() const override;

			TDE2_API CScopedPtr<CBaseParticlesEmitter> GetSharedEmitter() const override;

			TDE2_API CScopedPtr<CAnimationCurve> GetSizeCurve() const override;

			TDE2_API E_PARTICLE_EFFECT_INFO_FLAGS GetEnabledModifiersFlags() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CParticleEffect)

			TDE2_API const IResourceLoader* _getResourceLoader() override;
		protected:
			F32                               mDuration;

			bool                              mIsLooped;

			U16                               mMaxParticlesCount;

			std::string                       mMaterialName;

			TRangeF32                         mLifeTime;
			TRangeF32                         mInitialSize;
			TRangeF32                         mInitialRotation;
			TParticleColorParameter           mInitialColor;

			U32                               mEmissionRate;

			E_PARTICLE_EFFECT_INFO_FLAGS      mModifiersInfoFlags;

			CScopedPtr<CAnimationCurve>       mpSizeCurve;

			CScopedPtr<CBaseParticlesEmitter> mpSharedEmitter;
	};



	/*!
		\brief A factory function for creation objects of CParticleEffectLoader's type

		\param[in, out] pResourceManager A pointer to IResourceManager's implementation
		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation
		\param[in, out] pFileSystem A pointer to IFileSystem's implementation
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CParticleEffectLoader's implementation
	*/

	TDE2_API IResourceLoader* CreateParticleEffectLoader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem, E_RESULT_CODE& result);


	/*!
		class CParticleEffectLoader

		\brief The class implements a functionality of a base material loader
	*/

	class CParticleEffectLoader : public CBaseObject, public IMeshLoader
	{
		public:
			friend TDE2_API IResourceLoader* CreateParticleEffectLoader(IResourceManager*, IGraphicsContext*, IFileSystem*, E_RESULT_CODE&);
		public:
			/*!
				\brief The method initializes an inner state of an object

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\param[in, out] pFileSystem A pointer to IFileSystem's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem) override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method loads data into the specified resource based on its
				internal information

				\param[in, out] pResource A pointer to an allocated resource

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE LoadResource(IResource* pResource) const override;

			/*!
				\brief The method returns an identifier of a resource's type, which
				the loader serves

				\return The method returns an identifier of a resource's type, which
				the loader serves
			*/

			TDE2_API TypeId GetResourceTypeId() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CParticleEffectLoader)
		protected:
			IResourceManager* mpResourceManager;

			IFileSystem*      mpFileSystem;

			IGraphicsContext* mpGraphicsContext;
	};


	/*!
		\brief A factory function for creation objects of CParticleEffectFactory's type

		\param[in, out] pResourceManager A pointer to IResourceManager's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CParticleEffectFactory's implementation
	*/

	TDE2_API IResourceFactory* CreateParticleEffectFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);


	/*!
		class CParticleEffectFactory

		\brief The class is an abstract factory of CParticleEffect objects that
		is used by a resource manager
	*/

	class CParticleEffectFactory : public CBaseObject, public IMeshFactory
	{
		public:
			friend TDE2_API IResourceFactory* CreateParticleEffectFactory(IResourceManager*, IGraphicsContext*, E_RESULT_CODE&);
		public:
			/*!
				\brief The method initializes an internal state of a material factory

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext) override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method creates a new instance of a resource based on passed parameters

				\param[in] name A name of a resource

				\param[in] params An object that contains parameters that are needed for the resource's creation

				\return A pointer to a new instance of IResource type
			*/

			TDE2_API IResource* Create(const std::string& name, const TBaseResourceParameters& params) const override;

			/*!
				\brief The method creates a new instance of a resource based on passed parameters

				\param[in] name A name of a resource

				\param[in] params An object that contains parameters that are needed for the resource's creation

				\return A pointer to a new instance of IResource type
			*/

			TDE2_API IResource* CreateDefault(const std::string& name, const TBaseResourceParameters& params) const override;

			/*!
				\brief The method returns an identifier of a resource's type, which
				the factory serves

				\return The method returns an identifier of a resource's type, which
				the factory serves
			*/

			TDE2_API TypeId GetResourceTypeId() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CParticleEffectFactory)
		protected:
			IResourceManager* mpResourceManager;

			IGraphicsContext* mpGraphicsContext;
	};
}