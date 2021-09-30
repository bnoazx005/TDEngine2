/*!
	\file CFMODListenerUpdateSystem.h
	\date 07.01.2021
	\authors Kasimov Ildar
*/

#pragma once


#include <utils/Types.h>
#include <utils/Utils.h>
#include <ecs/CBaseSystem.h>


namespace TDEngine2
{
	class IAudioContext;


	/*!
		\brief A factory function for creation objects of CAudioListenerUpdateSystem's type.

		\param[in, out] pAudioContext A pointer to IAudioContext's implementation
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CAudioListenerUpdateSystem's implementation
	*/

	TDE2_API ISystem* CreateAudioListenerUpdateSystem(IAudioContext* pAudioContext, E_RESULT_CODE& result);


	/*!
		class CAudioListenerUpdateSystem

		\brief The class is a system that processes IAudioListenerUpdate components
	*/

	class CAudioListenerUpdateSystem : public CBaseSystem
	{
		public:
			friend TDE2_API ISystem* CreateAudioListenerUpdateSystem(IAudioContext*, E_RESULT_CODE&);
		public:
			TDE2_SYSTEM(CAudioListenerUpdateSystem);

			/*!
				\brief The method initializes an inner state of a system

				\param[in, out] pAudioContext A pointer to IAudioContext's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IAudioContext* pAudioContext);

			/*!
				\brief The method inject components array into a system

				\param[in] pWorld A pointer to a main scene's object
			*/

			TDE2_API void InjectBindings(IWorld* pWorld) override;

			/*!
				\brief The main method that should be implemented in all derived classes.
				It contains all the logic that the system will execute during engine's work.

				\param[in] pWorld A pointer to a main scene's object

				\param[in] dt A delta time's value
			*/

			TDE2_API void Update(IWorld* pWorld, F32 dt) override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CAudioListenerUpdateSystem)

		protected:
			IAudioContext* mpAudioContext;

			TEntityId mMainListenerEntityId;
	};
}