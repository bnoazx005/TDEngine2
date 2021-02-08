#include "./../../include/core/CBaseJobManager.h"
#include "./../../include/utils/CFileLogger.h"


namespace TDEngine2
{
	CBaseJobManager::CBaseJobManager():
		mIsInitialized(false)
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
	
	E_RESULT_CODE CBaseJobManager::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

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

		delete this;

		mIsInitialized = false;

		LOG_MESSAGE("[Job Manager] The job manager was successfully destroyed");


		return RC_OK;
	}

	E_RESULT_CODE CBaseJobManager::ExecuteInMainThread(const std::function<void()>& action)
	{
		if (!action)
		{
			return RC_INVALID_ARGS;
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
			(mMainThreadCallbacksQueue.front)();
			mMainThreadCallbacksQueue.pop();
		}
	}
	
	E_ENGINE_SUBSYSTEM_TYPE CBaseJobManager::GetType() const
	{
		return EST_JOB_MANAGER;
	}

	void CBaseJobManager::_executeTasksLoop()
	{
		std::unique_ptr<IJob> pJob;

		while (true)
		{
			{
				std::unique_lock<std::mutex> lock(mQueueMutex);

				mHasNewJobAdded.wait(lock, [this] { return !mIsRunning || !mJobs.empty(); });

				if (!mIsRunning && mJobs.empty())
				{
					return;
				}

				pJob = std::move(mJobs.front());

				mJobs.pop();
			}

			(*pJob)();
		}
	}

	E_RESULT_CODE CBaseJobManager::_submitJob(std::unique_ptr<IJob> pJob)
	{
		if (!pJob)
		{
			return RC_INVALID_ARGS;
		}

		std::lock_guard<std::mutex> lock(mQueueMutex);

		mJobs.emplace(std::move(pJob));

		mHasNewJobAdded.notify_one();

		return RC_OK;
	}


	IJobManager* CreateBaseJobManager(U32 maxNumOfThreads, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IJobManager, CBaseJobManager, result, maxNumOfThreads);
	}
}