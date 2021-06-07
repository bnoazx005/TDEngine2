/*!
	\file IAnimationClip.h
	\date 15.04.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "../../utils/Config.h"
#include "../../utils/Utils.h"
#include "../../core/IResourceFactory.h"
#include "../../core/IResourceLoader.h"
#include "../../core/Serialization.h"
#include "../../core/Event.h"


namespace TDEngine2
{
	class IResourceManager;
	class IGraphicsContext;
	class IAnimationTrack;


	TDE2_DECLARE_HANDLE_TYPE(TAnimationTrackId);


	/*!
		struct TAnimationPlaybackEvent

		\brief The structure represents an event which occurs when some animation is playing in AnimationContainer component
	*/

	typedef struct TAnimationPlaybackEvent : TBaseEvent
	{
		virtual ~TAnimationPlaybackEvent() = default;

		TDE2_REGISTER_TYPE(TAnimationPlaybackEvent)
		REGISTER_EVENT_TYPE(TAnimationPlaybackEvent)

		TEntityId mAnimationSourceEntityId;

		std::string mEventId;
	} TAnimationPlaybackEvent, *TAnimationPlaybackEventPtr;

	
	/*!
		enum class E_ANIMATION_WRAP_MODE_TYPE

		\brief The enumeration defines allowed types of animation playback
	*/

	enum class E_ANIMATION_WRAP_MODE_TYPE: U8
	{
		PLAY_ONCE,
		LOOP,
	};


	enum class E_ANIMATION_INTERPOLATION_MODE_TYPE: U8
	{
		CONSTANT,
		LINEAR,
		CUBIC,
	};


	/*!
		struct TAnimationClipParameters

		\brief The stucture contains minimal information for creation IAnimationClip objects
	*/

	typedef struct TAnimationClipParameters : TBaseResourceParameters
	{
		F32                        mDuration = 1.0f;

		E_ANIMATION_WRAP_MODE_TYPE mWrapMode = E_ANIMATION_WRAP_MODE_TYPE::PLAY_ONCE; ///< The wrap mode could be overwritten by animation component
		E_ANIMATION_INTERPOLATION_MODE_TYPE mInterpolationMode = E_ANIMATION_INTERPOLATION_MODE_TYPE::LINEAR;
	} TAnimationClipParameters, *TAnimationClipParametersPtr;


	/*!
		interface IAnimationClip

		\brief The interface describes API of animation clips resources
	*/

	class IAnimationClip: public ISerializable
	{
		public:
			TDE2_REGISTER_TYPE(IAnimationClip);

			/*!
				\brief The method initializes a state of a brand new animation clip.

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation
				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation
				\param[in] name A resource's name
				\param[in] params Additional parameters of a clip

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
												const TAnimationClipParameters& params) = 0;
			
			/*!
				\brief The method initializes an internal state of an animation clip

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\param[in] name A resource's name

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name) = 0;

			template <typename T>
			TDE2_API TAnimationTrackId CreateTrack(const std::string& name = "")
			{
				return _createTrackInternal(T::GetTypeId(), name);
			}

			TDE2_API virtual E_RESULT_CODE RemoveTrack(TAnimationTrackId handle) = 0;

			/*!
				\brief The method specifies duration of the clip

				\param[in] duration A time length of the animation clip, should be positive value

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE SetDuration(F32 duration) = 0;

			/*!
				\brief The method specified wrapping mode for the clip. Remember that this value probably would
				be overwritten when the clip would be assigned into animation component

				\param[in] mode The value determines how the clip would be played
			*/

			TDE2_API virtual void SetWrapMode(E_ANIMATION_WRAP_MODE_TYPE mode) = 0;

			/*!
				\brief The method returns duration of the animation clip
				\return The method returns duration of the animation clip
			*/

			TDE2_API virtual F32 GetDuration() const = 0;

			/*!
				\brief The method returns wrap mode that's defined for the clip
				\return The method returns wrap mode that's defined for the clip
			*/

			TDE2_API virtual E_ANIMATION_WRAP_MODE_TYPE GetWrapMode() const = 0;

			template <typename T>
			T* GetTrack(TAnimationTrackId handle) const
			{
				return dynamic_cast<T*>(_getTrackInternal(handle));
			}

			/*!
				\return The method returns a pointer to an animation track that controls events broadcasting. Returns nullptr if the latter doesn't exist
			*/

			TDE2_API virtual IAnimationTrack* GetEventTrack() const = 0;

			TDE2_API virtual void ForEachTrack(const std::function<void(IAnimationTrack*)>& action = nullptr) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IAnimationClip)

			TDE2_API virtual TAnimationTrackId _createTrackInternal(TypeId typeId, const std::string& name) = 0;

			TDE2_API virtual IAnimationTrack* _getTrackInternal(TAnimationTrackId handle) const = 0;
	};


	/*!
		interface IAnimationClipLoader

		\brief The interface describes a functionality of a loader of animation clips
	*/

	class IAnimationClipLoader : public IGenericResourceLoader<IResourceManager*, IGraphicsContext*, IFileSystem*> {};


	/*!
		interface IAnimationClipFactory

		\brief The interface describes a functionality of an animation clips factory
	*/

	class IAnimationClipFactory : public IResourceFactory
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
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IAnimationClipFactory)
	};
}