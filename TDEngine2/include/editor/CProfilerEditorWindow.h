/*!
	\file CProfilerEditorWindow.h
	\date 10.01.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "IEditorWindow.h"


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	class IProfiler;


	/*!
		\brief A factory function for creation objects of CProfilerEditorWindow's type

		\param[in, out] pProfiler A pointer to IProfiler implementation
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to IEditorWindow's implementation
	*/

	TDE2_API IEditorWindow* CreateProfilerEditorWindow(IProfiler* pProfiler, E_RESULT_CODE& result);

	/*!
		class CProfilerEditorWindow

		\brief The class is an implementation of a window for in-game profiler
	*/

	class CProfilerEditorWindow : public CBaseEditorWindow
	{
		public:
			friend TDE2_API IEditorWindow* CreateProfilerEditorWindow(IProfiler* pProfiler, E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes internal state of the editor

				\param[in, out] pProfiler A pointer to IProfiler implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IProfiler* pProfiler);

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CProfilerEditorWindow)

			/*!
				\brief The method should be should be implemented in all derived classes. It's called
				once per frame only if the editor's window is visible
			*/

			TDE2_API void _onDraw() override;
		protected:
			IProfiler* mpProfiler;
	};
}

#endif