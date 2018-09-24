/*!
	\file IEngineListener.h
	\date 24.09.2018
	\authors Kasimov Ildar
*/
#pragma once


#include "./../utils/Types.h"


namespace TDEngine2
{
	class IEngineCore;


	/*!
		interface IEngineListener

		\brief The interface represents a functionality of a listener of
		engine's events.
		You should implement your own logic within a derived class of this
		interface to get access to update loop, initialization.
	*/

	class IEngineListener
	{
		public:
			TDE2_API IEngineListener() = default;
			TDE2_API virtual ~IEngineListener() = default;

			/*!
				\brief The event occurs after the engine was initialized

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE OnStart() = 0;

			/*!
				\brief The event occurs every frame

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE OnUpdate(const float& dt) = 0;

			/*!
				\brief The event occurs before the engine will be destroyed

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE OnFree() = 0;

			/*!
				\brief The method is used to inject a pointer to the engine into the object of this type

				\param[in] An instance of the engine's core
			*/

			TDE2_API virtual void SetEngineInstance(IEngineCore* pEngineCore) = 0;
		protected:
			TDE2_API IEngineListener(const IEngineListener& listener) = delete;
			TDE2_API virtual IEngineListener& operator= (IEngineListener& listener) = delete;
	};
}
