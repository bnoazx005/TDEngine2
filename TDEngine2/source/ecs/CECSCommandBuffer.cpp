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

		TCommandBufferEntry commandEntry{ E_ECS_COMMAND_BUFFER_ENTRY_TYPE::ADD_ENTITY };
		commandEntry.mArgs.mOpWithID.mValue = newEntityId;

		return newEntityId;
	}

	E_RESULT_CODE CECSCommandBuffer::AddEntityWithUUID(TEntityId uuid)
	{
		TCommandBufferEntry commandEntry{ E_ECS_COMMAND_BUFFER_ENTRY_TYPE::ADD_ENTITY_WITH_UUID };
		commandEntry.mArgs.mOpWithID.mValue = uuid;

		mCommands.emplace_back(commandEntry);

		return RC_OK;
	}

	TEntityId CECSCommandBuffer::AddEntityByName(const std::string& name)
	{
		const TEntityId newEntityId = GetNextUniqueIdentifier();

		TCommandBufferEntry commandEntry{ E_ECS_COMMAND_BUFFER_ENTRY_TYPE::ADD_ENTITY_BY_NAME };

		memcpy(static_cast<void*>(commandEntry.mArgs.mOpWithName.mName), name.data(), name.size());
		commandEntry.mArgs.mOpWithName.mNameLength = static_cast<U32>(name.length());
		commandEntry.mArgs.mOpWithName.mId         = newEntityId;

		mCommands.emplace_back(commandEntry);

		return newEntityId;
	}

	E_RESULT_CODE CECSCommandBuffer::DestroyEntity(TEntityId entityId)
	{
		TCommandBufferEntry commandEntry{ E_ECS_COMMAND_BUFFER_ENTRY_TYPE::DESTROY_ENTITY };
		commandEntry.mArgs.mOpWithID.mValue = entityId;

		mCommands.emplace_back(commandEntry);

		return RC_OK;
	}

	E_RESULT_CODE CECSCommandBuffer::AddComponent(TEntityId entityId, TypeId componentTypeId)
	{
		TCommandBufferEntry commandEntry{ E_ECS_COMMAND_BUFFER_ENTRY_TYPE::ADD_COMPONENT };
		commandEntry.mArgs.mComponentOpWithID.mValue           = entityId;
		commandEntry.mArgs.mComponentOpWithID.mComponentTypeId = componentTypeId;

		mCommands.emplace_back(commandEntry);

		return RC_OK;
	}

	E_RESULT_CODE CECSCommandBuffer::RemoveComponent(TEntityId entityId, TypeId componentTypeId)
	{
		TCommandBufferEntry commandEntry{ E_ECS_COMMAND_BUFFER_ENTRY_TYPE::REMOVE_COMPONENT };
		commandEntry.mArgs.mComponentOpWithID.mValue           = entityId;
		commandEntry.mArgs.mComponentOpWithID.mComponentTypeId = componentTypeId;

		mCommands.emplace_back(commandEntry);

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
			switch (currCommandEntry.mType)
			{
				case E_ECS_COMMAND_BUFFER_ENTRY_TYPE::ADD_ENTITY:
				case E_ECS_COMMAND_BUFFER_ENTRY_TYPE::ADD_ENTITY_WITH_UUID:
					pWorld->CreateEntityWithUUID(currCommandEntry.mArgs.mOpWithID.mValue);
					break;

				case E_ECS_COMMAND_BUFFER_ENTRY_TYPE::ADD_ENTITY_BY_NAME:
					if (CEntity* pEntity = pWorld->CreateEntityWithUUID(currCommandEntry.mArgs.mOpWithName.mId))
					{
						pEntity->SetName(std::string(currCommandEntry.mArgs.mOpWithName.mNameLength, currCommandEntry.mArgs.mOpWithName.mNameLength));
					}
					else
					{
						result = result | RC_FAIL;
					}
					break;

				case E_ECS_COMMAND_BUFFER_ENTRY_TYPE::DESTROY_ENTITY:
					result = result | pWorld->Destroy(currCommandEntry.mArgs.mOpWithID.mValue);
					break;

				case E_ECS_COMMAND_BUFFER_ENTRY_TYPE::ADD_COMPONENT:
					if (CEntity* pEntity = pWorld->FindEntity(currCommandEntry.mArgs.mComponentOpWithID.mValue))
					{
						pEntity->AddComponent(currCommandEntry.mArgs.mComponentOpWithID.mComponentTypeId);
					}
					else
					{
						result = result | RC_FAIL;
					}
					break;

				case E_ECS_COMMAND_BUFFER_ENTRY_TYPE::REMOVE_COMPONENT:
					if (CEntity* pEntity = pWorld->FindEntity(currCommandEntry.mArgs.mComponentOpWithID.mValue))
					{
						result = result | pEntity->RemoveComponent(currCommandEntry.mArgs.mComponentOpWithID.mComponentTypeId);
					}
					else
					{
						result = result | RC_FAIL;
					}
					break;

				default:
					TDE2_UNREACHABLE();
					break;
			}
		}

		mCommands.clear();

		return result;
	}


	IECSCommandBuffer* CreateECSCommandBuffer(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IECSCommandBuffer, CECSCommandBuffer, result);
	}
}