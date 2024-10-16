/*!
	\file CAnimationEditorWindow.h
	\date 19.01.2021
	\authors Kasimov Ildar
*/

#pragma once


#include <TDEngine2.h>
#include <unordered_set>
#include "CTrackSheetEditor.h"


namespace TDEngine2
{
	class IResourceManager;
	class IWorld;
	class IDesktopInputContext;
	class IAnimationTrackVisitor;


	TDE2_DECLARE_SCOPED_PTR(IResourceManager);
	TDE2_DECLARE_SCOPED_PTR(IWorld);
	TDE2_DECLARE_SCOPED_PTR(IDesktopInputContext);
	TDE2_DECLARE_SCOPED_PTR(IAnimationTrackVisitor);


	/*!
		\brief A factory function for creation objects of CAnimationEditorWindow's type

		\param[in, out] pResourceManager A pointer to implementation of IResourceManager
		\param[in, out] pWorld A pointer to implementation of IWorld
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to IEditorWindow's implementation
	*/

	TDE2_API IEditorWindow* CreateAnimationEditorWindow(TPtr<IResourceManager> pResourceManager, TPtr<IWorld> pWorld, TPtr<IDesktopInputContext> pInputContext, E_RESULT_CODE& result);

	/*!
		class CAnimationEditorWindow

		\brief The class is an implementation of a window for in-game level editor
	*/

	class CAnimationEditorWindow : public CBaseEditorWindow
	{
		public:
			friend TDE2_API IEditorWindow* CreateAnimationEditorWindow(TPtr<IResourceManager>, TPtr<IWorld>, TPtr<IDesktopInputContext>, E_RESULT_CODE&);

		public:
			/*!
				\brief The method initializes internal state of the editor

				\param[in, out] pResourceManager A pointer to implementation of IResourceManager
				\param[in, out] pWorld A pointer to implementation of IWorld

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(TPtr<IResourceManager> pResourceManager, TPtr<IWorld> pWorld, TPtr<IDesktopInputContext> pInputContext);

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
			TDE2_API void _drawTimelineSheetWidget(F32 currPlaybackTime);
			TDE2_API void _drawDopesheetTracks(const TVector2& cursorPos, F32 frameWidth, F32 frameHeight, F32 pixelsPerSecond);

			TDE2_API void _drawPropertyBindingsWindow();

			TDE2_API void _onAddEventsTrackButtonHandler();
			TDE2_API void _onAddKeyframeButtonHandler();

			TDE2_API F32 _getCurrAnimationClipTime() const;

		protected:
			static const std::string        mAddPropertyWindowId;

			TResourceId                     mCurrAnimationResourceHandle;
			TPtr<IAnimationClip>            mpCurrAnimationClip;

			TEntityId                       mCurrAnimatedEntity;

			TPtr<IResourceManager>          mpResourceManager;
			TPtr<IWorld>                    mpWorld;
			TPtr<IDesktopInputContext>      mpInputContext;

			std::string                     mCurrSelectedPropertyBinding;
			TypeId                          mNewTrackTypeId;

			std::unordered_set<std::string> mUsedPropertyBindings; /// \note Contains string bindings that are already used by tracks 

			TVector2                        mTimelineScrollPosition;

			TAnimationTrackId               mSelectedTrackId = TAnimationTrackId::Invalid;

			bool                            mIsDopeSheetModeEnabled = true;
			bool                            mHasEditModeBeenChanged = true;

			CScopedPtr<CTrackSheetEditor>   mpTrackSheetEditor;

			F32                             mLastClipDuration = -1.0f;


			TPtr<IAnimationTrackVisitor>    mpPropertyCapturer;
	};


	TDE2_DECLARE_SCOPED_PTR_INLINED(CAnimationEditorWindow)
}