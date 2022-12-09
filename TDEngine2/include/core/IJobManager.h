/*!
	\file IJobManager.h
	\date 18.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Types.h"
#include "../utils/Utils.h"
#include "IEngineSubsystem.h"
#include <functional>
#include <memory>
#include <atomic>


namespace TDEngine2
{
	/*!
		struct TJobCounter

		\brief The type is used to create a syncronization points within the main thread to explicitly schedule dependencies
	*/

	struct TJobCounter
	{
		std::atomic<U32> mValue { 0 };
	};


	struct TJobArgs
	{
		U32 mJobIndex = 0;
		U32 mGroupIndex = 0;
	};
		

	/*!
		interface IJobManager

		\brief The interface describes a functionality of a thread pool
	*/

	class IJobManager : public IEngineSubsystem
	{
		public:
			typedef std::function<void(const TJobArgs&)> TJobCallback;
		public:
			/*!
				\brief The method initializes an inner state of a resource manager

				\param[in] maxNumOfThreads A maximum number of threads that will be created and processed by the manager

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(U32 maxNumOfThreads) = 0;

			/*!
				\brief The method pushes specified job into a queue for an execution

				\param[in] pCounter A pointer to created object of counter. Can be nullptr if synchronization isn't needed
				\param[in] job A callback with the task that will be executed

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE SubmitJob(TJobCounter* pCounter, const TJobCallback& job) = 0;
			
			/*!
				\brief The method is an equvivalent for "parallel_for" algorithm that splits some complex work between groups and
				executes the given job for all of them

				\param[in] pCounter A pointer to created object of counter. Can be nullptr if synchronization isn't needed
				\param[in] job A callback with the task that will be executed

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE SubmitMultipleJobs(TJobCounter* pCounter, U32 jobsCount, U32 groupSize, const TJobCallback& job) = 0;

			/*!
				\brief The function represents an execution barrier to make sure that any dependencies are finished to the point

				\param[in] counter A reference to syncronization context
				\param[in] counterThreshold A value to compare with context's one
			*/

			TDE2_API virtual void WaitForJobCounter(const TJobCounter& counter, U32 counterThreshold = 0) = 0;

			/*!
				\brief The method allows to execute some code from main thread nomatter from which thread it's called

				\param[in] action A function that should be executed in the main thread

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE ExecuteInMainThread(const std::function<void()>& action = nullptr) = 0;

			/*!
				\brief The method unrolls main thread's queue of actions that should be executed only in the main thread
			*/

			TDE2_API virtual void ProcessMainThreadQueue() = 0;

			TDE2_API static E_ENGINE_SUBSYSTEM_TYPE GetTypeID() { return EST_JOB_MANAGER; }
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IJobManager)
	};


	TDE2_DECLARE_SCOPED_PTR(IJobManager)
}
