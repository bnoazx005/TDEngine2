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

		\param[in, out] pResourceManager A pointer to implementation of IResourceManager
		\param[in, out] pWorld A pointer to implementation of IWorld
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to IEditorWindow's implementation
	*/

	TDE2_API IEditorWindow* CreateAnimationEditorWindow(IResourceManager* pResourceManager, IWorld* pWorld, E_RESULT_CODE& result);

	/*!
		class CAnimationEditorWindow

		\brief The class is an implementation of a window for in-game level editor
	*/

	class CAnimationEditorWindow : public CBaseEditorWindow
	{
		public:
			friend TDE2_API IEditorWindow* CreateAnimationEditorWindow(IResourceManager*, IWorld*, E_RESULT_CODE&);

		public:
			/*!
				\brief The method initializes internal state of the editor

				\param[in, out] pResourceManager A pointer to implementation of IResourceManager
				\param[in, out] pWorld A pointer to implementation of IWorld

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, IWorld* pWorld);

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			TDE2_API void SetAnimationResourceHandle(TResourceId handle);
			TDE2_API void SetAnimatedEntityId(TEntityId entity);

		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CAnimationEditorWindow)

			/*!
				\brief The method should be should be implemented in all derived classes. It's called
				once per frame only if the editor's window is visible
			*/

			TDE2_API void _onDraw() override;

			TDE2_API void _drawToolbar();
			TDE2_API void _drawTimelineEditorGroup();
			TDE2_API void _drawTracksHierarchy(F32 blockWidth);
			TDE2_API void _drawTimelineEditor(F32 blockWidth);
			TDE2_API void _drawDopesheetWidget(F32 currPlaybackTime);

		protected:
			TResourceId       mCurrAnimationResourceHandle;
			IAnimationClip*   mpCurrAnimationClip;

			TEntityId         mCurrAnimatedEntity;

			IResourceManager* mpResourceManager;
			IWorld*           mpWorld;

			bool              mIsDopeSheetModeEnabled = true;
	};
}