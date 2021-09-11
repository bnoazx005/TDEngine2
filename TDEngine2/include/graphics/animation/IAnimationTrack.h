/*!
	\file IAnimationTrack.h
	\date 18.04.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "../../utils/Utils.h"
#include "../../utils/Types.h"
#include "../../core/IBaseObject.h"
#include "../../core/Serialization.h"


namespace TDEngine2
{
	class IAnimationClip;
	class IPropertyWrapper;

	enum class E_ANIMATION_INTERPOLATION_MODE_TYPE : U8;


	TDE2_DECLARE_HANDLE_TYPE(TAnimationTrackKeyId);


	/*!
		interface IAnimationTrack

		\brief The interface represents functionality of animation tracks
	*/

	class IAnimationTrack: public ISerializable, public virtual IBaseObject
	{
		public:
			/*!
				\brief The method initializes an internal state of an animation track

				\param[in, out] pTrackOwner A pointer to IAnimationClip which holds the track

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IAnimationClip* pTrackOwner) = 0;

			/*!
				\brief The method creates a new sample in the track at the given time position

				\param[in] time A position of a created sample
				\param[in] pChannelsUsageMask A bitset that tells which channels of the track are used in the sample. Pass nullptr to skip overwriting of previous value if 
				the key is already exist. The invokation with nullptr for the sample that isn't in the track generates a new one with 0xFF as usedChannels mask
			*/

			TDE2_API virtual TAnimationTrackKeyId CreateKey(F32 time, const U8* pChannelsUsageMask = nullptr) = 0;
			TDE2_API virtual E_RESULT_CODE RemoveKey(TAnimationTrackKeyId handle) = 0;

			TDE2_API virtual E_RESULT_CODE Apply(IPropertyWrapper* pPropertyWrapper, F32 time) = 0;

#if TDE2_EDITORS_ENABLED
			TDE2_API virtual E_RESULT_CODE AssignTrackForEditing(class IAnimationTrackVisitor* pTrackEditor) = 0;
#endif

			TDE2_API virtual void RemoveAllKeys() = 0;

			/*!
				\brief The method specifies interpolation mode for tracks

				\param[in] mode The value determines sampling type
			*/

			TDE2_API virtual void SetInterpolationMode(E_ANIMATION_INTERPOLATION_MODE_TYPE mode) = 0;

			/*!
				\brief The method assign a string which contains a name of property that will be animated
				by this track

				\param[in] binding Current format looks like the following component_name.property_name

				\return RC_OK if everything went ok, or some other code, which describes an error				
			*/

			TDE2_API virtual E_RESULT_CODE SetPropertyBinding(const std::string& binding) = 0;
			
			TDE2_API virtual E_RESULT_CODE SetName(const std::string& name) = 0;

#if TDE2_EDITORS_ENABLED
			TDE2_API virtual E_RESULT_CODE UpdateKeyTime(TAnimationTrackKeyId keyId, F32 value, bool reorderKeys = false) = 0;
#endif

			TDE2_API virtual const std::string& GetPropertyBinding() const = 0;

			TDE2_API virtual const std::string& GetName() const = 0;

			TDE2_API virtual E_ANIMATION_INTERPOLATION_MODE_TYPE GetInterpolationMode() const = 0;

			TDE2_API virtual TAnimationTrackKeyId GetKeyHandleByTime(F32 value) const = 0;

			TDE2_API virtual std::vector<F32> GetSamples() const = 0;

			TDE2_API virtual TypeId GetTrackTypeId() const = 0;

			TDE2_API virtual IAnimationClip* GetOwner() = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IAnimationTrack)
	};


#if TDE2_EDITORS_ENABLED

	class IAnimationTrackVisitor : public virtual IBaseObject
	{
		public:
			TDE2_API virtual E_RESULT_CODE VisitVector2Track(class CVector2AnimationTrack* pTrack) = 0;
			TDE2_API virtual E_RESULT_CODE VisitVector3Track(class CVector3AnimationTrack* pTrack) = 0;
			TDE2_API virtual E_RESULT_CODE VisitQuaternionTrack(class CQuaternionAnimationTrack* pTrack) = 0;
			TDE2_API virtual E_RESULT_CODE VisitColorTrack(class CColorAnimationTrack* pTrack) = 0;
			TDE2_API virtual E_RESULT_CODE VisitFloatTrack(class CFloatAnimationTrack* pTrack) = 0;
			TDE2_API virtual E_RESULT_CODE VisitIntegerTrack(class CIntegerAnimationTrack* pTrack) = 0;
			TDE2_API virtual E_RESULT_CODE VisitBooleanTrack(class CBooleanAnimationTrack* pTrack) = 0;
			TDE2_API virtual E_RESULT_CODE VisitEventTrack(class CEventAnimationTrack* pTrack) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IAnimationTrackVisitor)
	};

#endif
}