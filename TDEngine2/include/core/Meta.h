/*!
	\file Meta.h
	\date 22.01.2021
	\authors Kasimov Ildar
*/

#pragma once


#include "CBaseObject.h"
#include <functional>
#include <string>
#include <vector>


namespace TDEngine2
{
	class IPropertyWrapper;


	/*!
		\brief All classes that want to provide some level of reflection for a user should implement this interface
	*/

	class IIntrospectable
	{
		public:
			/*!
				\return The method returns type name (lowercase is preffered)
			*/

			TDE2_API virtual const std::string& GetTypeName() const = 0;

			/*!
				\return The method returns a pointer to a type's property if the latter does exist or null pointer in other cases
			*/

			TDE2_API virtual TPtr<IPropertyWrapper> GetProperty(const std::string& propertyName) = 0;

			/*!
				\brief The method returns an array of properties names that are available for usage
			*/

			TDE2_API virtual const std::vector<std::string>& GetAllProperties() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IIntrospectable)
	};


	class IWorld;
	class CEntity;
	class CEntityManager;

	/*!
	*	
	*	\brief The method resolved given binding path looking downwards entity's hierarchy
	
		The format of input binding is 
		Entity_1 / ... / Entity_N.ComponentName.property

	* 
	*/ 

	TDE2_API TPtr<IPropertyWrapper> ResolveBinding(IWorld* pWorld, CEntity* pEntity, const std::string& path);
	TDE2_API TPtr<IPropertyWrapper> ResolveBinding(CEntityManager* pEntityManager, CEntity* pEntity, const std::string& path);
}