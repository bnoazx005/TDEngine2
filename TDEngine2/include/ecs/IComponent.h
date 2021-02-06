/*!
	\file IComponent.h
	\date 28.09.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "../core/CBaseObject.h"
#include "../core/Serialization.h"
#include "../core/Meta.h"
#include "../utils/Utils.h"
#include <unordered_map>


namespace TDEngine2
{
	class IWorld;


	/*!
		interface IComponent

		\brief The interface describes a functionality of a component

		\note The interface is empty by now
	*/

	class IComponent: public virtual IBaseObject, public ISerializable, public IIntrospectable
	{
		public:
			typedef std::unordered_map<TEntityId, TEntityId> TEntitiesMapper;
		public:
			/*!
				\brief The method is called after all entities of particular scene were loaded. It remaps all identifiers to
				make them correctly corresponds to saved state

				\param[in, out] pWorld A pointer to game world
				\param[in] entitiesIdentifiersRemapper A structure that maps saved identifier to current runtime equivalent
			*/

			TDE2_API virtual E_RESULT_CODE PostLoad(IWorld* pWorld, const TEntitiesMapper& entitiesIdentifiersRemapper) = 0;

			TDE2_API virtual TypeId GetComponentTypeId() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IComponent)
	};
}