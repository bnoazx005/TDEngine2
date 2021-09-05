/*!
	\file CTrackSheetEditor.h
	\date 27.08.2021
	\authors Kasimov Ildar
*/

#pragma once


#include <TDEngine2.h>
#include <unordered_map>
#include <functional>


namespace TDEngine2
{
	class CAnimationCurve;


	/*!
		\brief A factory function for creation objects of CTrackSheetEditor's type

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to IEditorWindow's implementation
	*/

	TDE2_API class CTrackSheetEditor* CreateTrackSheetEditor(E_RESULT_CODE& result);


	/*!
		class CTrackSheetEditor

		\brief The class is an implementation of a widget for editing animation tracks
	*/

	class CTrackSheetEditor : public CBaseObject, public IAnimationTrackVisitor
	{
	public:
		friend TDE2_API CTrackSheetEditor* CreateTrackSheetEditor(E_RESULT_CODE&);

	public:
		typedef std::function<void(const CScopedPtr<CAnimationCurve>&)> TCurveBindingCallback;
		typedef std::function<void(const TVector2&)> TCurveDrawCallback;

		struct TTrack2CurveBindingInfo
		{
			CScopedPtr<CAnimationCurve> mpCurve;

			TCurveBindingCallback mOnInitCurvePointsCallback = nullptr;
			TCurveBindingCallback mOnSerializeCurvePointsCallback = nullptr;
		};

		typedef std::unordered_map<std::string, TTrack2CurveBindingInfo> TCurveBindingsTable;

	public:
		/*!
			\brief The method initializes internal state of the editor
	
			\return RC_OK if everything went ok, or some other code, which describes an error
		*/

		TDE2_API E_RESULT_CODE Init();

		/*!
			\brief The method frees all memory occupied by the object

			\return RC_OK if everything went ok, or some other code, which describes an error
		*/

		TDE2_API E_RESULT_CODE Free() override;

		TDE2_API E_RESULT_CODE Draw(const TVector2& frameSizes);

		TDE2_API E_RESULT_CODE Reset();

		TDE2_API void SetImGUIContext(IImGUIContext* pImGUIContext);

		TDE2_API E_RESULT_CODE VisitVector2Track(class CVector2AnimationTrack* pTrack) override;
		TDE2_API E_RESULT_CODE VisitVector3Track(class CVector3AnimationTrack* pTrack) override;
		TDE2_API E_RESULT_CODE VisitQuaternionTrack(class CQuaternionAnimationTrack* pTrack) override;
		TDE2_API E_RESULT_CODE VisitColorTrack(class CColorAnimationTrack* pTrack) override;
		TDE2_API E_RESULT_CODE VisitFloatTrack(class CFloatAnimationTrack* pTrack) override;
		TDE2_API E_RESULT_CODE VisitIntegerTrack(class CIntegerAnimationTrack* pTrack) override;
		TDE2_API E_RESULT_CODE VisitBooleanTrack(class CBooleanAnimationTrack* pTrack) override;
		TDE2_API E_RESULT_CODE VisitEventTrack(class CEventAnimationTrack* pTrack) override;

		TDE2_API bool IsEditing() const;
	private:
		DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CTrackSheetEditor)

		TDE2_API E_RESULT_CODE _resetState();
		TDE2_API void _initCurvesState();

		TDE2_API TCurveDrawCallback _generateDrawCallback();

	private:
		IImGUIContext* mpImGUIContext;

		TCurveBindingsTable mCurvesTable;

		TCurveDrawCallback  mOnDrawImpl = nullptr;

		bool mIsEditing = false;

		std::string mCurrSelectedCurveId;

		TAnimationTrackKeyId mCurrSelectedSampleId;
	};
}