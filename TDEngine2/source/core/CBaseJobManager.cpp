#include "./../../include/core/CBaseJobManager.h"
#include "./../../include/utils/CFileLogger.h"
#include <cmath>


namespace TDEngine2
{
	CBaseJobManager::CBaseJobManager():
		CBaseObject()
	{
	}

	E_RESULT_CODE CBaseJobManager::Init(U32 maxNumOfThreads)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}
		
		mNumOfThreads = maxNumOfThreads;

		mIsRunning = true;

		for (U32 i = 0; i < mNumOfThreads; ++i)
		{
			mWorkerThreads.emplace_back(&CBaseJobManager::_executeTasksLoop, this);
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

	E_RESULT_CODE CBaseJobManager::SubmitJob(TJobCounter* pCounter, const TJobCallback& job)
	{
		if (!job)
		{
			return RC_INVALID_ARGS;
		}

		TJobDecl jobDecl;
		jobDecl.mJob = job;
		jobDecl.mpCounter = pCounter;

		if (pCounter)
		{
			pCounter->mValue.fetch_add(1);
		}

		std::lock_guard<std::mutex> lock(mQueueMutex);

		mJobs.emplace(std::move(jobDecl));

		mHasNewJobAdded.notify_one();

		return RC_OK;
	}

	E_RESULT_CODE CBaseJobManager::SubmitMultipleJobs(TJobCounter* pCounter, U32 jobsCount, U32 groupSize, const TJobCallback& job)
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

				mJobs.emplace(std::move(jobDecl));
			}			
		}

		mHasNewJobAdded.notify_all();

		return RC_OK;
	}

	void CBaseJobManager::WaitForJobCounter(const TJobCounter& counter, U32 counterThreshold)
	{
		if (counter.mValue <= counterThreshold)
		{
			return;
		}

		mHasNewJobAdded.notify_all();

		while (counter.mValue > counterThreshold)
		{
			std::this_thread::yield();
		}
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

			TJobArgs args;
			args.mJobIndex = jobDecl.mJobIndex;
			args.mGroupIndex = jobDecl.mGroupIndex;

			(jobDecl.mJob)(args);

			if (auto pCounter = jobDecl.mpCounter)
			{
				pCounter->mValue.fetch_sub(1);
			}
		}
	}


	IJobManager* CreateBaseJobManager(U32 maxNumOfThreads, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IJobManager, CBaseJobManager, result, maxNumOfThreads);
	}
}