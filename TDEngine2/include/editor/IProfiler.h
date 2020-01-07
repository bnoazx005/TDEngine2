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
			/*!
				\brief The method push a new scope into internal stack of tracked scopes to provide
				information about stack trace

				\param[in] scopeName A string identifier of pushed scope

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE PushScope(const std::string& scopeName) = 0;

			/*!
				\brief The method pops up current scope from the stack

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE PopScope() = 0;

			/*!
				\brief The method writes measurement's sample into profiler's table

				\param[in] time A elapsed time's value for this sample
				\param[in] threadID An identifier of a thread
			*/

			TDE2_API virtual void WriteSample(F32 time, U32 threadID) = 0;

			/*!
				\brief The method returns a string with name of a parent scope

				\return The method returns a string with name of a parent scope, an empty string
				in case of the root scope when there is no any pushed scope yet
			*/

			TDE2_API virtual const std::string& GetCurrParentScopeName() const = 0;

			/*!
				\brief The method returns instrumental timer that's used for measurements

				\return The method returns instrumental timer that's used for measurements
			*/

			TDE2_API virtual ITimer* GetTimer() const = 0;

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