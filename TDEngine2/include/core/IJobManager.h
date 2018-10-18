/*!
	\file IJobManager.h
	\date 18.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Types.h"
#include "IEngineSubsystem.h"
#include "./../utils/Utils.h"
#include <functional>


namespace TDEngine2
{
	/*!
		interface IJob

		\brief The interface represents a single job's functionality
	*/

	struct IJob
	{
		/*!
			\brief The operator executes current job
		*/

		TDE2_API virtual void operator()() = 0;
	};


	template <typename... TArgs>
	struct TJob: public IJob
	{
		protected:
			typedef std::function<void(TArgs...)> TJobCallback;
			typedef std::tuple<TArgs...>          TArguments;
		public:
			/*!
				\brief The main constructor of the type

				\param[in] callback A job's callback
				\param[in] args An arguments that should be passed into a job
			*/

			TDE2_API TJob(const TJobCallback& callback, TArgs&&... args);

			/*!
				\brief The operator executes current job
			*/

			TDE2_API void operator()() override;
		protected:
			TJobCallback mJobCallback;
			TArguments   mArguments;
	};

	
	template <typename... TArgs>
	TJob<TArgs...>::TJob(const TJobCallback& callback, TArgs&&... args) :
		mJobCallback(callback), mArguments(args...)
	{
	}

	template <typename... TArgs>
	void TJob<TArgs...>::operator() ()
	{
		mJobCallback(mArguments);
	}
	

	/*!
		interface IJobManager

		\brief The interface describes a functionality of a thread pool
	*/

	class IJobManager : public IEngineSubsystem
	{
		public:
			/*!
				\brief The method initializes an inner state of a resource manager

				\param[in] maxNumOfThreads A maximum number of threads that will be created and processed by the manager

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(U32 maxNumOfThreads) = 0;

			/*!
				\brief The method pushes specified job into a queue for an execution

				\param[in] pJob A pointer to IJob's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE SubmitJob(IJob* pJob) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IJobManager)
	};
}
