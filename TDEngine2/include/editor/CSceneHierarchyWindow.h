/*!
	\file CSceneHierarchyWindow.h
	\date 08.12.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "IEditorWindow.h"


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	class ISceneManager;
	class ISelectionManager;


	/*!
		\brief A factory function for creation objects of CSceneHierarchyEditorWindow's type

		\param[in, out] pSceneManager A pointer to ISceneManager implementation
		\param[in, out] pSelectionManager A pointer to ISelectionManager implementation
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to IEditorWindow's implementation
	*/

	TDE2_API IEditorWindow* CreateSceneHierarchyEditorWindow(ISceneManager* pSceneManager, ISelectionManager* pSelectionManager, E_RESULT_CODE& result);

	/*!
		class CSceneHierarchyEditorWindow

		\brief The class is an implementation of a window for debugger of active render targets
	*/

	class CSceneHierarchyEditorWindow : public CBaseEditorWindow
	{
		public:
			friend TDE2_API IEditorWindow* CreateSceneHierarchyEditorWindow(ISceneManager*, ISelectionManager*, E_RESULT_CODE&);
		public:
			/*!
				\brief The method initializes internal state of the editor

				\param[in, out] pSceneManager A pointer to ISceneManager implementation
				\param[in, out] pSelectionManager A pointer to ISelectionManager implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(ISceneManager* pSceneManager, ISelectionManager* pSelectionManager);

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CSceneHierarchyEditorWindow)

			/*!
				\brief The method should be should be implemented in all derived classes. It's called
				once per frame only if the editor's window is visible
			*/

			TDE2_API void _onDraw() override;
		protected:
			ISceneManager* mpSceneManager;
			ISelectionManager* mpSelectionManager;

	};
}

#endif