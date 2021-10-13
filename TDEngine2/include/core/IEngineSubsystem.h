/*!
	\file IEngineSubsystem.h
	\date 16.09.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Config.h"
#include "../utils/Utils.h"
#include "IBaseObject.h"


namespace TDEngine2
{
	/*!
		interface IEngineSubsystem

		\brief The interface describes a functionality of an engine's subsystem.

		The subsystem term includes low-level graphics context, window creation and etc.
	*/

	class IEngineSubsystem: public virtual IBaseObject
	{
		public:
			/*!
				\brief The method returns a type of the subsystem

				\return A type, which is represented with E_ENGINE_SUBSYSTEM_TYPE's value
			*/

			TDE2_API virtual E_ENGINE_SUBSYSTEM_TYPE GetType() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IEngineSubsystem)
	};


	TDE2_DECLARE_SCOPED_PTR_INLINED(IEngineSubsystem)
}
