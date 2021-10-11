/*!
	\file ITimer.h
	\date 29.09.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../core/IBaseObject.h"
#include "./../utils/Utils.h"


namespace TDEngine2
{
	constexpr F32 DefaultFrameRate = 1.0f / 60.0f;


	/*!
		interface ITimer

		\brief The interface describes a functionality that a base
		inengine timer provides to user
	*/

	class ITimer: public virtual IBaseObject
	{
		public:
			/*!
				\brief The method initializes a ISystemManager's instance

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init() = 0;

			/*!
				\brief The method starts a timer
			*/

			TDE2_API virtual void Start() = 0;
			
			/*!
				\brief The method stops time measurement
			*/

			TDE2_API virtual void Stop() = 0;
			
			/*!
				\brief The method updates an inner state of a timer.
				Should be called every frame.
			*/

			TDE2_API virtual void Tick() = 0;

			/*!
				\brief The method returns a time's value, which has passed
				between two frames in seconds

				\return The method returns a time's value, which has passed
				between two frames
			*/

			TDE2_API virtual F32 GetDeltaTime() const = 0;

			/*!
				\brief The method returns elapsed time since a timer was started in seconds

				\return The method returns elapsed time since a timer was started in seconds
			*/

			TDE2_API virtual F32 GetCurrTime() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ITimer)
	};


	TDE2_DECLARE_SCOPED_PTR_INLINED(ITimer);
}
