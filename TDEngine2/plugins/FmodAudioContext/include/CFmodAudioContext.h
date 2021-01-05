/*!
	\file CFMODAudioContext.h
	\date 05.01.2021
	\authors Kasimov Ildar
*/
#pragma once


#include <core/IAudioContext.h>
#include <core/Event.h>
#include <atomic>


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CFMODAudioContext's type

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CFMODAudioContext's implementation
	*/

	TDE2_API IAudioContext* CreateFMODAudioContext(E_RESULT_CODE& result);


	class CFMODAudioContext : public IAudioContext
	{
		public:
			friend TDE2_API IAudioContext* CreateFMODAudioContext(E_RESULT_CODE&);
		public:
			TDE2_REGISTER_TYPE(CFMODAudioContext)

			/*!
				\brief The method initializes an initial state of the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init() override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method returns a type of the subsystem

				\return A type, which is represented with E_ENGINE_SUBSYSTEM_TYPE's value
			*/

			TDE2_API E_ENGINE_SUBSYSTEM_TYPE GetType() const override;

		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CFMODAudioContext)
		protected:
			std::atomic_bool mIsInitialized;
	};
}