/*!
	\file CTrigger2D.h
	\date 18.11.2019
	\authors Kasimov Ildar
*/

#pragma once


#include "../../core/CBaseObject.h"
#include "../../ecs/CBaseComponent.h"
#include "ITrigger2D.h"


namespace TDEngine2
{
	CLASS_META(SECTION = ecs, flags = SERIALIZE_MARKED_ONLY_FIELDS)
	struct TTrigger2DComponentData
	{
		TDE2_DECLARE_COMPONENT_META(TTrigger2DComponentData);
	};


	/*!
		\brief A factory function for creation objects of CTrigger2D's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CTrigger2D's implementation
	*/

	TDE2_API IComponent* CreateTrigger2D(E_RESULT_CODE& result);


	/*!
		class CTrigger2D

		\brief The class implements a functionality of a 2D trigger
	*/

	class CTrigger2D: public ITrigger2D, public CBaseComponentT<CTrigger2D, TTrigger2DComponentData>
	{
		public:
			friend TDE2_API IComponent* CreateTrigger2D(E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CTrigger2D)

			/*!
				\return The method returns type name (lowercase is preffered)
			*/

			TDE2_API const std::string& GetTypeName() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CTrigger2D)
	};


	/*!
		struct TTrigger2DParameters
	*/

	typedef struct TTrigger2DParameters : public TBaseComponentParameters
	{
	} TTrigger2DParameters;


	TDE2_DECLARE_COMPONENT_FACTORY(Trigger2D, TTrigger2DParameters);
}
