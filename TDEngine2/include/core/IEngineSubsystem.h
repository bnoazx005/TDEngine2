/*!
	\file IEngineSubsystem.h
	\date 16.09.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Config.h"
#include "./../utils/Utils.h"


namespace TDEngine2
{
	/*!
		interface IEngineSubsystem

		\brief The interface describes a functionality of an engine's subsystem.

		The subsystem term includes low-level graphics context, window creation and etc.
	*/

	class IEngineSubsystem
	{
		public:
			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Free() = 0;

			/*!
				\brief The method returns a type of the subsystem

				\return A type, which is represented with E_ENGINE_SUBSYSTEM_TYPE's value
			*/

			TDE2_API virtual E_ENGINE_SUBSYSTEM_TYPE GetType() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IEngineSubsystem)
	};
}
