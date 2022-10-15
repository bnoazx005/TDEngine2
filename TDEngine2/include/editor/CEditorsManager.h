/*!
	\file CEditorsManager.h
	\date 08.01.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "IEditorsManager.h"
#include "../core/CBaseObject.h"
#include "../core/Event.h"
#include <vector>
#include <string>
#include <tuple>
#include <unordered_map>


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	class IDesktopInputContext;


	enum class E_EDITOR_TYPE: U16
	{
		TIME_PROFILER,
		MEMORY_PROFILER,
		LEVEL_EDITOR,
		DEV_CONSOLE,
		RENDER_TARGET_VIEWER,
		PROJECT_SETTINGS_EDITOR,
		STATISTICS_OVERLAYED_VIEWER,
	};


	/*!
		\brief A factory function for creation objects of CEditorsManager's type

		\param[in, out] pInputContext A pointer to IInputContext implementation
		\param[in, out] pImGUIContext A pointer to IImGUIContext implementation
				\param[in, out] pEventManager A pointer to IEventManager implementation
		\param[in, out] pWorld A pointer to IWorld implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to IEditorsManager's implementation
	*/

	TDE2_API IEditorsManager* CreateEditorsManager(TPtr<IInputContext> pInputContext, TPtr<IImGUIContext> pImGUIContext, TPtr<IEventManager> pEventManager, TPtr<IWorld> pWorld, E_RESULT_CODE& result);


	/*!
		class CEditorsManager

		\brief The class implements main manager for all editors that are available within the engine
	*/

	class CEditorsManager : public IEditorsManager, public CBaseObject, public IEventHandler
	{
		public:
			friend TDE2_API IEditorsManager* CreateEditorsManager(TPtr<IInputContext>, TPtr<IImGUIContext>, TPtr<IEventManager>, TPtr<IWorld>, E_RESULT_CODE&);
		public:
			typedef std::vector<std::tuple<std::string, IEditorWindow*, bool, bool>> TEditorsArray;
		public:
			TDE2_REGISTER_TYPE(CEditorsManager)

			/*!
				\brief The method initializes an internal state of main manager for all engine's editors

				\param[in, out] pInputContext A pointer to IInputContext implementation
				\param[in, out] pImGUIContext A pointer to IImGUIContext implementation
				\param[in, out] pEventManager A pointer to IEventManager implementation
				\param[in, out] pWorld A pointer to IWorld implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(TPtr<IInputContext> pInputContext, TPtr<IImGUIContext> pImGUIContext, TPtr<IEventManager> pEventManager, TPtr<IWorld> pWorld) override;

			/*!
				\brief The method registers custom editor within the manager

				\param[in] commandName A name of a command within the development menu's which the editor will be linked to
				\param[in, out] pEditorWindow A pointer to IEditorWindow implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE RegisterEditor(const std::string& commandName, IEditorWindow* pEditorWindow, bool isSeparate = false, bool isOverlayed = false) override;

			/*!
				\brief The method registers inspector for the given component's type

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE RegisterComponentInspector(TypeId componentTypeId, const TOnDrawInspectorCallback& onDrawCallback = nullptr) override;

			/*!
				\brief The method sets up a pointer to IWorld instance

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE SetWorldInstance(TPtr<IWorld> pWorld) override;

			/*!
				\brief The method sets up a pointer to selection manager

				\param[in, out] pSelectionManager A pointer to ISelectionManager implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE SetSelectionManager(TPtr<ISelectionManager> pSelectionManager) override;

			/*!
				\brief The method updates the current state of the manager

				\param[in] dt A time elapsed from last frame

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Update(F32 dt) override;

			/*!
				\brief The method returns a type of the subsystem

				\return A type, which is represented with E_ENGINE_SUBSYSTEM_TYPE's value
			*/

			TDE2_API E_ENGINE_SUBSYSTEM_TYPE GetType() const override;
			
			/*!
				\brief The method returns true if editor mode is enabled, false in other cases

				\return The method returns true if editor mode is enabled, false in other cases
			*/

			TDE2_API bool IsEditorModeEnabled() const override;

			/*!
				\brief The method returns a pointer to ISelectionManager that's currently used in the manager
				\return The method returns a pointer to ISelectionManager that's currently used in the manager
			*/

			TDE2_API TPtr<ISelectionManager> GetSelectionManager() const override;

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

			/*!
				\brief The method returns a pointer to world's instance

				\return The method returns a pointer to world's instance
			*/

			TDE2_API TPtr<IWorld> GetWorldInstance() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CEditorsManager)

			TDE2_API E_RESULT_CODE _showEditorWindows(F32 dt);

			TDE2_API E_RESULT_CODE _onFreeInternal() override;
		public:
			static const std::unordered_map<E_EDITOR_TYPE, std::string> mEditorNamesMap;
		protected:
			TPtr<IDesktopInputContext> mpInputContext;

			TPtr<IImGUIContext>     mpImGUIContext;

			bool                    mIsVisible;

			TEditorsArray           mRegisteredEditors;

			TPtr<IEventManager>     mpEventManager;

			TPtr<IWorld>            mpWorld;

			TSystemId               mEditorCameraControlSystemId = TSystemId::Invalid;

			TPtr<ISelectionManager> mpSelectionManager;
	};
}

#endif