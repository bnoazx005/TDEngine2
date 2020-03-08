/*!
	\file CLevelEditorWindow.h
	\date 17.01.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "IEditorWindow.h"


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	class IEditorsManager;
	class ISelectionManager;
	class IInputContext;
	class IDesktopInputContext;


	/*!
		\brief A factory function for creation objects of CLevelEditorWindow's type

		\param[in, out] pEditorsManager A pointer to IEditorsManager implementation
		\param[in, out] pInputContext A pointer to IInputContext implementation
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to IEditorWindow's implementation
	*/

	TDE2_API IEditorWindow* CreateLevelEditorWindow(IEditorsManager* pEditorsManager, IInputContext* pInputContext, E_RESULT_CODE& result);

	/*!
		class CLevelEditorWindow

		\brief The class is an implementation of a window for in-game level editor
	*/

	class CLevelEditorWindow : public CBaseEditorWindow
	{
		public:
			friend TDE2_API IEditorWindow* CreateLevelEditorWindow(IEditorsManager* pEditorsManager, IInputContext* pInputContext, E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes internal state of the editor
				
				\param[in, out] pEditorsManager A pointer to IEditorsManager implementation
				\param[in, out] pInputContext A pointer to IInputContext implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IEditorsManager* pEditorsManager, IInputContext* pInputContext);

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CLevelEditorWindow)

			/*!
				\brief The method should be should be implemented in all derived classes. It's called
				once per frame only if the editor's window is visible
			*/

			TDE2_API void _onDraw() override;

			TDE2_API void _onHandleInput();

			TDE2_API ISelectionManager* _getSelectionManager();
		protected:
			IEditorsManager*      mpEditorsManager;

			IDesktopInputContext* mpInputContext;

			ISelectionManager*    mpSelectionManager;
	};
}

#endif