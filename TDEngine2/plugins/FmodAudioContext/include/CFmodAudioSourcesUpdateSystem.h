/*!
	\file CFMODSourcesUpdateSystem.h
	\date 09.01.2021
	\authors Kasimov Ildar
*/

#pragma once


#include <utils/Types.h>
#include <utils/Utils.h>
#include <ecs/CBaseSystem.h>
#include <unordered_map>


namespace FMOD
{
	class Channel;
}


namespace TDEngine2
{
	class IAudioContext;
	class IResourceManager;


	/*!
		\brief A factory function for creation objects of CAudioSourcesUpdateSystem's type.

		\param[in, out] pAudioContext A pointer to IAudioContext's implementation
		\param[in, out] pResourceManager A pointer to IResourceManager's implementation
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CAudioSourcesUpdateSystem's implementation
	*/

	TDE2_API ISystem* CreateAudioSourcesUpdateSystem(IAudioContext* pAudioContext, IResourceManager* pResourceManager, E_RESULT_CODE& result);


	/*!
		class CAudioSourcesUpdateSystem

		\brief The class is a system that processes IAudioSourcesUpdate components
	*/

	class CAudioSourcesUpdateSystem : public CBaseSystem
	{
		public:
			friend TDE2_API ISystem* CreateAudioSourcesUpdateSystem(IAudioContext*, IResourceManager*, E_RESULT_CODE&);
		public:
			TDE2_SYSTEM(CAudioSourcesUpdateSystem);

			/*!
				\brief The method initializes an inner state of a system

				\param[in, out] pAudioContext A pointer to IAudioContext's implementation
				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IAudioContext* pAudioContext, IResourceManager* pResourceManager);

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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CAudioSourcesUpdateSystem)

		protected:
			IAudioContext* mpAudioContext;
			IResourceManager* mpResourceManager;

			std::vector<TEntityId> mAudioSources;
			std::unordered_map<TEntityId, FMOD::Channel*> mpActiveChannels;
	};
}