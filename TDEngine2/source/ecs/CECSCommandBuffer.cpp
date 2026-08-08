#include "../../include/ecs/CECSCommandBuffer.h"
#include "../../include/ecs/IWorld.h"
#include "../../include/ecs/CEntityManager.h"


namespace TDEngine2
{
	CECSCommandBuffer::CECSCommandBuffer() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CECSCommandBuffer::Init()
	{
		mIsInitialized = true;
		return RC_OK;
	}

	TEntityId CECSCommandBuffer::AddEntity()
	{
		const TEntityId newEntityId = GetNextUniqueIdentifier();

		mCommands.emplace_back(TAddEntityWithUUIDCmd { newEntityId });
		return newEntityId;
	}

	E_RESULT_CODE CECSCommandBuffer::AddEntityWithUUID(TEntityId uuid)
	{
		mCommands.emplace_back(TAddEntityWithUUIDCmd{ uuid });
		return RC_OK;
	}

	TEntityId CECSCommandBuffer::AddEntityByName(const std::string& name)
	{
		const TEntityId newEntityId = GetNextUniqueIdentifier();

		mCommands.emplace_back(TAddEntityByNameCmd{ newEntityId, name });
		return newEntityId;
	}

	E_RESULT_CODE CECSCommandBuffer::DestroyEntity(TEntityId entityId)
	{
		mCommands.emplace_back(TDestroyEntityCmd{ entityId });
		return RC_OK;
	}

	E_RESULT_CODE CECSCommandBuffer::AddComponent(TEntityId entityId, TypeId componentTypeId)
	{
		mCommands.emplace_back(TAddComponentCmd{ entityId, componentTypeId });
		return RC_OK;
	}

	E_RESULT_CODE CECSCommandBuffer::RemoveComponent(TEntityId entityId, TypeId componentTypeId)
	{
		mCommands.emplace_back(TRemoveComponentCmd{ entityId, componentTypeId });
		return RC_OK;
	}

	E_RESULT_CODE CECSCommandBuffer::AddDelayedAction(const TCustomAction& action)
	{
		if (!action)
		{
			return RC_INVALID_ARGS;
		}

		mCommands.emplace_back(TCustomActionCmd{ action });
		return RC_OK;
	}

	E_RESULT_CODE CECSCommandBuffer::Flush(IWorld* pWorld)
	{
		if (!pWorld)
		{
			return RC_INVALID_ARGS;
		}

		// \todo Optimize commands
		// \todo Delete all pairs AddEntityWithUUID + DestroyEntity with id of previous command
		// \tpdp AddComponent + RemoveComponent on same entity for component of same type

		E_RESULT_CODE result = RC_OK;

		for (const TCommandBufferEntry& currCommandEntry : mCommands)
		{
			std::visit([pWorld, &result](const auto& arg) {
				using T = std::decay_t<decltype(arg)>;

				if constexpr (std::is_same_v<T, TAddEntityWithUUIDCmd>) {
					pWorld->CreateEntityWithUUID(arg.mUUID);
				}
				else if constexpr (std::is_same_v<T, TAddEntityByNameCmd>) {
					if (CEntity* pEntity = pWorld->CreateEntityWithUUID(arg.mEntityId))
					{
						pEntity->SetName(arg.mName);
					}
					else
					{
						result = result | RC_FAIL;
					}
				}
				else if constexpr (std::is_same_v<T, TDestroyEntityCmd>) {
					pWorld->Destroy(arg.mEntityId);
				}
				else if constexpr (std::is_same_v<T, TAddComponentCmd>) {
					if (CEntity* pEntity = pWorld->FindEntity(arg.mEntityId))
					{
						pEntity->AddComponent(arg.mComponentTypeId);
					}
					else
					{
						result = result | RC_FAIL;
					}
				}
				else if constexpr (std::is_same_v<T, TRemoveComponentCmd>) {
					if (CEntity* pEntity = pWorld->FindEntity(arg.mEntityId))
					{
						result = result | pEntity->RemoveComponent(arg.mComponentTypeId);
					}
					else
					{
						result = result | RC_FAIL;
					}
				}
				else if constexpr (std::is_same_v<T, TCustomActionCmd>) {
					arg.mAction();
				}
				}, currCommandEntry);
		}

		mCommands.clear();

		return result;
	}


	IECSCommandBuffer* CreateECSCommandBuffer(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IECSCommandBuffer, CECSCommandBuffer, result);
	}
}