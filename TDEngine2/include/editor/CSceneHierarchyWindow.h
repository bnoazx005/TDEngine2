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
	class IScene;
	class IWindowSystem;


	/*!
		\brief A factory function for creation objects of CSceneHierarchyEditorWindow's type

		\param[in, out] pSceneManager A pointer to ISceneManager implementation
				\param[in, out] pWindowSystem A pointer to IWindowSystem implementation
		\param[in, out] pSelectionManager A pointer to ISelectionManager implementation
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to IEditorWindow's implementation
	*/

	TDE2_API IEditorWindow* CreateSceneHierarchyEditorWindow(ISceneManager* pSceneManager, IWindowSystem* pWindowSystem, ISelectionManager* pSelectionManager, E_RESULT_CODE& result);

	/*!
		class CSceneHierarchyEditorWindow

		\brief The class is an implementation of a window for debugger of active render targets
	*/

	class CSceneHierarchyEditorWindow : public CBaseEditorWindow
	{
		public:
			friend TDE2_API IEditorWindow* CreateSceneHierarchyEditorWindow(ISceneManager*, IWindowSystem*, ISelectionManager*, E_RESULT_CODE&);
		public:
			/*!
				\brief The method initializes internal state of the editor

				\param[in, out] pSceneManager A pointer to ISceneManager implementation
				\param[in, out] pWindowSystem A pointer to IWindowSystem implementation
				\param[in, out] pSelectionManager A pointer to ISelectionManager implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(ISceneManager* pSceneManager, IWindowSystem* pWindowSystem, ISelectionManager* pSelectionManager);

			TDE2_API IScene* GetSelectedSceneInfo() const;

		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CSceneHierarchyEditorWindow)

			/*!
				\brief The method should be should be implemented in all derived classes. It's called
				once per frame only if the editor's window is visible
			*/

			TDE2_API void _onDraw() override;

			TDE2_API void _unloadSceneOperation(const std::string& sceneId);
			TDE2_API void _executeLoadLevelChunkOperation();

		protected:
			ISceneManager* mpSceneManager;
			IWindowSystem* mpWindowSystem;
			ISelectionManager* mpSelectionManager;

			IScene* mpSelectedScene;

			U32 mSelectedPrefabIndex = 0;

	};
}

#endif