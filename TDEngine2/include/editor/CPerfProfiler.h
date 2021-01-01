/*!
	\file CPerfProfiler.h
	\date 07.01.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "IProfiler.h"
#include "../core/CBaseObject.h"
#include "../utils/ITimer.h"
#include <string>
#include <stack>
#include <thread>
#include <unordered_map>


namespace TDEngine2
{

	/*!
		class CPerfProfiler

		\brief The interface describes functionality of built-in performace profiler system.
		Note that all profilers implementation should be singletons
	*/

	class CPerfProfiler : public ITimeProfiler, public CBaseObject
	{
		public:
			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method stars to record current frame's statistics. The method should be called only once per frame

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE BeginFrame() override;

			/*!
				\brief The method stops recording statistics. The method should be called only once per frame

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE EndFrame() override;

			/*!
				\brief The method writes measurement's sample into profiler's table

				\param[in] name A name of the sample
				\param[in] startTime A time when the record of the sample was started to record
				\param[in] duration A elapsed time's value for this sample
				\param[in] threadID An identifier of a thread
			*/

			TDE2_API void WriteSample(const std::string& name, F32 startTime, F32 duration, U32 threadID) override;

			/*!
				\brief The method returns instrumental timer that's used for measurements

				\return The method returns instrumental timer that's used for measurements
			*/

			TDE2_API ITimer* GetTimer() const override;

			/*!
				\brief The method returns an array which contains log of frames's timings. The method
				is better to use when you want to get contigious block of data instead of retrieving particular samples

				\return The method returns an array which contains log of frames's timings
			*/

			TDE2_API const std::vector<F32>& GetFramesTimes() const override;

			/*
				\brief The method returns an index of a frame which time is the worstest between other ones

				\return The method returns an index of a frame which time is the worstest between other ones
			*/

			TDE2_API U16 GetWorstFrameIndexByTime() const override;


			/*!
				\brief The method returns samples based on a given frame's index

				\param[in] frameIndex Frame's index

				\return The method returns samples based on a given frame's index
			*/

			TDE2_API const TSamplesTable& GetSamplesLogByFrameIndex(U32 frameIndex) const override;

			/*!
				\brief The function is replacement of factory method for instances of this type.
				The only instance will be created per program's lifetime. To destroy it call Free
				as for any other type within the engine

				\return A pointer to an instance of IProfiler type
			*/

			TDE2_API static ITimeProfiler* Get();
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CPerfProfiler);
		private:
			static const U16    mLogBuffer;

			ITimer*             mpPerformanceTimer;
			
			bool                mIsRecording;

			TSamplesLog         mFramesStatistics;

			TFramesTimesLog     mFramesTimesStatistics;

			U32                 mCurrFrameIndex;

			U32                 mWorstTimeFrameIndex;

	};


	/*!
		class CProfilerScope

		\brief The class is a wrapper which's used to make measurements within a scope
	*/

	class CProfilerScope
	{
		public:
			CProfilerScope(const std::string& name):
				mName(name)
			{
				ITimeProfiler* pProfiler = CPerfProfiler::Get();

				ITimer* pTimer = pProfiler->GetTimer();
				pTimer->Tick();

				mStartTime = pTimer->GetCurrTime();
			}

			~CProfilerScope()
			{
				ITimeProfiler* pProfiler = CPerfProfiler::Get();

				ITimer* pTimer = pProfiler->GetTimer();
				pTimer->Tick();

				mEndTime = pTimer->GetCurrTime();

				pProfiler->WriteSample(mName, mStartTime, mEndTime - mStartTime, std::hash<std::thread::id>{}(mThreadID));
			}
		private:
			std::string     mName;

			F32             mStartTime = 0.0f;
			F32             mEndTime = 0.0f;

			std::thread::id mThreadID = std::this_thread::get_id();
	};


#define TDE2_PROFILER_SCOPE(Name) CProfilerScope scope##__LINE__(Name)
}