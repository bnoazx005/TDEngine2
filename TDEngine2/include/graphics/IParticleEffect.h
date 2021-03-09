/*!
	\file IParticleEffect.h
	\date 09.03.2021
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Types.h"
#include "../core/IResource.h"
#include "../core/IResourceFactory.h"
#include "../core/IResourceLoader.h"
#include "../core/Serialization.h"


namespace TDEngine2
{

	/*!
		struct TParticleEffect2DParameters

		\brief The stucture contains fields for creation IParticleEffect and its derived classes
	*/

	typedef struct TParticleEffectParameters : TBaseResourceParameters
	{
		F32  mDuration;
		bool mIsLooped;
		U16  mMaxParticlesCount;
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