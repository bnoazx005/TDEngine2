/*!
	\file CMaterialEditorWindow.h
	\date 11.06.2022
	\authors Kasimov Ildar
*/

#pragma once


#include <TDEngine2.h>
#include <string>
#include <vector>


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	const std::string EditableEntityId         = "MaterialEffectEntity";
	const std::string EditableEffectResourceId = "MaterialEffectResource";


	/*!
		\brief A factory function for creation objects of CMaterialEditorWindow's type

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to IEditorWindow's implementation
	*/

	TDE2_API IEditorWindow* CreateMaterialEditorWindow(IResourceManager* pResourceManager, IInputContext* pInputContext, E_RESULT_CODE& result);

	/*!
		class CMaterialEditorWindow

		\brief The class is an implementation of a window for in-game level editor
	*/

	class CMaterialEditorWindow : public CBaseEditorWindow
	{
		public:
			friend TDE2_API IEditorWindow* CreateMaterialEditorWindow(IResourceManager*, IInputContext*, E_RESULT_CODE&);

		public:
			/*!
				\brief The method initializes internal state of the editor

				\param[in, out] desc An object that determines parameters of the window

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, IInputContext* pInputContext);

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

			TDE2_API void SetMaterialResourceHandle(TResourceId handle);

		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CMaterialEditorWindow)

			/*!
				\brief The method should be should be implemented in all derived classes. It's called
				once per frame only if the editor's window is visible
			*/

			TDE2_API void _onDraw() override;

		protected:
			TResourceId                       mCurrMaterialId;

			IResourceManager*                 mpResourceManager;
			IDesktopInputContext*             mpInputContext;

			CScopedPtr<IEditorActionsHistory> mpEditorHistory;

			TPtr<IMaterial>                   mpCurrMaterial;

	};


	TDE2_DECLARE_SCOPED_PTR_INLINED(CMaterialEditorWindow)
}

#endif