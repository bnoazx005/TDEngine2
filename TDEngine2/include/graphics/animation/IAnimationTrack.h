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

			TDE2_API virtual TAnimationTrackKeyId CreateKey(F32 time) = 0;
			TDE2_API virtual E_RESULT_CODE RemoveKey(TAnimationTrackKeyId handle) = 0;

			TDE2_API virtual E_RESULT_CODE Apply(IPropertyWrapper* pPropertyWrapper, F32 time) = 0;

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

			TDE2_API virtual const std::string& GetPropertyBinding() const = 0;

			TDE2_API virtual const std::string& GetName() const = 0;

			TDE2_API virtual E_ANIMATION_INTERPOLATION_MODE_TYPE GetInterpolationMode() const = 0;

			TDE2_API virtual TypeId GetTrackTypeId() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IAnimationTrack)
	};
}