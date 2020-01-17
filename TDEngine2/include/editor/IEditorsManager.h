/*!
	\file IEditorsManager.h
	\date 08.01.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Config.h"
#include "../utils/Types.h"
#include "../core/IEngineSubsystem.h"
#include <string>


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	class IImGUIContext;
	class IInputContext;
	class IEditorWindow;


	/*!
		interface IEditorsManager

		\brief The interface describes a functionality of editors manager
	*/

	class IEditorsManager: public IEngineSubsystem
	{
		public:
			/*!
				\brief The method initializes an internal state of main manager for all engine's editors
				
				\param[in, out] pInputContext A pointer to IInputContext implementation
				\param[in, out] pImGUIContext A pointer to IImGUIContext implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IInputContext* pInputContext, IImGUIContext* pImGUIContext) = 0;

			/*!
				\brief The method registers custom editor within the manager

				\param[in] commandName A name of a command within the development menu's which the editor will be linked to
				\param[in, out] pEditorWindow A pointer to IEditorWindow implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE RegisterEditor(const std::string& commandName, IEditorWindow* pEditorWindow) = 0;

			/*!
				\brief The method updates the current state of the manager

				\return RC_OK if everything went ok, or some other code, which describes an error
				\param[in] dt A time elapsed from last frame
			*/

			TDE2_API virtual E_RESULT_CODE Update(F32 dt) = 0;

			TDE2_API static E_ENGINE_SUBSYSTEM_TYPE GetTypeID() { return EST_EDITORS_MANAGER; }
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IEditorsManager)
	};
}

#endif