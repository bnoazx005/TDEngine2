/*!
	\file CEditorActionsManager.h
	\date 20.04.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "EditorActions.h"
#include "../core/CBaseObject.h"
#include <vector>


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CEditorActionsManager's type

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CEditorActionsManager's implementation
	*/

	TDE2_API IEditorActionsHistory* CreateEditorActionsManager(E_RESULT_CODE& result);


	/*!
		class CEditorActionsManager

		\brief The class represents container of all editor actions that were performed in past
	*/

	class CEditorActionsManager : public IEditorActionsHistory, public CBaseObject
	{
		public:
			friend TDE2_API IEditorActionsHistory* CreateEditorActionsManager(E_RESULT_CODE&);
		public:
			/*!
				\brief The method initializes internal state of the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init() override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method pushes given action into the log

				\param[in] pAction A pointer to IEditorAction implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE PushAction(IEditorAction* pAction) override;

			/*!
				\brief The method extracts last added action from the log

				\return The method retrieves either a pointer to last added action or returnrs an error code
			*/

			TDE2_API TResult<IEditorAction*> PopAction() override;

			/*!
				\brief The method dumps current state of the log into the logger
			*/

			TDE2_API void Dump() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CEditorActionsManager)
		protected:
			std::vector<IEditorAction*> mpActionsHistory;
	};
}

#endif