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
	class CEntityManager;

	struct TEntitiesMapper;


	/*!
		interface IComponent

		\brief The interface describes a functionality of a component

		\note The interface is empty by now
	*/

	class IComponent: public virtual IBaseObject, public ISerializable, public IIntrospectable, public INonAllocCloneable<IComponent>
	{
		public:
			/*!
				\brief The method initializes an internal state of an object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init() = 0;

			/*!
				\brief The method is called after all entities of particular scene were loaded. It remaps all identifiers to
				make them correctly corresponds to saved state

				\param[in, out] pEntityManager A pointer to entities manager
				\param[in] entitiesIdentifiersRemapper A structure that maps saved identifier to current runtime equivalent
			*/

			TDE2_API virtual E_RESULT_CODE PostLoad(CEntityManager* pEntityManager, const TEntitiesMapper& entitiesIdentifiersRemapper) = 0;

			TDE2_API virtual TypeId GetComponentTypeId() const = 0;

			/*!
				\return The method returns true if the given component type is for runtime purposes only
			*/

			TDE2_API virtual bool IsRuntimeOnly() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IComponent)
	};
}