/*!
	\file CPerfProfiler.h
	\date 07.01.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "IProfiler.h"
#include "../core/CBaseObject.h"
#include "../utils/ITimer.h"
#include "../utils/CResult.h"
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

	class CPerfProfiler : public IProfiler, public CBaseObject
	{
		protected:
			typedef struct TSampleRecord
			{
				F32 mTime = 0.0f;

				U32 mThreadID = 0x0;
			} TSampleRecord, *TSampleRecordPtr;

			typedef std::stack<std::string>                        TScopesStackContext;
			typedef std::unordered_map<std::string, TSampleRecord> TSamplesTable;
		public:
			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method push a new scope into internal stack of tracked scopes to provide
				information about stack trace

				\param[in] scopeName A string identifier of pushed scope

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE PushScope(const std::string& scopeName) override;

			/*!
				\brief The method pops up current scope from the stack

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE PopScope() override;

			/*!
				\brief The method writes measurement's sample into profiler's table

				\param[in] time A elapsed time's value for this sample
				\param[in] threadID An identifier of a thread
			*/

			TDE2_API void WriteSample(F32 time, U32 threadID) override;

			/*!
				\brief The method returns a string with name of a parent scope

				\return The method returns a string with name of a parent scope, an empty string
				in case of the root scope when there is no any pushed scope yet
			*/

			TDE2_API const std::string& GetCurrParentScopeName() const override;

			/*!
				\brief The method returns instrumental timer that's used for measurements

				\return The method returns instrumental timer that's used for measurements
			*/

			TDE2_API ITimer* GetTimer() const override;

			/*!
				\brief The function is replacement of factory method for instances of this type.
				The only instance will be created per program's lifetime. To destroy it call Free
				as for any other type within the engine

				\return A pointer to an instance of IProfiler type
			*/

			TDE2_API static IProfiler* Get();
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CPerfProfiler);
		private:
			ITimer*             mpPerformanceTimer;
			
			TScopesStackContext mScopesContext;

			TSamplesTable       mSamplesTable;
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
				IProfiler* pProfiler = CPerfProfiler::Get();
				PANIC_ON_FAILURE(pProfiler->PushScope(name));

				ITimer* pTimer = pProfiler->GetTimer();
				pTimer->Tick();

				mStartTime = pTimer->GetCurrTime();
			}

			~CProfilerScope()
			{
				IProfiler* pProfiler = CPerfProfiler::Get();

				ITimer* pTimer = pProfiler->GetTimer();
				pTimer->Tick();

				mEndTime = pTimer->GetCurrTime();

				pProfiler->WriteSample(mEndTime - mStartTime, std::hash<std::thread::id>{}(mThreadID));
				PANIC_ON_FAILURE(pProfiler->PopScope());
			}
		private:
			std::string     mName;

			F32             mStartTime = 0.0f;
			F32             mEndTime = 0.0f;

			std::thread::id mThreadID = std::this_thread::get_id();
	};


#define TDE2_PROFILER_SCOPE(Name) CProfilerScope scope##__LINE__(#Name)
}