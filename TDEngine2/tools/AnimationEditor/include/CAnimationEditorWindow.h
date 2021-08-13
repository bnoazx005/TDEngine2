/*!
	\file CAnimationEditorWindow.h
	\date 19.01.2021
	\authors Kasimov Ildar
*/

#pragma once


#include <TDEngine2.h>


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CAnimationEditorWindow's type

		\param[in, out] desc An object that determines parameters of the window
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to IEditorWindow's implementation
	*/

	TDE2_API IEditorWindow* CreateAnimationEditorWindow(IResourceManager* pResourceManager, E_RESULT_CODE& result);

	/*!
		class CAnimationEditorWindow

		\brief The class is an implementation of a window for in-game level editor
	*/

	class CAnimationEditorWindow : public CBaseEditorWindow
	{
		public:
			friend TDE2_API IEditorWindow* CreateAnimationEditorWindow(IResourceManager*, E_RESULT_CODE&);

		public:
			/*!
				\brief The method initializes internal state of the editor

				\param[in, out] desc An object that determines parameters of the window

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager);

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			TDE2_API void SetAnimationResourceHandle(TResourceId handle);

		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CAnimationEditorWindow)

			/*!
				\brief The method should be should be implemented in all derived classes. It's called
				once per frame only if the editor's window is visible
			*/

			TDE2_API void _onDraw() override;

			TDE2_API void _drawToolbar();

		protected:
			TResourceId mCurrAnimationResourceHandle;
			IAnimationClip* mpCurrAnimationClip;

			IResourceManager* mpResourceManager;
	};
}