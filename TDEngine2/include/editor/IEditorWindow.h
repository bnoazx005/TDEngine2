/*!
	\file IEditorWindow.h
	\date 09.01.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Config.h"
#include "../utils/Types.h"
#include "../core/CBaseObject.h"
#include <atomic>
#include <string>


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	class IImGUIContext;


	/*!
		interface IEditorWindow

		\brief The interface describes functionality of a editor's window
	*/

	class IEditorWindow: public virtual IBaseObject
	{
		public:
			/*!
				\brief The method should be called once per frame and contains actual commands for
				the IMGUI context

				\param[in, out] pImGUIContext A pointer to IImGUIContext implementation
				\param[in] dt A time elapsed from last frame
			*/

			TDE2_API virtual void Draw(IImGUIContext* pImGUIContext, F32 dt) = 0;

			/*!
				\brief The method sets visibility state of the window

				\param[in] isVisible A flag that determines whether an editor's window is visible or not
			*/

			TDE2_API virtual void SetVisible(bool isVisible) = 0;

#if TDE2_EDITORS_ENABLED

			/*!
				\brief The method rolls back last editor's action

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE ExecuteUndoAction() = 0;

			/*!
				\brief The method applies latest rolled back editor's action

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE ExecuteRedoAction() = 0;

#endif

			/*!
				\brief The method returns a name of the window

				\return The method returns a name of the editor's window
			*/

			TDE2_API virtual const std::string& GetName() const = 0;

			/*!
				\brief The method returns true if the editor's window is visible, false in other cases

				\return The method returns true if the editor's window is visible, false in other cases
			*/

			TDE2_API virtual bool IsVisible() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IEditorWindow)

			/*!
				\brief The method should be should be implemented in all derived classes. It's called
				once per frame only if the editor's window is visible
			*/

			TDE2_API virtual void _onDraw() = 0;

			/*!
				\brief The method could be reimplemented in different classes, put your own update
				logic of the window here

				\param[in] dt A time elapsed from last frame
			*/

			TDE2_API virtual void _onUpdate(F32 dt) = 0;
	};


	/*!
		class CBaseEditorWindow

		\brief The class is a common implementation for all user's editors. If you need to
		implement a custom editor just derive it from this one
	*/

	class CBaseEditorWindow: public IEditorWindow, public CBaseObject
	{
		public:
			/*!
				\brief The method should be called once per frame and contains actual commands for
				the IMGUI context

				\param[in, out] pImGUIContext A pointer to IImGUIContext implementation
				\param[in] dt A time elapsed from last frame
			*/

			TDE2_API void Draw(IImGUIContext* pImGUIContext, F32 dt) override;
			
			/*!
				\brief The method sets visibility state of the window

				\param[in] isVisible A flag that determines whether an editor's window is visible or not
			*/

			TDE2_API void SetVisible(bool isVisible) override;

#if TDE2_EDITORS_ENABLED

			/*!
				\brief The method rolls back last editor's action

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE ExecuteUndoAction() override { return RC_OK; }

			/*!
				\brief The method applies latest rolled back editor's action

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE ExecuteRedoAction() override { return RC_OK; }

#endif

			/*!
				\brief The method returns a name of the window

				\return The method returns a name of the editor's window
			*/

			TDE2_API const std::string& GetName() const override;

			/*!
				\brief The method returns true if the editor's window is visible, false in other cases

				\return The method returns true if the editor's window is visible, false in other cases
			*/

			TDE2_API bool IsVisible() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseEditorWindow)

			/*!
				\brief The method could be reimplemented in different classes, put your own update
				logic of the window here

				\param[in] dt A time elapsed from last frame
			*/

			TDE2_API void _onUpdate(F32 dt) override;
		protected:
			static const U32 mAvgDeltaTimeFramesCount = 10;

			IImGUIContext*   mpImGUIContext;

			std::string      mName;

			std::atomic_bool mIsVisible;

			F32              mDeltaTimeAccumulator = 0.0f;
			F32              mCurrDeltaTime = 0.0f;

			U32              mFramesCounter = 0;
	};
}

#endif