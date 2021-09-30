/*!
	\file CBaseObject.h
	\date 28.09.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "IBaseObject.h"
#include <atomic>


namespace TDEngine2
{
	/*!
		class CBaseObject
		
		\brief The class implements a base object for the engine.
	*/

	class CBaseObject: public virtual IBaseObject
	{
		public:
			/*!
				\brief The method increment a value of the internal reference counter. You can ignore
				this method if you work in single-threaded manner. If you want to use an object from
				another thread you should increment its counter to prevent non thread-safe deletion by
				some of running threads.
			*/

			TDE2_API void AddRef() override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() final override;

			/*!
				\return The method returns a number of strong references that points to the object
			*/

			TDE2_API U32 GetRefCount() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseObject)

			TDE2_API virtual E_RESULT_CODE _onFreeInternal();
		protected:
			std::atomic_uint32_t mRefCounter;

			std::atomic_bool     mIsInitialized;
	};
}