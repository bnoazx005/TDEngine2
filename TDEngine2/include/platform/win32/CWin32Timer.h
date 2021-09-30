/*!
	\file CWin32Timer.h
	\date 29.09.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../../utils/Config.h"


#if defined (TDE2_USE_WIN32PLATFORM)

#include "./../../core/CBaseObject.h"
#include "./../../utils/ITimer.h"
#include <Windows.h>


namespace TDEngine2
{
	/*!
		class Cwin32Timer

		\brief The class implements a timer for Win32 platform
		based on QueryPerformanceCounter function
	*/

	class CWin32Timer : public CBaseObject, public ITimer
	{
		public:
			friend TDE2_API ITimer* CreateWin32Timer(E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes a ISystemManager's instance

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init() override;

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
				between two frames in seconds

				\return The method returns a time's value, which has passed
				between two frames in seconds
			*/

			TDE2_API F32 GetDeltaTime() const override;

			/*!
				\brief The method returns elapsed time since a timer was started in seconds

				\return The method returns elapsed time since a timer was started in seconds
			*/

			TDE2_API F32 GetCurrTime() const override;
		protected:
			TDE2_API CWin32Timer();
			TDE2_API ~CWin32Timer() = default;
			TDE2_API CWin32Timer(const CWin32Timer& timer) = delete;
			TDE2_API CWin32Timer& operator=(const CWin32Timer& timer) = delete;
		protected:
			LARGE_INTEGER mPrevTime;
			LARGE_INTEGER mCurrTime;
			LARGE_INTEGER mStartTime;
			LARGE_INTEGER mStopTime;
			F32           mFrequency;
			F32           mDeltaTime;
	};


	/*!
		\brief A factory function for creation objects of CWin32Timer's type.

		\return A pointer to CWin32Timer's implementation
	*/

	TDE2_API ITimer* CreateWin32Timer(E_RESULT_CODE& result);
}

#endif