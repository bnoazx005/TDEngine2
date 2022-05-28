/*!
	\file IEditorsManager.h
	\date 08.01.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Config.h"
#include "../utils/Types.h"
#include "../core/IEngineSubsystem.h"
#include "../core/Event.h"
#include <string>
#include <functional>


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	class IImGUIContext;
	class IInputContext;
	class IEditorWindow;
	class IWorld;
	class ISelectionManager;
	class IEventManager;
	struct TEditorContext;


	TDE2_DECLARE_SCOPED_PTR(IInputContext)
	TDE2_DECLARE_SCOPED_PTR(IImGUIContext)
	TDE2_DECLARE_SCOPED_PTR(IEventManager)
	TDE2_DECLARE_SCOPED_PTR(IWorld)
	TDE2_DECLARE_SCOPED_PTR(ISelectionManager)


	/*!
		interface IEditorsManager

		\brief The interface describes a functionality of editors manager
	*/

	class IEditorsManager: public IEngineSubsystem
	{
		public:
			typedef std::function<void(const TEditorContext&)> TOnDrawInspectorCallback;
		public:
			/*!
				\brief The method initializes an internal state of main manager for all engine's editors
				
				\param[in, out] pInputContext A pointer to IInputContext implementation
				\param[in, out] pImGUIContext A pointer to IImGUIContext implementation
				\param[in, out] pEventManager A pointer to IEventManager implementation
				\param[in, out] pWorld A pointer to IWorld implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(TPtr<IInputContext> pInputContext, TPtr<IImGUIContext> pImGUIContext, TPtr<IEventManager> pEventManager, TPtr<IWorld> pWorld) = 0;

			/*!
				\brief The method registers custom editor within the manager

				\param[in] commandName A name of a command within the development menu's which the editor will be linked to
				\param[in, out] pEditorWindow A pointer to IEditorWindow implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE RegisterEditor(const std::string& commandName, IEditorWindow* pEditorWindow, bool isSeparate = false) = 0;

			/*!
				\brief The method registers inspector for the given component's type
				
				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE RegisterComponentInspector(TypeId componentTypeId, const TOnDrawInspectorCallback& onDrawCallback = nullptr) = 0;

			/*!
				\brief The method sets up a pointer to IWorld instance

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE SetWorldInstance(TPtr<IWorld> pWorld) = 0;

			/*!
				\brief The method sets up a pointer to selection manager

				\param[in, out] pSelectionManager A pointer to ISelectionManager implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE SetSelectionManager(TPtr<ISelectionManager> pSelectionManager) = 0;

			/*!
				\brief The method updates the current state of the manager

				\return RC_OK if everything went ok, or some other code, which describes an error
				\param[in] dt A time elapsed from last frame
			*/

			TDE2_API virtual E_RESULT_CODE Update(F32 dt) = 0;

			/*!
				\brief The method returns true if editor mode is enabled, false in other cases
				
				\return The method returns true if editor mode is enabled, false in other cases
			*/

			TDE2_API virtual bool IsEditorModeEnabled() const = 0;

			/*!
				\brief The method returns a pointer to ISelectionManager that's currently used in the manager
				\return The method returns a pointer to ISelectionManager that's currently used in the manager
			*/

			TDE2_API virtual TPtr<ISelectionManager> GetSelectionManager() const = 0;

			/*!
				\brief The method returns a pointer to world's instance

				\return The method returns a pointer to world's instance
			*/

			TDE2_API virtual TPtr<IWorld> GetWorldInstance() const = 0;

			TDE2_API static E_ENGINE_SUBSYSTEM_TYPE GetTypeID() { return EST_EDITORS_MANAGER; }
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IEditorsManager)
	};


	TDE2_DECLARE_SCOPED_PTR_INLINED(IEditorsManager)


	/*!
		struct TOnEditorModeEnabled

		\brief The structure represents an event which occurs when a user activates level editor's mode
	*/

	typedef struct TOnEditorModeEnabled : TBaseEvent
	{
		TDE2_EVENT(TOnEditorModeEnabled);
	} TOnEditorModeEnabled, *TOnEditorModeEnabledPtr;


	/*!
		struct TOnEditorModeDisabled

		\brief The structure represents an event which occurs when a user closes the level editor's
	*/

	typedef struct TOnEditorModeDisabled : TBaseEvent
	{
		TDE2_EVENT(TOnEditorModeDisabled);
	} TOnEditorModeDisabled, *TOnEditorModeDisabledPtr;
}

#endif