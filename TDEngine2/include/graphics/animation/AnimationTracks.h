/*!
	\file AnimationTracks.h
	\date 26.01.2021
	\authors Kasimov Ildar
*/

#pragma once


#include "CBaseAnimationTrack.h"
#include "../../math/TVector2.h"
#include "../../math/TVector3.h"
#include "../../math/TQuaternion.h"
#include "../../utils/Color.h"


namespace TDEngine2
{
	typedef struct TVector2KeyFrame
	{
		F32 mTime;
		TVector2 mValue;
	} TVector2KeyFrame, *TVector2KeyFramePtr;


	/*!
		\brief A factory function for creation objects of CVector2AnimationTrack's type

		\param[in, out] pOwner A pointer to IAnimationClip implementation
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CVector2AnimationTrack's implementation
	*/

	TDE2_API IAnimationTrack* CreateVector2AnimationTrack(IAnimationClip* pOwner, E_RESULT_CODE& result);


	class CVector2AnimationTrack : public CBaseAnimationTrack<TVector2KeyFrame>
	{
		public:
			friend TDE2_API IAnimationTrack* CreateVector2AnimationTrack(IAnimationClip*, E_RESULT_CODE&);
		public:
			TDE2_REGISTER_TYPE(CVector2AnimationTrack);
			TDE2_REGISTER_VIRTUAL_TYPE_EX(CVector2AnimationTrack, GetTrackTypeId);

			TDE2_API E_RESULT_CODE Apply(IPropertyWrapper* pPropertyWrapper, F32 time) override;

			/*!
				\brief The method deserializes object's state from given reader

				\param[in, out] pReader An input stream of data that contains information about the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Load(IArchiveReader* pReader) override;

		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CVector2AnimationTrack)

			TDE2_API E_RESULT_CODE _saveKeyFrameValue(const TVector2KeyFrame& value, IArchiveWriter* pWriter) override;
		protected:

	};


	typedef struct TVector3KeyFrame
	{
		F32 mTime;
		TVector3 mValue;
	} TVector3KeyFrame, *TVector3KeyFramePtr;


	/*!
		\brief A factory function for creation objects of CVector3AnimationTrack's type

		\param[in, out] pOwner A pointer to IAnimationClip implementation
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CVector3AnimationTrack's implementation
	*/

	TDE2_API IAnimationTrack* CreateVector3AnimationTrack(IAnimationClip* pOwner, E_RESULT_CODE& result);


	class CVector3AnimationTrack : public CBaseAnimationTrack<TVector3KeyFrame>
	{
		public:
			friend TDE2_API IAnimationTrack* CreateVector3AnimationTrack(IAnimationClip*, E_RESULT_CODE&);
		public:
			TDE2_REGISTER_TYPE(CVector3AnimationTrack);
			TDE2_REGISTER_VIRTUAL_TYPE_EX(CVector3AnimationTrack, GetTrackTypeId);

			TDE2_API E_RESULT_CODE Apply(IPropertyWrapper* pPropertyWrapper, F32 time) override;

			/*!
				\brief The method deserializes object's state from given reader

				\param[in, out] pReader An input stream of data that contains information about the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Load(IArchiveReader* pReader) override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CVector3AnimationTrack)

			TDE2_API E_RESULT_CODE _saveKeyFrameValue(const TVector3KeyFrame& value, IArchiveWriter* pWriter) override;
		protected:

	};


	/*!
		\brief TQuaternionKeyFrame
	*/


	typedef struct TQuaternionKeyFrame
	{
		F32 mTime;
		TQuaternion mValue;
	} TQuaternionKeyFrame, *TQuaternionKeyFramePtr;


	/*!
		\brief A factory function for creation objects of CQuaternionAnimationTrack's type

		\param[in, out] pOwner A pointer to IAnimationClip implementation
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CQuaternionAnimationTrack's implementation
	*/

	TDE2_API IAnimationTrack* CreateQuaternionAnimationTrack(IAnimationClip* pOwner, E_RESULT_CODE& result);


	class CQuaternionAnimationTrack : public CBaseAnimationTrack<TQuaternionKeyFrame>
	{
		public:
			friend TDE2_API IAnimationTrack* CreateQuaternionAnimationTrack(IAnimationClip*, E_RESULT_CODE&);
		public:
			TDE2_REGISTER_TYPE(CQuaternionAnimationTrack);
			TDE2_REGISTER_VIRTUAL_TYPE_EX(CQuaternionAnimationTrack, GetTrackTypeId);

			TDE2_API E_RESULT_CODE Apply(IPropertyWrapper* pPropertyWrapper, F32 time) override;

			/*!
				\brief The method deserializes object's state from given reader

				\param[in, out] pReader An input stream of data that contains information about the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Load(IArchiveReader* pReader) override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CQuaternionAnimationTrack)

			TDE2_API E_RESULT_CODE _saveKeyFrameValue(const TQuaternionKeyFrame& value, IArchiveWriter* pWriter) override;
		protected:

	};


	/*!
		\brief TColorKeyFrame
	*/


	typedef struct TColorKeyFrame
	{
		F32 mTime;
		TColor32F mValue;
	} TColorKeyFrame, *TColorKeyFramePtr;


	/*!
		\brief A factory function for creation objects of CColorAnimationTrack's type

		\param[in, out] pOwner A pointer to IAnimationClip implementation
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CColorAnimationTrack's implementation
	*/

	TDE2_API IAnimationTrack* CreateColorAnimationTrack(IAnimationClip* pOwner, E_RESULT_CODE& result);


	class CColorAnimationTrack : public CBaseAnimationTrack<TColorKeyFrame>
	{
		public:
			friend TDE2_API IAnimationTrack* CreateColorAnimationTrack(IAnimationClip*, E_RESULT_CODE&);
		public:
			TDE2_REGISTER_TYPE(CColorAnimationTrack);
			TDE2_REGISTER_VIRTUAL_TYPE_EX(CColorAnimationTrack, GetTrackTypeId);

			TDE2_API E_RESULT_CODE Apply(IPropertyWrapper* pPropertyWrapper, F32 time) override;

			/*!
				\brief The method deserializes object's state from given reader

				\param[in, out] pReader An input stream of data that contains information about the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Load(IArchiveReader* pReader) override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CColorAnimationTrack)

			TDE2_API E_RESULT_CODE _saveKeyFrameValue(const TColorKeyFrame& value, IArchiveWriter* pWriter) override;
		protected:

	};
}