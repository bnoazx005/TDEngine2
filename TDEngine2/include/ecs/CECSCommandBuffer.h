/*!
	\file CECSCommandBuffer.h
	\date 21.04.2026
	\authors Kasimov Ildar
*/

#pragma once


#include "IECSCommandBuffer.h"
#include "../core/CBaseObject.h"
#include "../utils/CContainers.h"


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CECSCommandBuffer's type.

		\return A pointer to IECSCommandBuffer's implementation
	*/

	TDE2_API IECSCommandBuffer* CreateECSCommandBuffer(E_RESULT_CODE& result);


	/*!
		\brief The implementation of commands buffer that stores deferred commands that should be executed at the end of ECS world's update phase
	*/

	class CECSCommandBuffer : public CBaseObject, public IECSCommandBuffer
	{
		public:
			friend TDE2_API IECSCommandBuffer* CreateECSCommandBuffer(E_RESULT_CODE&);
		private:
			struct TAddEntityWithUUIDCmd
			{
				TEntityId mUUID;
			};

			struct TAddEntityByNameCmd
			{
				TEntityId   mEntityId;
				std::string mName;
			};

			struct TDestroyEntityCmd
			{
				TEntityId mEntityId;
			};

			struct TAddComponentCmd
			{
				TEntityId mEntityId;
				TypeId    mComponentTypeId;
			};

			struct TRemoveComponentCmd
			{
				TEntityId mEntityId;
				TypeId    mComponentTypeId;
			};

			struct TCustomActionCmd
			{
				TCustomAction mAction = nullptr;
			};

			using TCommandBufferEntry = std::variant<TAddEntityWithUUIDCmd,
			                                         TAddEntityByNameCmd,
			                                         TDestroyEntityCmd,
			                                         TAddComponentCmd,
			                                         TRemoveComponentCmd,
			                                         TCustomActionCmd>;

			typedef CFixedVector<TCommandBufferEntry, 1024> TCommandsArray;
		public:
			TDE2_API E_RESULT_CODE Init() override;

			TDE2_API TEntityId AddEntity() override;
			TDE2_API TEntityId AddEntityByName(const std::string& name) override;
			TDE2_API E_RESULT_CODE AddEntityWithUUID(TEntityId uuid) override;

			TDE2_API E_RESULT_CODE DestroyEntity(TEntityId entityId) override;

			TDE2_API E_RESULT_CODE AddComponent(TEntityId entityId, TypeId componentTypeId) override;

			TDE2_API E_RESULT_CODE RemoveComponent(TEntityId entityId, TypeId componentTypeId) override;

			TDE2_API E_RESULT_CODE AddDelayedAction(const TCustomAction& action) override;

			TDE2_API E_RESULT_CODE Flush(IWorld* pWorld) override;
		private:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CECSCommandBuffer)
		private:
			TCommandsArray mCommands{};
	};
}