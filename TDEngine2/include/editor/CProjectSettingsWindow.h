/*!
	\file CProjectSettingsWindow.h
	\date 25.12.2021
	\authors Kasimov Ildar
*/

#pragma once


#include "IEditorWindow.h"
#include "../core/Event.h"


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	class IEventManager;
	class IWorld;

	TDE2_DECLARE_SCOPED_PTR(IEventManager)
	TDE2_DECLARE_SCOPED_PTR(IWorld)


	/*!
		\brief A factory function for creation objects of CProjectSettingsWindow's type

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to IEditorWindow's implementation
	*/

	TDE2_API IEditorWindow* CreateProjectSettingsWindow(TPtr<IEventManager> pEventManager, E_RESULT_CODE& result);

	/*!
		class CCProjectSettingsWindow

		\brief The class is an implementation of a window that allows to change ECS subsystem settings
	*/

	class CProjectSettingsWindow : public CBaseEditorWindow, public IEventHandler
	{
		public:
			friend TDE2_API IEditorWindow* CreateProjectSettingsWindow(TPtr<IEventManager>, E_RESULT_CODE&);
		public:
			TDE2_REGISTER_TYPE(CProjectSettingsWindow)

			/*!
				\brief The method initializes internal state of the editor

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(TPtr<IEventManager> pEventManager);

			/*!
				\brief The method receives a given event and processes it

				\param[in] pEvent A pointer to event data

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE OnEvent(const TBaseEvent* pEvent) override;

			/*!
				\brief The method returns an identifier of a listener

				\return The method returns an identifier of a listener
			*/

			TDE2_API TEventListenerId GetListenerId() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CProjectSettingsWindow)

			/*!
				\brief The method should be should be implemented in all derived classes. It's called
				once per frame only if the editor's window is visible
			*/

			TDE2_API void _onDraw() override;
		protected:
			TPtr<IWorld> mpWorld;
	};
}

#endif