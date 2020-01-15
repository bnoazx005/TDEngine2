/*!
	\file IProfiler.h
	\date 07.01.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Config.h"
#include "../utils/Types.h"
#include "../core/IBaseObject.h"


namespace TDEngine2
{
	class ITimer;


	/*!
		interface IProfiler

		\brief The interface describes functionality of built-in performace profiler system.
		Note that all profilers implementation should be singletons
	*/

	class IProfiler : public virtual IBaseObject
	{
		public:
			typedef struct TSampleRecord
			{
				F32         mStartTime = 0.0f;
				F32         mDuration = 0.0f;

				U32         mThreadID = 0x0;

				std::string mName;
			} TSampleRecord, *TSampleRecordPtr;

			typedef std::vector<TSampleRecord>             TSamplesArray;
			typedef std::unordered_map<U32, TSamplesArray> TSamplesTable;
			typedef std::vector<TSamplesTable>             TSamplesLog;
			typedef std::vector<F32>                       TFramesTimesLog;
		public:
			/*!
				\brief The method stars to record current frame's statistics. The method should be called only once per frame

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE BeginFrame() = 0;
			
			/*!
				\brief The method stops recording statistics. The method should be called only once per frame

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE EndFrame() = 0;

			/*!
				\brief The method writes measurement's sample into profiler's table

				\param[in] name A name of the sample
				\param[in] startTime A time when the record of the sample was started to record
				\param[in] duration A elapsed time's value for this sample
				\param[in] threadID An identifier of a thread
			*/

			TDE2_API virtual void WriteSample(const std::string& name, F32 startTime, F32 duration, U32 threadID) = 0;

			/*!
				\brief The method returns instrumental timer that's used for measurements

				\return The method returns instrumental timer that's used for measurements
			*/

			TDE2_API virtual ITimer* GetTimer() const = 0;

			/*!
				\brief The method returns an array which contains log of frames's timings. The method
				is better to use when you want to get contigious block of data instead of retrieving particular samples

				\return The method returns an array which contains log of frames's timings
			*/

			TDE2_API virtual const std::vector<F32>& GetFramesTimes() const = 0;

			/*
				\brief The method returns an index of a frame which time is the worstest between other ones

				\return The method returns an index of a frame which time is the worstest between other ones
			*/

			TDE2_API virtual U16 GetWorstFrameIndexByTime() const = 0;

			/*!
				\brief The method returns samples based on a given frame's index

				\param[in] frameIndex Frame's index

				\return The method returns samples based on a given frame's index
			*/

			TDE2_API virtual const TSamplesTable& GetSamplesLogByFrameIndex(U32 frameIndex) const = 0;

			/*!
				\brief The function is replacement of factory method for instances of this type.
				The only instance will be created per program's lifetime. To destroy it call Free
				as for any other type within the engine

				\return A pointer to an instance of IProfiler type 
			*/

			TDE2_API static IProfiler* Get();
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IProfiler);
	};
}