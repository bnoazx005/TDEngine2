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
			enum class E_ECS_COMMAND_BUFFER_ENTRY_TYPE : U8
			{
				ADD_ENTITY           = 0x1,
				ADD_ENTITY_WITH_UUID = 0x2,
				ADD_ENTITY_BY_NAME   = 0x3,
				DESTROY_ENTITY       = 0x4,
				ADD_COMPONENT        = 0x5,
				REMOVE_COMPONENT     = 0x6,
				UNKNOWN = 0xFF
			};

			struct TCommandBufferEntry
			{
				E_ECS_COMMAND_BUFFER_ENTRY_TYPE mType = E_ECS_COMMAND_BUFFER_ENTRY_TYPE::UNKNOWN;

				union
				{
					struct { TEntityId mValue = TEntityId::Invalid; } mOpWithID;

					struct 
					{ 
						C8        mName[255];
						U32       mNameLength = 0;
						TEntityId mId = TEntityId::Invalid; 
					} mOpWithName;

					struct 
					{
						TEntityId mValue = TEntityId::Invalid;
						TypeId    mComponentTypeId = TypeId::Invalid;
					} mComponentOpWithID;
				} mArgs;
			};

			typedef CFixedVector<TCommandBufferEntry, 1024> TCommandsArray;

			static_assert(std::is_trivially_copyable_v<TCommandBufferEntry>, "TCommandBufferEntry must be trivially copyable");
		public:
			TDE2_API E_RESULT_CODE Init() override;

			TDE2_API TEntityId AddEntity() override;
			TDE2_API TEntityId AddEntityByName(const std::string& name) override;
			TDE2_API E_RESULT_CODE AddEntityWithUUID(TEntityId uuid) override;

			TDE2_API E_RESULT_CODE DestroyEntity(TEntityId entityId) override;

			TDE2_API E_RESULT_CODE AddComponent(TEntityId entityId, TypeId componentTypeId) override;

			TDE2_API E_RESULT_CODE RemoveComponent(TEntityId entityId, TypeId componentTypeId) override;

			TDE2_API E_RESULT_CODE Flush(IWorld* pWorld) override;
		private:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CECSCommandBuffer)
		private:
			TCommandsArray mCommands{};
	};
}