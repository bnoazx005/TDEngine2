/*!
	\file CFMODAudioContext.h
	\date 05.01.2021
	\authors Kasimov Ildar
*/
#pragma once


#include <core/IAudioContext.h>
#include <core/Event.h>
#include <math/TVector3.h>
#include <atomic>


namespace FMOD
{
	namespace Studio
	{
		class System;
	}

	class System;
}


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
				\brief The method updates an internal state of the context
				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Update() override;

			/*!
				\brief The method runs playback of the source
				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Play(IAudioSource* pAudioSource) override;

			TDE2_API E_RESULT_CODE SetListenerPosition(const TVector3& position) override;

			TDE2_API const TVector3& GetListenerPosition() const override;

			/*!
				\brief The method returns a type of the subsystem

				\return A type, which is represented with E_ENGINE_SUBSYSTEM_TYPE's value
			*/

			TDE2_API E_ENGINE_SUBSYSTEM_TYPE GetType() const override;

			TDE2_API FMOD::System* GetInternalContext() const;

		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CFMODAudioContext)

			TDE2_API E_RESULT_CODE _initInternalContext();
		protected:
			std::atomic_bool mIsInitialized;
			
			FMOD::Studio::System* mpSystem;
			FMOD::System* mpCoreSystem;

			TVector3 mCurrListenerPosition;
	};
}