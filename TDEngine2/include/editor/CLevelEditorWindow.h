/*!
	\file CLevelEditorWindow.h
	\date 17.01.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "IEditorWindow.h"
#include "../math/TRay.h"
#include <functional>
#include <unordered_map>


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	enum class E_GIZMO_TYPE : U8;

	class IEditorsManager;
	class ISelectionManager;
	class IInputContext;
	class IDesktopInputContext;
	class IDebugUtility;
	class IWorld;
	class IEditorActionsHistory;
	class ICamera;
	class IEditorWindow;
	class ISceneManager;
	class IComponent;
	class IWindowSystem;
	class IFileSystem;
	struct TEditorContext;


	TDE2_DECLARE_SCOPED_PTR(ISelectionManager)


	typedef struct TLevelEditorWindowDesc
	{
		IEditorsManager* mpEditorsManager;
		IInputContext*   mpInputContext;
		IWindowSystem*   mpWindowSystem;
		IDebugUtility*   mpDebugUtility;
		ISceneManager*   mpSceneManager;
		IFileSystem*     mpFileSystem;
	} TLevelEditorWindowDesc, *TLevelEditorWindowDescPtr;



	/*!
		\brief A factory function for creation objects of CLevelEditorWindow's type

		\param[in, out] desc An object that determines parameters of the window
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to IEditorWindow's implementation
	*/

	TDE2_API IEditorWindow* CreateLevelEditorWindow(const TLevelEditorWindowDesc& desc, E_RESULT_CODE& result);

	/*!
		class CLevelEditorWindow

		\brief The class is an implementation of a window for in-game level editor
	*/

	class CLevelEditorWindow : public CBaseEditorWindow
	{
		public:
			friend TDE2_API IEditorWindow* CreateLevelEditorWindow(const TLevelEditorWindowDesc&, E_RESULT_CODE&);

		public:
			typedef std::function<void(const TEditorContext&)> TOnGuiCallback;
			typedef std::unordered_map<TypeId, TOnGuiCallback> TInspectorsTable;

		public:
			/*!
				\brief The method initializes internal state of the editor
				
				\param[in, out] desc An object that determines parameters of the window

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(const TLevelEditorWindowDesc& desc);

			TDE2_API E_RESULT_CODE RegisterInspector(TypeId targetType, const TOnGuiCallback& onGuiFunctor);

#if TDE2_EDITORS_ENABLED

			/*!
				\brief The method rolls back last editor's action

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE ExecuteUndoAction() override;

			/*!
				\brief The method applies latest rolled back editor's action

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE ExecuteRedoAction() override;

#endif
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CLevelEditorWindow)

			/*!
				\brief The method should be should be implemented in all derived classes. It's called
				once per frame only if the editor's window is visible
			*/

			TDE2_API void _onDraw() override;

			TDE2_API void _onHandleInput();
			TDE2_API bool _onDrawGizmos();

			TDE2_API void _onDrawInspector();
			TDE2_API bool _onDrawObjectInspector();
			TDE2_API bool _onDrawSceneInspector();

			TDE2_API ISelectionManager* _getSelectionManager();

			TDE2_API E_RESULT_CODE _onFreeInternal() override;
		protected:
			IEditorsManager*       mpEditorsManager;

			IDesktopInputContext*  mpInputContext;

			TPtr<ISelectionManager> mpSelectionManager;

			IDebugUtility*         mpDebugUtility;

			IWindowSystem*         mpWindowSystem;

			ISceneManager*         mpSceneManager;

			TEntityId              mSelectedEntityId = TEntityId::Invalid;
			TEntityId              mCameraEntityId = TEntityId::Invalid;
			TEntityId              mTemporaryGroupEntityId = TEntityId::Invalid;

			E_GIZMO_TYPE           mCurrManipulatorType;

			// \todo Move into separate gizmo's context
			TVector3               mFirstPosition, mLastPosition;

			U8                     mCurrSelectedGizmoAxis = -1;

			bool                   mIsGizmoBeingDragged = false;

			bool                   mShouldRecordHistory = false;

			IEditorActionsHistory* mpActionsHistory;

			IEditorWindow*         mpHierarchyWidget;

			TInspectorsTable       mInspectorsDrawers;
	};
}

#endif