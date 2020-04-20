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
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IEditorActionsHistory)
	};
}