/*!
	\file EditorActions.h
	\date 19.04.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Utils.h"
#include "../utils/Types.h"
#include "../utils/CResult.h"
#include "../core/IBaseObject.h"
#include "../core/CBaseObject.h"
#include <string>


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	/*!
		interface IEditorAction

		\brief The interface describes editor's action to provide undo/redo system
	*/

	class IEditorAction: public virtual IBaseObject
	{
		public:
			/*!
				\brief The method applies some action to global state of the application

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Execute() = 0;

			/*!
				\brief The method applies reversed action to restore previous state

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Restore() = 0;

			/*!
				\brief The method converts the state of the action into a string

				\return A string which contains type information and other additional information
			*/

			TDE2_API virtual std::string ToString() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IEditorAction)
	};


	/*!
		interface IEditorActionsHistory

		\brief The interface describes a functionality of actions log for the editor
	*/

	class IEditorActionsHistory: public virtual IBaseObject
	{
		public:
			/*!
				\brief The method initializes internal state of the object
			
				\return RC_OK if everything went ok, or some other code, which describes an error
			*/
			
			TDE2_API virtual E_RESULT_CODE Init() = 0;

			/*!
				\brief The method pushes given action into the log

				\param[in] pAction A pointer to IEditorAction implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE PushAction(IEditorAction* pAction) = 0;

			/*!
				\brief The method extracts last added action from the log

				\return The method retrieves either a pointer to last added action or returnrs an error code
			*/

			TDE2_API virtual TResult<IEditorAction*> PopAction() = 0;

			/*!
				\brief The method dumps current state of the log into the logger. The oldest actions are outputed
				at last momemnt
			*/

			TDE2_API virtual void Dump() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IEditorActionsHistory)
	};


	class CLevelEditorWindow;


	/*!
		\brief A factory function for creation objects of CUndoEditorAction's type

		\param[in, out] pEditorWindow A pointer to level editor's window
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CUndoEditorAction's implementation
	*/

	TDE2_API IEditorAction* CreateUndoAction(CLevelEditorWindow* pEditorWindow, E_RESULT_CODE& result);

	/*!
		class CUndoEditorAction

		\brief The class implement undo action for the editor
	*/

	class CUndoEditorAction : public IEditorAction, public CBaseObject
	{
		public:
			friend TDE2_API IEditorAction* CreateUndoAction(CLevelEditorWindow*, E_RESULT_CODE&);
		public:
			/*!
				\brief The method initializes state of the object

				\param[in, out] pEditorWindow A pointer to level editor's window

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(CLevelEditorWindow* pEditorWindow);

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method applies some action to global state of the application

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Execute() override;

			/*!
				\brief The method applies reversed action to restore previous state

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Restore() override;

			/*!
				\brief The method converts the state of the action into a string

				\return A string which contains type information and other additional information
			*/

			TDE2_API std::string ToString() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CUndoEditorAction)
		protected:
			CLevelEditorWindow* mpEditorWindow;
	};


	/*!
		\brief A factory function for creation objects of CRedoEditorAction's type

		\param[in, out] pEditorWindow A pointer to level editor's window
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CRedoEditorAction's implementation
	*/

	TDE2_API IEditorAction* CreateRedoAction(CLevelEditorWindow* pEditorWindow, E_RESULT_CODE& result);

	/*!
		class CRedoEditorAction

		\brief The class implement redo action for the editor
	*/

	class CRedoEditorAction : public IEditorAction, public CBaseObject
	{
		public:
			friend TDE2_API IEditorAction* CreateRedoAction(CLevelEditorWindow*, E_RESULT_CODE&);
		public:
			/*!
				\brief The method initializes state of the object

				\param[in, out] pEditorWindow A pointer to level editor's window

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(CLevelEditorWindow* pEditorWindow);

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method applies some action to global state of the application

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Execute() override;

			/*!
				\brief The method applies reversed action to restore previous state

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Restore() override;

			/*!
				\brief The method converts the state of the action into a string

				\return A string which contains type information and other additional information
			*/

			TDE2_API std::string ToString() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CRedoEditorAction)
		protected:
			CLevelEditorWindow* mpEditorWindow;
	};
}

#endif