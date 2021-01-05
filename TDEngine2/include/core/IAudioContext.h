/*!
	\file IAudioContext.h
	\date 05.01.2021
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Types.h"
#include "../utils/Utils.h"
#include "IEngineSubsystem.h"


namespace TDEngine2
{
	/*!
		interface IAudioContext

		\brief The interface represents functionality of a low-level system that manages all audio staffs
	*/

	class IAudioContext : public IEngineSubsystem
	{
		public:
			/*!
				\brief The method initializes an initial state of the object
			
				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init() = 0;

			TDE2_API static E_ENGINE_SUBSYSTEM_TYPE GetTypeID() { return E_ENGINE_SUBSYSTEM_TYPE::EST_AUDIO_CONTEXT; }
	protected:
		DECLARE_INTERFACE_PROTECTED_MEMBERS(IAudioContext)
	};
}