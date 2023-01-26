/*!
	\file IJobManager.h
	\date 18.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Types.h"
#include "../utils/Utils.h"
#include "IEngineSubsystem.h"
#include "memory/CBaseAllocator.h"
#include <functional>
#include <memory>
#include <atomic>


namespace TDEngine2
{
	struct TJobDecl;
	struct TJobCounter;


	struct TJobArgs
	{
		U32 mJobIndex = 0;
		U32 mGroupIndex = 0;
		TJobDecl* mpCurrJob = nullptr;
	};
		

	typedef struct TJobManagerInitParams
	{
		U32                      mMaxNumOfThreads;								///< A maximum number of threads that will be created and processed by the manager
		TAllocatorFactoryFunctor mAllocatorFactoryFunctor;						///< Allocator's factory, used to allocate buffers for fibers stacks
		USIZE                    mFiberStackSize = 64 * 1024;					///< A stack's size for a single allocated fiber
		U32                      mFibersPoolSize = 128;							///< Amount of created fibers that will be used by the manager
	} TJobManagerInitParams, *TJobManagerInitParamsPtr;


	/*! 
		\brief All the values correspond to the only entity from the job queues. There are 3 queues + extra explicit "Main" queue in the engine respectively
	*/
	
	enum class E_JOB_PRIORITY_TYPE : U8
	{
		LOW, NORMAL, HIGH
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

				\param[in] desc A configuration of the job system 

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(const TJobManagerInitParams& desc) = 0;

			/*!
				\brief The method pushes specified job into a queue for an execution

				\param[in] pCounter A pointer to created object of counter. Can be nullptr if synchronization isn't needed
				\param[in] job A callback with the task that will be executed
				\param[in] priority The value determines into which queue the given job will be submited
				\param[in] jobName An optional identifier for the job

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE SubmitJob(TJobCounter* pCounter, const TJobCallback& job, E_JOB_PRIORITY_TYPE priority = E_JOB_PRIORITY_TYPE::NORMAL, const C8* jobName = "TDE2Job") = 0;
			
			/*!
				\brief The method is an equvivalent for "parallel_for" algorithm that splits some complex work between groups and
				executes the given job for all of them

				\param[in] pCounter A pointer to created object of counter. Can be nullptr if synchronization isn't needed
				\param[in] job A callback with the task that will be executed
				\param[in] priority The value determines into which queue jobs will be submited

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE SubmitMultipleJobs(TJobCounter* pCounter, U32 jobsCount, U32 groupSize, const TJobCallback& job, E_JOB_PRIORITY_TYPE priority = E_JOB_PRIORITY_TYPE::NORMAL) = 0;

			/*!
				\brief The function represents an execution barrier to make sure that any dependencies are finished to the point

				\param[in, out] counter A reference to syncronization context
				\param[in] counterThreshold A value to compare with context's one
				\param[in] pAwaitingJob There is should be a pointer to a job that emits another one and should wait for its completion. In other cases pass nullptr
			*/

			TDE2_API virtual void WaitForJobCounter(TJobCounter& counter, U32 counterThreshold = 0, TJobDecl* pAwaitingJob = nullptr) = 0;

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
