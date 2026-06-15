/*!
	\file CObjIdComponent.h
	\date 22.02.2023
	\authors Kasimov Ildar
*/

#pragma once


#include "../../ecs/CBaseComponent.h"
#include <string>


namespace TDEngine2
{
	CLASS_META(SECTION = ecs, flags = SERIALIZE_MARKED_ONLY_FIELDS)
	struct TObjIdComponentData
	{
		FIELD_META(name = obj_id) U32 mId = 0;

		TDE2_DECLARE_COMPONENT_META(TObjIdComponentData);
	};


	/*!
		struct TObjIdComponentParameters
	*/

	typedef struct TObjIdComponentParameters : public TBaseComponentParameters
	{
	} TObjIdComponentParameters;


	/*!
		\brief A factory function for creation objects of CObjIdComponent's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CObjIdComponent's implementation
	*/

	TDE2_API IComponent* CreateObjIdComponent(E_RESULT_CODE& result);


	/*!
		class CObjIdComponent

		The class is used to resolve references to entities that're part of prefabs in runtime
	*/

	class CObjIdComponent : public CBaseComponentT<CObjIdComponent, TObjIdComponentData>
	{
		public:
			friend TDE2_API IComponent* CreateObjIdComponent(E_RESULT_CODE&);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CObjIdComponent)

			/*!
				\return The method returns true if the given component type is for runtime purposes only
			*/

			TDE2_API bool IsRuntimeOnly() const override;

			/*!
				\return The method returns type name (lowercase is preffered)
			*/

			TDE2_API const std::string& GetTypeName() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CObjIdComponent)
	};


	TDE2_DECLARE_COMPONENT_FACTORY(ObjIdComponent, TObjIdComponentParameters);
}