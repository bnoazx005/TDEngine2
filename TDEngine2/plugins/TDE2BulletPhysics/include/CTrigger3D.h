/*!
	\file CTrigger3D.h
	\date 07.12.2020
	\authors Kasimov Ildar
*/

#pragma once


#include <core/CBaseObject.h>
#include <ecs/CBaseComponent.h>
#include <physics/3D/ITrigger3D.h>


namespace TDEngine2
{
	CLASS_META(SECTION = bullet_ecs_plugin, flags = SERIALIZE_MARKED_ONLY_FIELDS)
	struct TTrigger3DComponentData
	{
		TDE2_DECLARE_COMPONENT_META(TTrigger3DComponentData);
	};


	/*!
		\brief A factory function for creation objects of CTrigger3D's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CTrigger3D's implementation
	*/

	TDE2_API IComponent* CreateTrigger3D(E_RESULT_CODE& result);


	/*!
		class CTrigger3D

		\brief The class implements a functionality of a 3D trigger
	*/

	class CTrigger3D : public ITrigger3D, public CBaseComponentT<CTrigger3D, TTrigger3DComponentData>
	{
		public:
			friend TDE2_API IComponent* CreateTrigger3D(E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CTrigger3D)

			/*!
				\return The method returns type name (lowercase is preffered)
			*/

			TDE2_API const std::string& GetTypeName() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CTrigger3D)
	};


	/*!
		struct TTrigger3DParameters

		\brief The structure contains parameters for creation of CTrigger3D
	*/

	typedef struct TTrigger3DParameters : public TBaseComponentParameters
	{
	} TTrigger3DParameters;


	TDE2_DECLARE_COMPONENT_FACTORY(Trigger3D, TTrigger3DParameters);
}
