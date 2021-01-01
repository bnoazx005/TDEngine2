/*!
	\file CProfilerEditorWindow.h
	\date 10.01.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "IEditorWindow.h"
#include "./../editor/IProfiler.h"
#include "./../math/TRect.h"
#include "./../utils/Color.h"
#include <string>
#include <vector>


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CProfilerEditorWindow's type

		\param[in, out] pProfiler A pointer to IProfiler implementation
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to IEditorWindow's implementation
	*/

	TDE2_API IEditorWindow* CreateProfilerEditorWindow(ITimeProfiler* pProfiler, E_RESULT_CODE& result);

	/*!
		class CProfilerEditorWindow

		\brief The class is an implementation of a window for in-game profiler
	*/

	class CProfilerEditorWindow : public CBaseEditorWindow
	{
		public:
			friend TDE2_API IEditorWindow* CreateProfilerEditorWindow(ITimeProfiler* pProfiler, E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes internal state of the editor

				\param[in, out] pProfiler A pointer to IProfiler implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(ITimeProfiler* pProfiler);

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method sets up an identifier of the main thread

				\param[in] mainThreadID An identifier of the main thread
			*/

			TDE2_API virtual void SetMainThreadID(U32 mainThreadID);
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CProfilerEditorWindow)

			/*!
				\brief The method should be should be implemented in all derived classes. It's called
				once per frame only if the editor's window is visible
			*/

			TDE2_API void _onDraw() override;

			TDE2_API void _drawIntervalsTree(IImGUIContext& imguiContext, const TVector2& initPosition, const ITimeProfiler::TSampleRecord& currSample, 
											 ITimeProfiler::TSamplesArray& samples, F32 pixelsPerMillisecond, I16 currTrackId = 0);

			TDE2_API static TRectF32 _drawRectWithText(IImGUIContext& imguiContext, const std::string& text, const TRectF32& rect, const TColor32F& rectColor,
													   const TColor32F& textColor);
		protected:
			static const U16 mBufferSize = 128;

			ITimeProfiler*       mpProfiler;

			F32              mFrameRatesBuffer[mBufferSize];

			F32              mIntervalRectHeight = 20.0f;

			TVector2         mSpacingSizes = TVector2(2.0f, 2.0f);

			U32              mMainThreadID;
	};
}

#endif