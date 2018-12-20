/*!
	\file CUnixTimer.h
	\date 12.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../../utils/Config.h"


#if defined (TDE2_USE_UNIXPLATFORM)

#include "./../../core/CBaseObject.h"
#include "./../../utils/ITimer.h"
#include <sys/time.h>


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CUnixTimer's type.

		\return A pointer to CUnixTimer's implementation
	*/

	TDE2_API ITimer* CreateUnixTimer(E_RESULT_CODE& result);


	/*!
		class CUnixTimer

		\brief The class implements a timer for UNIX platform
	*/

	class CUnixTimer : public CBaseObject, public ITimer
	{
		public:
			friend TDE2_API ITimer* CreateUnixTimer(E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes a ISystemManager's instance

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init() override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method starts a timer
			*/

			TDE2_API void Start() override;

			/*!
				\brief The method stops time measurement
			*/

			TDE2_API void Stop() override;

			/*!
				\brief The method updates an inner state of a timer.
				Should be called every frame.
			*/

			TDE2_API void Tick() override;

			/*!
				\brief The method returns a time's value, which has passed
				between two frames

				\return The method returns a time's value, which has passed
				between two frames
			*/

			TDE2_API F32 GetDeltaTime() const override;

			/*!
				\brief The method returns elapsed time since a timer was started

				\return The method returns elapsed time since a timer was started
			*/

			TDE2_API F32 GetCurrTime() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CUnixTimer)
		protected:
			timeval mPrevTime;
			timeval mCurrTime;
			timeval mStartTime;
			timeval mStopTime;
			F32     mDeltaTime;
	};
}

#endif