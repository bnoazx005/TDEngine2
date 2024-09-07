#include "./../../include/core/CBaseJobManager.h"
#include "./../../include/utils/CFileLogger.h"
#include "stringUtils.hpp"
#include <cmath>
#include "marl/scheduler.h"
#include "marl/waitgroup.h"
#include "marl/blockingcall.h"

#ifdef TDE2_USE_WINPLATFORM
#include "optick.h"
#endif

#include "tracy/Tracy.hpp"


#define TDE2_JOB_MANAGER_VERBOSE_LOG_ENABLED 0


namespace TDEngine2
{
	CBaseJobManager::CBaseJobManager():
		CBaseObject()
	{
	}

	CBaseJobManager::~CBaseJobManager()
	{
	}

	E_RESULT_CODE CBaseJobManager::Init(const TJobManagerInitParams& desc)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (desc.mMaxNumOfThreads < 1)
		{
			return RC_INVALID_ARGS;
		}

		mConfig = desc;
		mUpdateCounter = 0;

		mNextFreeCounterIndex.store(0);

		mIsInitialized = true;

		LOG_MESSAGE("[Job Manager] The job manager was successfully initialized...");
		LOG_MESSAGE("[Job Manager] " + std::to_string(desc.mMaxNumOfThreads) + " worker threads were created");

		return RC_OK;
	}

	E_RESULT_CODE CBaseJobManager::StartWorkerThreads()
	{
		marl::Scheduler::Config cfg;

		cfg.setWorkerThreadCount(mConfig.mMaxNumOfThreads);
		cfg.setFiberStackSize(mConfig.mFiberStackSize);
		cfg.setWorkerThreadInitializer([this](int workerId)
			{
				auto&& threadName = Wrench::StringUtils::Format("WorkerThread_{0}", workerId);
#ifdef TDE2_USE_WINPLATFORM
				OPTICK_START_THREAD(threadName.c_str());
#endif
				tracy::SetThreadName(threadName.c_str());

				if (mConfig.mInitWorkerThreadCallback)
				{
					mConfig.mInitWorkerThreadCallback();
				}
			});

		mpScheduler = std::make_unique<marl::Scheduler>(cfg);
		if (!mpScheduler)
		{
			return RC_FAIL;
		}

		mpScheduler->bind();

		for (U32 i = 0; i < mConfig.mCountersPoolSize; i++)
		{
			mpWaitCountersPool.emplace_back(std::make_unique<marl::WaitGroup>());
		}

		return RC_OK;
	}
	
	E_RESULT_CODE CBaseJobManager::_onFreeInternal()
	{
		mpScheduler->unbind();

		LOG_MESSAGE("[Job Manager] The job manager was successfully destroyed");

		return RC_OK;
	}


	static const auto MainThreadId = std::this_thread::get_id();


	bool CBaseJobManager::IsMainThread()
	{
		return MainThreadId == std::this_thread::get_id();
	}

	E_RESULT_CODE CBaseJobManager::SubmitJob(TJobCounter* pCounter, const TJobCallback& job, const TSubmitJobParams& params)
	{
		if (!job)
		{
			return RC_INVALID_ARGS;
		}

		const USIZE waitGroupIndex = mNextFreeCounterIndex.load() % (mpWaitCountersPool.size() - 1);
		if (pCounter)
		{
			pCounter->store(static_cast<TJobCounterId>(waitGroupIndex));
		}

		auto pWaitGroup = mpWaitCountersPool[waitGroupIndex].get();
		pWaitGroup->add(1);

		mNextFreeCounterIndex.fetch_add(1);

		TJobArgs args;
		args.mJobIndex = 0;
		args.mGroupIndex = 0;

		auto task = [=]
		{
			job(args);
			pWaitGroup->done();
		};

		if (params.mBlockingCallsAwaited)
		{
			marl::blocking_call(task);
		}
		else
		{
			marl::schedule(task);
		}

#if TDE2_JOB_MANAGER_VERBOSE_LOG_ENABLED
		LOG_MESSAGE(Wrench::StringUtils::Format("[Job Manager] Submit a new job, id: {0}", jobName));
#endif

		return RC_OK;
	}

	E_RESULT_CODE CBaseJobManager::SubmitMultipleJobs(TJobCounter* pCounter, U32 jobsCount, U32 groupSize, const TJobCallback& job, E_JOB_PRIORITY_TYPE priority)
	{
		if (!job || !groupSize || groupSize > jobsCount)
		{
			return RC_INVALID_ARGS;
		}

		const U32 groupsCount = static_cast<U32>(::ceilf(jobsCount / static_cast<F32>(groupSize)));

		const USIZE waitGroupIndex = mNextFreeCounterIndex.load() % (mpWaitCountersPool.size() - 1);
		if (pCounter)
		{
			pCounter->store(static_cast<TJobCounterId>(waitGroupIndex));
		}

		auto pWaitGroup = mpWaitCountersPool[waitGroupIndex].get();
		pWaitGroup->add(jobsCount);

		mNextFreeCounterIndex.fetch_add(1);

		for (U32 groupId = 0; groupId < groupsCount; groupId++)
		{
			marl::schedule([=]()
			{
				TJobArgs args;

				for (U32 jobIndex = groupId * groupSize; jobIndex < (groupId + 1) * groupSize && jobIndex < jobsCount; jobIndex++)
				{
					args.mJobIndex = jobIndex;
					args.mGroupIndex = groupId;

					job(args);	
					pWaitGroup->done();
				}
			});
		}

#if TDE2_JOB_MANAGER_VERBOSE_LOG_ENABLED
		LOG_MESSAGE(Wrench::StringUtils::Format("[Job Manager] Submit new jobs, count: {0}; groups_count: {1}", jobsCount, groupsCount));
#endif

		return RC_OK;
	}

	void CBaseJobManager::WaitForJobCounter(TJobCounter& counter)
	{
		if (auto pWaitGroup = mpWaitCountersPool[static_cast<USIZE>(counter.load())].get())
		{
			pWaitGroup->wait();
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


	IJobManager* CreateBaseJobManager(const TJobManagerInitParams& desc, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IJobManager, CBaseJobManager, result, desc);
	}
}