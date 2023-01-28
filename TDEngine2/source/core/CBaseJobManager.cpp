#include "./../../include/core/CBaseJobManager.h"
#include "./../../include/utils/CFileLogger.h"
#include "tina.h"
#include "stringUtils.hpp"
#include <cmath>

#ifdef TDE2_USE_WINPLATFORM
#include "optick.h"
#endif


#define TDE2_JOB_MANAGER_VERBOSE_LOG_ENABLED 1


namespace TDEngine2
{
	CBaseJobManager::CBaseJobManager():
		CBaseObject()
	{
	}


	static void* ProcessFiberRoutine(tina* pCoroutine, void* pData)
	{
		while (true)
		{
			TJobDecl* pJobDecl = reinterpret_cast<TJobDecl*>(pData);

			TJobArgs args;
			args.mJobIndex = pJobDecl->mJobIndex;
			args.mGroupIndex = pJobDecl->mGroupIndex;
			args.mpCurrJob = pJobDecl;
			args.mpJobManager = pJobDecl->mpJobManager;

			(pJobDecl->mJob)(args);

			pJobDecl->mIsCompleted = true;

			tina_swap(pCoroutine, pJobDecl->mpThreadFiber, nullptr);
		}

		TDE2_UNREACHABLE();
		return nullptr;
	}

	E_RESULT_CODE CBaseJobManager::Init(const TJobManagerInitParams& desc)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (desc.mMaxNumOfThreads < 1 || !desc.mAllocatorFactoryFunctor)
		{
			return RC_INVALID_ARGS;
		}

		E_RESULT_CODE result = RC_OK;

		mpFibersStackAllocator = TPtr<IAllocator>((desc.mAllocatorFactoryFunctor)((desc.mFibersPoolSize + 1) * desc.mFiberStackSize, result));

		if (!mpFibersStackAllocator || RC_OK != result)
		{
			return result;
		}

		mNumOfThreads = desc.mMaxNumOfThreads;
		mIsRunning = true;

		for (U32 i = 0; i < mNumOfThreads; ++i)
		{
			mWorkerThreads.emplace_back(&CBaseJobManager::_executeTasksLoop, this);
		}

		/// \note Create fibers 
		{
			void* pFibersStackBlock = mpFibersStackAllocator->Allocate(desc.mFibersPoolSize * desc.mFiberStackSize, __alignof(U32));
			U8* pCurrFiberStack = reinterpret_cast<U8*>(pFibersStackBlock);

			for (U32 i = 0; i < desc.mFibersPoolSize; i++)
			{
				mFreeFibersPool.push(tina_init(pCurrFiberStack, desc.mFiberStackSize, ProcessFiberRoutine, nullptr));
				pCurrFiberStack += desc.mFiberStackSize;

				TDE2_ASSERT(mFreeFibersPool.back());
			}
		}

		mUpdateCounter = 0;

		mIsInitialized = true;

		LOG_MESSAGE("[Job Manager] The job manager was successfully initialized...");
		LOG_MESSAGE("[Job Manager] " + std::to_string(mNumOfThreads) + " threads were created");

		return RC_OK;
	}
	
	E_RESULT_CODE CBaseJobManager::_onFreeInternal()
	{
		mIsRunning = false;

		mHasNewJobAdded.notify_all();

		// wait for all working threads
		for (std::thread& currThread : mWorkerThreads)
		{
			if (currThread.joinable())
			{
				currThread.join();
			}
		}

		LOG_MESSAGE("[Job Manager] The job manager was successfully destroyed");

		return RC_OK;
	}


	static const auto MainThreadId = std::this_thread::get_id();


	bool CBaseJobManager::IsMainThread()
	{
		return MainThreadId == std::this_thread::get_id();
	}

	E_RESULT_CODE CBaseJobManager::SubmitJob(TJobCounter* pCounter, const TJobCallback& job, E_JOB_PRIORITY_TYPE priority, const C8* jobName)
	{
		if (!job)
		{
			return RC_INVALID_ARGS;
		}

		TJobDecl jobDecl;
		jobDecl.mJob = job;
		jobDecl.mpCounter = pCounter;
		jobDecl.mpJobName = jobName;
		jobDecl.mpJobManager = this;

		if (pCounter)
		{
			pCounter->mValue.fetch_add(1);
		}

#if TDE2_JOB_MANAGER_VERBOSE_LOG_ENABLED
		LOG_MESSAGE(Wrench::StringUtils::Format("[Job Manager] Submit a new job, id: {0}, counter_value: {1}", jobName, (pCounter ? pCounter->mValue.load() : 0)));
#endif

		std::lock_guard<std::mutex> lock(mQueueMutex);

		mJobs.emplace(std::move(jobDecl));

		mHasNewJobAdded.notify_one();

		return RC_OK;
	}

	E_RESULT_CODE CBaseJobManager::SubmitMultipleJobs(TJobCounter* pCounter, U32 jobsCount, U32 groupSize, const TJobCallback& job, E_JOB_PRIORITY_TYPE priority)
	{
		if (!job || !groupSize || groupSize > jobsCount)
		{
			return RC_INVALID_ARGS;
		}

		const U32 groupsCount = static_cast<U32>(::ceilf(jobsCount / static_cast<F32>(groupSize)));

		if (pCounter)
		{
			pCounter->mValue.fetch_add(jobsCount);
		}

		std::lock_guard<std::mutex> lock(mQueueMutex);

		for (U32 groupId = 0; groupId < groupsCount; groupId++)
		{
			for (U32 jobIndex = groupId * groupSize; jobIndex < (groupId + 1) * groupSize && jobIndex < jobsCount; jobIndex++)
			{
				TJobDecl jobDecl;
				jobDecl.mJob = job;
				jobDecl.mpCounter = pCounter;
				jobDecl.mJobIndex = jobIndex;
				jobDecl.mGroupIndex = groupId;
				jobDecl.mpJobManager = this;

				mJobs.emplace(std::move(jobDecl));
			}			
		}

		mHasNewJobAdded.notify_all();

		return RC_OK;
	}

	void CBaseJobManager::WaitForJobCounter(TJobCounter& counter, U32 counterThreshold, TJobDecl* pAwaitingJob)
	{
		if (counter.mValue <= counterThreshold)
		{
			return;
		}

		if (!pAwaitingJob)
		{
			mHasNewJobAdded.notify_all();

			while (counter.mValue > counterThreshold)
			{
				std::this_thread::yield();
			}

			return;
		}

		pAwaitingJob->mWaitingCounterThreshold = counterThreshold;

		/// \note Push waiting job into the TJobCounter's awating list
		{
			std::lock_guard<std::mutex> lock(counter.mWaitingJobListMutex);
			counter.mpWaitingJobList.emplace(*pAwaitingJob); /// \fixme Copying, not optimal solution
		}
#if 0
		{
			pAwaitingJob->mpNextAwaitingJob = counter.mpWaitingJobList.load();

			while (!counter.mpWaitingJobList.compare_exchange_weak(pAwaitingJob->mpNextAwaitingJob, pAwaitingJob))
			{
			}
		}
#endif

#if TDE2_JOB_MANAGER_VERBOSE_LOG_ENABLED
		LOG_MESSAGE(Wrench::StringUtils::Format("[Job Manager] The job pushed to waiting list, id: {0}, counter_value: {1}", 
			pAwaitingJob->mpJobName, counter.mValue.load()));
#endif

		tina_swap(pAwaitingJob->mpFiber, pAwaitingJob->mpThreadFiber, nullptr);
	}

	E_RESULT_CODE CBaseJobManager::ExecuteInMainThread(const std::function<void()>& action)
	{
		if (!action)
		{
			return RC_INVALID_ARGS;
		}

		if (IsMainThread()) /// \note Execute immediately if already execute in the main thread
		{
			action();
			return RC_OK;
		}

		std::lock_guard<std::mutex> lock(mMainThreadCallbacksQueueMutex);

		mMainThreadCallbacksQueue.emplace(action);

		return RC_OK;
	}

	void CBaseJobManager::ProcessMainThreadQueue()
	{
		if (mUpdateCounter < mUpdateTickRate)
		{
			++mUpdateCounter;
			return;
		}

		mUpdateCounter = 0;

		std::lock_guard<std::mutex> lock(mMainThreadCallbacksQueueMutex);

		while (!mMainThreadCallbacksQueue.empty())
		{
			mMainThreadCallbacksQueue.front()();
			mMainThreadCallbacksQueue.pop();
		}
	}
	
	E_ENGINE_SUBSYSTEM_TYPE CBaseJobManager::GetType() const
	{
		return EST_JOB_MANAGER;
	}

	void CBaseJobManager::_executeTasksLoop()
	{
		TJobDecl jobDecl;

		static thread_local tina WorkerFiber = TINA_EMPTY;
		WorkerFiber.name = "WorkerThread_Fiber";

#ifdef TDE2_USE_WINPLATFORM
		OPTICK_THREAD("Worker");
#endif

		while (true)
		{
			{
				std::unique_lock<std::mutex> lock(mQueueMutex);

				mHasNewJobAdded.wait(lock, [this] { return !mIsRunning || !mJobs.empty(); });

				if (!mIsRunning && mJobs.empty())
				{
					return;
				}

				jobDecl = std::move(mJobs.front());

				mJobs.pop();
			}

			{
				std::lock_guard<std::mutex> lock(mFreeFibersPoolMutex);
				if (!jobDecl.mpFiber && !mFreeFibersPool.empty())
				{
					jobDecl.mpFiber = mFreeFibersPool.front();
					mFreeFibersPool.pop();

					jobDecl.mpFiber->name = jobDecl.mpJobName;
					jobDecl.mpThreadFiber = &WorkerFiber;
					jobDecl.mIsCompleted = false;

#if TDE2_JOB_MANAGER_VERBOSE_LOG_ENABLED
					LOG_MESSAGE(Wrench::StringUtils::Format("[Job Manager] A new fiber is created, addr: {0}, id: {1}", jobDecl.mpFiber->_stack_pointer, jobDecl.mpJobName));
#endif
				}
			}

			TDE2_ASSERT(jobDecl.mpFiber);
			tina_swap(&WorkerFiber, jobDecl.mpFiber, reinterpret_cast<void*>(&jobDecl));

			if (!jobDecl.mIsCompleted)
			{
				continue;
			}

#if TDE2_JOB_MANAGER_VERBOSE_LOG_ENABLED
			LOG_MESSAGE(Wrench::StringUtils::Format("[Job Manager] The job {0} is completed", jobDecl.mpJobName));
#endif

			if (auto pCounter = jobDecl.mpCounter)
			{
				/// \note Push awaiting job back to the queue if that one exists
				{
					std::lock_guard<std::mutex> lock(pCounter->mWaitingJobListMutex);

					if (!pCounter->mpWaitingJobList.empty())
					{
						TJobDecl* pAwaitingJobDecl = &pCounter->mpWaitingJobList.front();

						if (pCounter->mValue - 1 <= pAwaitingJobDecl->mWaitingCounterThreshold)
						{
#if TDE2_JOB_MANAGER_VERBOSE_LOG_ENABLED
							LOG_MESSAGE(Wrench::StringUtils::Format("[Job Manager] The job {0} continues its execution", pAwaitingJobDecl->mpJobName));
#endif

							std::lock_guard<std::mutex> lock(mQueueMutex);
							mJobs.emplace(*pAwaitingJobDecl);

							mHasNewJobAdded.notify_one();

							/// \note Remove the current extracted job from the awaiting list
							pCounter->mpWaitingJobList.pop();
						}
					}
				}

				pCounter->mValue.fetch_sub(1);

#if 0
				if (pCounter->mpWaitingJobList)
				{
					TJobDecl* pAwaitingJobDecl = pCounter->mpWaitingJobList;

					if (pCounter->mValue <= pAwaitingJobDecl->mWaitingCounterThreshold)
					{
						/// \note Remove the current extracted job from the awaiting list
						while (!pCounter->mpWaitingJobList.compare_exchange_weak(pAwaitingJobDecl, pAwaitingJobDecl->mpNextAwaitingJob))
						{
						}

						pAwaitingJobDecl->mpNextAwaitingJob = nullptr;

#if TDE2_JOB_MANAGER_VERBOSE_LOG_ENABLED
						LOG_MESSAGE(Wrench::StringUtils::Format("[Job Manager] The job {0} continues its execution", pAwaitingJobDecl->mpJobName));
#endif

						std::lock_guard<std::mutex> lock(mQueueMutex);
						mJobs.emplace(*pAwaitingJobDecl);

						mHasNewJobAdded.notify_one();
					}
				}
#endif
			}

			{
				std::lock_guard<std::mutex> lock(mFreeFibersPoolMutex);
				mFreeFibersPool.push(jobDecl.mpFiber);
			}

			jobDecl.mpFiber = nullptr;
		}
	}


	IJobManager* CreateBaseJobManager(const TJobManagerInitParams& desc, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IJobManager, CBaseJobManager, result, desc);
	}
}