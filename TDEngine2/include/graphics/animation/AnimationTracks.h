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
	template <typename T, U32 componentsCount, typename TBitsetType = U8>
	struct TGenericKeyFrame
	{
		TDE2_API TGenericKeyFrame() = default;
		TDE2_API TGenericKeyFrame(F32 time, T value) : mTime(time), mValue(value) {}

		F32          mTime;
		T            mValue;
		TBitsetType  mUsedChannels;
	};


	template <typename T, U32 componentsCount, typename TBitsetType = U8>
	struct TSplineKeyFrame: TGenericKeyFrame<T, componentsCount, TBitsetType>
	{
		TDE2_API TSplineKeyFrame() = default;
		TDE2_API TSplineKeyFrame(F32 time, T value): TGenericKeyFrame<T, componentsCount, TBitsetType>(time, value) {}

		typedef std::array<TVector2, componentsCount> TSlopesArray;

		TSlopesArray mInTangents;
		TSlopesArray mOutTangents;
	};



	typedef TSplineKeyFrame<TVector2, 2> TVector2KeyFrame, *TVector2KeyFramePtr;


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

#if TDE2_EDITORS_ENABLED
			TDE2_API E_RESULT_CODE AssignTrackForEditing(IAnimationTrackVisitor* pTrackEditor) override;
#endif

		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CVector2AnimationTrack)

			TDE2_API E_RESULT_CODE _saveKeyFrameValue(const TVector2KeyFrame& value, IArchiveWriter* pWriter) override;
			TDE2_API E_RESULT_CODE _loadKeyFrameValue(TAnimationTrackKeyId keyHandle, IArchiveReader* pReader) override;

			TDE2_API TVector2KeyFrame _lerpKeyFrames(const TVector2KeyFrame& left, const TVector2KeyFrame& right, F32 t) const override;
			TDE2_API TVector2KeyFrame _cubicInterpolation(const TVector2KeyFrame& left, const TVector2KeyFrame& right, F32 t, F32 frameDelta) const override;
		protected:

	};


	typedef TSplineKeyFrame<TVector3, 3> TVector3KeyFrame, *TVector3KeyFramePtr;


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

#if TDE2_EDITORS_ENABLED
			TDE2_API E_RESULT_CODE AssignTrackForEditing(IAnimationTrackVisitor* pTrackEditor) override;
#endif

		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CVector3AnimationTrack)

			TDE2_API E_RESULT_CODE _saveKeyFrameValue(const TVector3KeyFrame& value, IArchiveWriter* pWriter) override;
			TDE2_API E_RESULT_CODE _loadKeyFrameValue(TAnimationTrackKeyId keyHandle, IArchiveReader* pReader) override;

			TDE2_API TVector3KeyFrame _lerpKeyFrames(const TVector3KeyFrame& left, const TVector3KeyFrame& right, F32 t) const override;
			TDE2_API TVector3KeyFrame _cubicInterpolation(const TVector3KeyFrame& left, const TVector3KeyFrame& right, F32 t, F32 frameDelta) const override;
		protected:

	};


	/*!
		\brief TQuaternionKeyFrame
	*/


	typedef TSplineKeyFrame<TQuaternion, 4> TQuaternionKeyFrame, *TQuaternionKeyFramePtr;


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

#if TDE2_EDITORS_ENABLED
			TDE2_API E_RESULT_CODE AssignTrackForEditing(IAnimationTrackVisitor* pTrackEditor) override;
#endif

		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CQuaternionAnimationTrack)

			TDE2_API E_RESULT_CODE _saveKeyFrameValue(const TQuaternionKeyFrame& value, IArchiveWriter* pWriter) override;
			TDE2_API E_RESULT_CODE _loadKeyFrameValue(TAnimationTrackKeyId keyHandle, IArchiveReader* pReader) override;

			TDE2_API TQuaternionKeyFrame _lerpKeyFrames(const TQuaternionKeyFrame& left, const TQuaternionKeyFrame& right, F32 t) const override;
			TDE2_API TQuaternionKeyFrame _cubicInterpolation(const TQuaternionKeyFrame& left, const TQuaternionKeyFrame& right, F32 t, F32 frameDelta) const override;
		protected:

	};


	/*!
		\brief TColorKeyFrame
	*/


	typedef TSplineKeyFrame<TColor32F, 4> TColorKeyFrame, *TColorKeyFramePtr;


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

#if TDE2_EDITORS_ENABLED
			TDE2_API E_RESULT_CODE AssignTrackForEditing(IAnimationTrackVisitor* pTrackEditor) override;
#endif

		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CColorAnimationTrack)

			TDE2_API E_RESULT_CODE _saveKeyFrameValue(const TColorKeyFrame& value, IArchiveWriter* pWriter) override;
			TDE2_API E_RESULT_CODE _loadKeyFrameValue(TAnimationTrackKeyId keyHandle, IArchiveReader* pReader) override;

			TDE2_API TColorKeyFrame _lerpKeyFrames(const TColorKeyFrame& left, const TColorKeyFrame& right, F32 t) const override;
			TDE2_API TColorKeyFrame _cubicInterpolation(const TColorKeyFrame& left, const TColorKeyFrame& right, F32 t, F32 frameDelta) const override;
		protected:

	};


	/*!
		\brief TBooleanKeyFrame
	*/


	typedef TGenericKeyFrame<bool, 1> TBooleanKeyFrame, *TBooleanKeyFramePtr;


	/*!
		\brief A factory function for creation objects of CBooleanAnimationTrack's type

		\param[in, out] pOwner A pointer to IAnimationClip implementation
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CBooleanAnimationTrack's implementation
	*/

	TDE2_API IAnimationTrack* CreateBooleanAnimationTrack(IAnimationClip* pOwner, E_RESULT_CODE& result);


	class CBooleanAnimationTrack : public CBaseAnimationTrack<TBooleanKeyFrame>
	{
		public:
			friend TDE2_API IAnimationTrack* CreateBooleanAnimationTrack(IAnimationClip*, E_RESULT_CODE&);
		public:
			TDE2_REGISTER_TYPE(CBooleanAnimationTrack);
			TDE2_REGISTER_VIRTUAL_TYPE_EX(CBooleanAnimationTrack, GetTrackTypeId);

			TDE2_API E_RESULT_CODE Apply(IPropertyWrapper* pPropertyWrapper, F32 time) override;

#if TDE2_EDITORS_ENABLED
			TDE2_API E_RESULT_CODE AssignTrackForEditing(IAnimationTrackVisitor* pTrackEditor) override;
#endif

		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBooleanAnimationTrack)

			TDE2_API E_RESULT_CODE _saveKeyFrameValue(const TBooleanKeyFrame& value, IArchiveWriter* pWriter) override;
			TDE2_API E_RESULT_CODE _loadKeyFrameValue(TAnimationTrackKeyId keyHandle, IArchiveReader* pReader) override;

			TDE2_API TBooleanKeyFrame _lerpKeyFrames(const TBooleanKeyFrame& left, const TBooleanKeyFrame& right, F32 t) const override;
			TDE2_API TBooleanKeyFrame _cubicInterpolation(const TBooleanKeyFrame& left, const TBooleanKeyFrame& right, F32 t, F32 frameDelta) const override;
		protected:
	};


	/*!
		\brief TFloatKeyFrame
	*/


	typedef TSplineKeyFrame<F32, 1> TFloatKeyFrame, *TFloatKeyFramePtr;


	/*!
		\brief A factory function for creation objects of CFloatAnimationTrack's type

		\param[in, out] pOwner A pointer to IAnimationClip implementation
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CFloatAnimationTrack's implementation
	*/

	TDE2_API IAnimationTrack* CreateFloatAnimationTrack(IAnimationClip* pOwner, E_RESULT_CODE& result);


	class CFloatAnimationTrack : public CBaseAnimationTrack<TFloatKeyFrame>
	{
		public:
			friend TDE2_API IAnimationTrack* CreateFloatAnimationTrack(IAnimationClip*, E_RESULT_CODE&);
		public:
			TDE2_REGISTER_TYPE(CFloatAnimationTrack);
			TDE2_REGISTER_VIRTUAL_TYPE_EX(CFloatAnimationTrack, GetTrackTypeId);

			TDE2_API E_RESULT_CODE Apply(IPropertyWrapper* pPropertyWrapper, F32 time) override;

#if TDE2_EDITORS_ENABLED
			TDE2_API E_RESULT_CODE AssignTrackForEditing(IAnimationTrackVisitor* pTrackEditor) override;
#endif

		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CFloatAnimationTrack)

			TDE2_API E_RESULT_CODE _saveKeyFrameValue(const TFloatKeyFrame& value, IArchiveWriter* pWriter) override;
			TDE2_API E_RESULT_CODE _loadKeyFrameValue(TAnimationTrackKeyId keyHandle, IArchiveReader* pReader) override;

			TDE2_API TFloatKeyFrame _lerpKeyFrames(const TFloatKeyFrame& left, const TFloatKeyFrame& right, F32 t) const override;
			TDE2_API TFloatKeyFrame _cubicInterpolation(const TFloatKeyFrame& left, const TFloatKeyFrame& right, F32 t, F32 frameDelta) const override;
		protected:
	};


	/*!
		\brief TIntegerKeyFrame
	*/


	typedef TGenericKeyFrame<I32, 1> TIntegerKeyFrame, *TIntegerKeyFramePtr;


	/*!
		\brief A factory function for creation objects of CIntegerAnimationTrack's type

		\param[in, out] pOwner A pointer to IAnimationClip implementation
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CIntegerAnimationTrack's implementation
	*/

	TDE2_API IAnimationTrack* CreateIntegerAnimationTrack(IAnimationClip* pOwner, E_RESULT_CODE& result);


	class CIntegerAnimationTrack : public CBaseAnimationTrack<TIntegerKeyFrame>
	{
		public:
			friend TDE2_API IAnimationTrack* CreateIntegerAnimationTrack(IAnimationClip*, E_RESULT_CODE&);
		public:
			TDE2_REGISTER_TYPE(CIntegerAnimationTrack);
			TDE2_REGISTER_VIRTUAL_TYPE_EX(CIntegerAnimationTrack, GetTrackTypeId);

			TDE2_API E_RESULT_CODE Apply(IPropertyWrapper* pPropertyWrapper, F32 time) override;

#if TDE2_EDITORS_ENABLED
			TDE2_API E_RESULT_CODE AssignTrackForEditing(IAnimationTrackVisitor* pTrackEditor) override;
#endif

		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CIntegerAnimationTrack)

			TDE2_API E_RESULT_CODE _saveKeyFrameValue(const TIntegerKeyFrame& value, IArchiveWriter* pWriter) override;
			TDE2_API E_RESULT_CODE _loadKeyFrameValue(TAnimationTrackKeyId keyHandle, IArchiveReader* pReader) override;

			TDE2_API TIntegerKeyFrame _lerpKeyFrames(const TIntegerKeyFrame& left, const TIntegerKeyFrame& right, F32 t) const override;
			TDE2_API TIntegerKeyFrame _cubicInterpolation(const TIntegerKeyFrame& left, const TIntegerKeyFrame& right, F32 t, F32 frameDelta) const override;
		protected:
	};


	/*!
		\brief TEventKeyFrame
	*/


	typedef TGenericKeyFrame<std::string, 1> TEventKeyFrame, *TEventKeyFramePtr;


	/*!
		\brief A factory function for creation objects of CEventAnimationTrack's type

		\param[in, out] pOwner A pointer to IAnimationClip implementation
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CEventAnimationTrack's implementation
	*/

	TDE2_API IAnimationTrack* CreateEventAnimationTrack(IAnimationClip* pOwner, E_RESULT_CODE& result);


	class CEventAnimationTrack : public CBaseAnimationTrack<TEventKeyFrame>
	{
		public:
			friend TDE2_API IAnimationTrack* CreateEventAnimationTrack(IAnimationClip*, E_RESULT_CODE&);
		public:
			TDE2_REGISTER_TYPE(CEventAnimationTrack);
			TDE2_REGISTER_VIRTUAL_TYPE_EX(CEventAnimationTrack, GetTrackTypeId);

			TDE2_API E_RESULT_CODE Apply(IPropertyWrapper* pPropertyWrapper, F32 time) override;

#if TDE2_EDITORS_ENABLED
			TDE2_API E_RESULT_CODE AssignTrackForEditing(IAnimationTrackVisitor* pTrackEditor) override;
#endif

		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CEventAnimationTrack)

			TDE2_API E_RESULT_CODE _saveKeyFrameValue(const TEventKeyFrame& value, IArchiveWriter* pWriter) override;
			TDE2_API E_RESULT_CODE _loadKeyFrameValue(TAnimationTrackKeyId keyHandle, IArchiveReader* pReader) override;

			TDE2_API TEventKeyFrame _lerpKeyFrames(const TEventKeyFrame& left, const TEventKeyFrame& right, F32 t) const override;
			TDE2_API TEventKeyFrame _cubicInterpolation(const TEventKeyFrame& left, const TEventKeyFrame& right, F32 t, F32 frameDelta) const override;
		protected:
	};
}