/*!
	\file CTestContext.h
	\date 03.09.2022
	\authors Kasimov Ildar
*/

#pragma once


#include "../core/CBaseObject.h"


namespace TDEngine2
{
	/*!
		class CTestContext

		\brief The type is a core point for all autotesting system. The API is used to define test fixtures and
		to run their execution
	*/

	class CTestContext: public CBaseObject
	{
		public:
			/*!
				\brief The function is replacement of factory method for instances of this type.
				The only instance will be created per program's lifetime.

				\return A pointer to an instance of IProfiler type
			*/

			TDE2_API static TPtr<CTestContext> Get();
		private:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CTestContext);
		private:

	};
}