/*!
	\file AudioComponents.h
	\date 07.01.2021
	\authors Kasimov Ildar
*/

#pragma once


#include "../../ecs/CBaseComponent.h"
#include <string>


namespace TDEngine2
{
	TDE2_DECLARE_FLAG_COMPONENT(AudioListenerComponent);


	/*!
		struct TAudioSourceComponentParameters

		\brief The structure contains parameters for creation of CAudioSourceComponent
	*/

	typedef struct TAudioSourceComponentParameters : public TBaseComponentParameters
	{
		std::string mAudioClipId;

		bool mIsMuted = false;
		bool mIsPaused = false;
		bool mIsLooped = false;

		F32 mVolume = 1.0f;
		F32 mPanning = 0.0f;
	} TAudioSourceComponentParameters;


	/*!
		\brief A factory function for creation objects of CAudioSourceComponent's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CAudioSourceComponent's implementation
	*/

	TDE2_API IComponent* CreateAudioSourceComponent(E_RESULT_CODE& result);


	/*!
		class CAudioSourceComponent

		\brief The class represents a component of a 3D audio source 
	*/

	class CAudioSourceComponent : public CBaseComponent, public CPoolMemoryAllocPolicy<CAudioSourceComponent, 1 << 20>
	{
		public:
			friend TDE2_API IComponent* CreateAudioSourceComponent(E_RESULT_CODE&);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CAudioSourceComponent)

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

			TDE2_API E_RESULT_CODE SetAudioClipId(const std::string& id);

			TDE2_API void SetMuted(bool value);
			TDE2_API void SetPaused(bool value);
			TDE2_API void SetLooped(bool value);
			TDE2_API void SetPlaying(bool value);

			TDE2_API E_RESULT_CODE SetVolume(F32 value);
			TDE2_API E_RESULT_CODE SetPanning(F32 value);

			TDE2_API const std::string& GetAudioClipId() const;

			TDE2_API bool IsMuted() const;
			TDE2_API bool IsPaused() const;
			TDE2_API bool IsLooped() const;
			TDE2_API bool IsPlaying() const;

			TDE2_API F32 GetVolume() const;
			TDE2_API F32 GetPanning() const;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CAudioSourceComponent)
		protected:
			std::string mAudioClipId;

			bool mIsMuted;
			bool mIsPaused;
			bool mIsLooped;
			bool mIsPlaying;

			F32 mVolume;
			F32 mPanning;
	};


	TDE2_DECLARE_COMPONENT_FACTORY(AudioSourceComponent, TAudioSourceComponentParameters);
}