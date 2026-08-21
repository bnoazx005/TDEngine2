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


	CLASS_META(SECTION = ecs, flags = SERIALIZE_MARKED_ONLY_FIELDS)
	struct TAudioSourceComponentData
	{
		FIELD_META(name = clip_id) std::string mAudioClipId = "";

		FIELD_META(name = is_muted) bool       mIsMuted = false;
		FIELD_META(name = is_paused) bool      mIsPaused = false;
		FIELD_META(name = is_looped) bool      mIsLooped = false;
		bool                                   mIsPlaying = false;

		FIELD_META(name = volume) F32          mVolume = 1.0f;
		FIELD_META(name = panning) F32         mPanning = 0.0f;

		TDE2_DECLARE_COMPONENT_META(TAudioSourceComponentData);
	};


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

	class CAudioSourceComponent: public CBaseComponentT<CAudioSourceComponent, TAudioSourceComponentData>
	{
		public:
			friend TDE2_API IComponent* CreateAudioSourceComponent(E_RESULT_CODE&);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CAudioSourceComponent)

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
	};


	TDE2_DECLARE_COMPONENT_FACTORY(AudioSourceComponent, TAudioSourceComponentParameters);
}