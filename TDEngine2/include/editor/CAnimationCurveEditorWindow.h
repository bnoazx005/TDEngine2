/*!
	\file CAnimationCurveEditorWindow.h
	\date 16.03.2021
	\authors Kasimov Ildar
*/

#pragma once


#include "IEditorWindow.h"
#include "../math/TRect.h"
#include "../math/TVector2.h"


namespace TDEngine2
{
	class CAnimationCurve;
	struct TVector2;


	/*!
		\brief A factory function for creation objects of CAnimationCurveEditorWindow's type

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to IEditorWindow's implementation
	*/

	TDE2_API IEditorWindow* CreateAnimationCurveEditorWindow(E_RESULT_CODE& result);


	/*!
		class CAnimationCurveEditorWindow

		\brief The class is an implementation of a window for an editor of an animation curves
	*/

	class CAnimationCurveEditorWindow : public CBaseEditorWindow
	{
		public:
			friend TDE2_API IEditorWindow* CreateAnimationCurveEditorWindow(E_RESULT_CODE&);

		public:
			typedef struct TCurveTransformParams
			{
				TRectF32 mCurveBounds;
				TVector2 mCursorPosition;
				F32      mFrameWidth;
				F32      mFrameHeight;
			} TCurveTransformParams;

		public:
			/*!
				\brief The method initializes internal state of the editor

				\param[in, out] desc An object that determines parameters of the window

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init();

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			TDE2_API E_RESULT_CODE SetCurveForEditing(CAnimationCurve* pCurve);

		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CAnimationCurveEditorWindow)

			/*!
				\brief The method should be should be implemented in all derived classes. It's called
				once per frame only if the editor's window is visible
			*/

			TDE2_API void _onDraw() override;

			TDE2_API void _drawCurveLine(F32 width, F32 height, const TVector2& cursorPos);
			TDE2_API void _drawCurvePoint(I32 id, const TVector2& pos, const TCurveTransformParams& invTransformParams);
			TDE2_API TVector2 _drawControlPoint(I32 id, const TVector2& pos, const TVector2& controlPointPos, const TCurveTransformParams& invTransformParams);

			TDE2_API void _handleCurveCursor(F32 width, F32 height, const TVector2& cursorPos);

		protected:
			static constexpr F32 mHandlePointSize = 4.0f;
			static constexpr F32 mHandlePointButtonSize = 30.0f;

			static constexpr I32 mControlPointsOffset = 1000;

			CAnimationCurve* mpCurrTargetCurve;
	};
}