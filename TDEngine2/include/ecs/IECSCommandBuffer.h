/*!
	\file IECSCommandBuffer.h
	\date 21.04.2026
	\authors Kasimov Ildar
*/

#pragma once


#include "../core/IBaseObject.h"
#include "../utils/Types.h"


namespace TDEngine2
{
	class IWorld;
	class IComponent;


	enum class TEntityId : U32;


	/*!
		interface IECSCommandBuffer

		\brief The interface represents a functionality of an array of commands that are applied to ECS world's instance at the end of Update phase
	*/

	class IECSCommandBuffer : public virtual IBaseObject
	{
		public:
			TDE2_API virtual E_RESULT_CODE Init() = 0;

			TDE2_API virtual TEntityId AddEntity() = 0;
			TDE2_API virtual TEntityId AddEntityByName(const std::string& name) = 0;
			TDE2_API virtual E_RESULT_CODE AddEntityWithUUID(TEntityId uuid) = 0;

			TDE2_API virtual E_RESULT_CODE DestroyEntity(TEntityId entityId) = 0;

			TDE2_API virtual E_RESULT_CODE AddComponent(TEntityId entityId, TypeId componentTypeId) = 0;

			template <typename T>
			std::enable_if_t<std::is_base_of_v<IComponent, T>, E_RESULT_CODE> RemoveComponent(TEntityId entityId)
			{
				return RemoveComponent(entityId, T::GetTypeId());
			}

			TDE2_API virtual E_RESULT_CODE RemoveComponent(TEntityId entityId, TypeId componentTypeId) = 0;

			TDE2_API virtual E_RESULT_CODE Flush(IWorld* pWorld) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IECSCommandBuffer)
	};


	TDE2_DECLARE_SCOPED_PTR_INLINED(IECSCommandBuffer)
}