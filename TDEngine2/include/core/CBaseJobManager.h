/*!
	\file CBaseJobManager.h
	\date 18.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "IJobManager.h"
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CResourceManager's type.

		\return A pointer to CResourceManager's implementation
	*/

	TDE2_API IJobManager* CreateBaseJobManager(U32 maxNumOfThreads, E_RESULT_CODE& result);


	/*!
		class CBaseJobManager

		\brief The class is a common implementation of a thread pool for
		Win32 and UNIX platforms
	*/

	class CBaseJobManager : public IJobManager
	{
		public:
			friend TDE2_API IJobManager* CreateBaseJobManager(U32 maxNumOfThreads, E_RESULT_CODE& result);
		protected:
			typedef std::vector<std::thread> TThreadsArray;
			typedef std::queue<IJob*>        TJobQueue;
		public:
			/*!
				\brief The method initializes an inner state of a resource manager

				\param[in] maxNumOfThreads A maximum number of threads that will be created and processed by the manager

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(U32 maxNumOfThreads) override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method pushes specified job into a queue for an execution

				\param[in] pJob A pointer to IJob's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE SubmitJob(IJob* pJob) override;

			
			/*!
				\brief The method returns a type of the subsystem

				\return A type, which is represented with E_ENGINE_SUBSYSTEM_TYPE's value
			*/

			TDE2_API E_ENGINE_SUBSYSTEM_TYPE GetType() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseJobManager)

			TDE2_API void _executeTasksLoop();
		protected:
			bool                    mIsInitialized;

			U32                     mNumOfThreads;

			std::atomic<bool>       mIsRunning;

			TThreadsArray           mWorkerThreads;

			mutable std::mutex      mQueueMutex;

			TJobQueue               mJobs;

			std::condition_variable mHasNewJobAdded;
	};
}
