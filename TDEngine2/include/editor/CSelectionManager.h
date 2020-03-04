/*!
	\file CSelectionManager.h
	\date 27.02.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "ISelectionManager.h"
#include "../core/CBaseObject.h"
#include "../core/Event.h"


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CSelectionManager's type.

		\param[in, out] pEditorsManager A pointer to IEditorsManager implementation
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CSelectionManager's implementation
	*/

	TDE2_API ISelectionManager* CreateSelectionManager(IEditorsManager* pEditorsManager, E_RESULT_CODE& result);


	/*!
		class CSelectionManager

		\brief The class implements a logic of manager that controls all selected objects
	*/

	class CSelectionManager : public CBaseObject, public ISelectionManager
	{
		public:
			friend TDE2_API ISelectionManager* CreateSelectionManager(IEditorsManager* pEditorsManager, E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_TYPE(CSelectionManager)

			/*!
				\brief The method initializes the internal state of the manager

				\param[in, out] pEditorsManager A pointer to IEditorsManager implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IEditorsManager* pEditorsManager) override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method builds so called selection map which is a render target that contains
				information about which object occupies each pixel of the screen

				\param[in] onDrawVisibleObjectsCallback A callback in which all visible objects should be drawn

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE BuildSelectionMap(const TRenderFrameCallback& onDrawVisibleObjectsCallback) override;

			/*!
				\brief The method receives a given event and processes it

				\param[in] pEvent A pointer to event data

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE OnEvent(const TBaseEvent* pEvent) override;

			/*!
				\brief The method sets up a pointer to IWorld instance

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE SetWorldInstance(IWorld* pWorld) override;

			/*!
				\brief The method returns an identifier of a listener

				\return The method returns an identifier of a listener
			*/

			TDE2_API TEventListenerId GetListenerId() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CSelectionManager)
		protected:
			IEditorsManager* mpEditorsManager;

			IWorld*          mpWorld;

			TSystemId        mObjectSelectionSystemId;
	};
}

#endif