/*!
	\file IEditorsManager.h
	\date 08.01.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Config.h"
#include "../utils/Types.h"
#include "../core/IEngineSubsystem.h"


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	class IImGUIContext;


	/*!
		interface IEditorsManager

		\brief The interface describes a functionality of editors manager
	*/

	class IEditorsManager: public IEngineSubsystem
	{
		public:
			/*!
				\brief The method initializes an internal state of main manager for all engine's editors

				\param[in, out] pImGUIContext A pointer to IImGUIContext implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IImGUIContext* pImGUIContext) = 0;

			/*!
				\brief The method updates the current state of the manager

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Update() = 0;

			TDE2_API static E_ENGINE_SUBSYSTEM_TYPE GetTypeID() { return EST_EDITORS_MANAGER; }
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IEditorsManager)
	};
}

#endif