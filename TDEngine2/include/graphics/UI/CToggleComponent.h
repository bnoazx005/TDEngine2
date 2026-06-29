/*!
	\file CToggleComponent.h
	\date 15.03.2023
	\authors Kasimov Ildar
*/

#pragma once


#include "../../ecs/CBaseComponent.h"
#include "../../ecs/CEntity.h"


namespace TDEngine2
{
	CLASS_META(SECTION = ecs, flags = SERIALIZE_MARKED_ONLY_FIELDS)
	struct TToggleComponentData
	{
		FIELD_META(name = marker_entity_ref) TEntityId mMarkerEntityRef = TEntityId::Invalid;
		FIELD_META(name = state) bool                  mCurrState = true;

		TDE2_DECLARE_COMPONENT_META(TToggleComponentData);
	};

	/*!
		\brief A factory function for creation objects of CToggle's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CToggle's implementation
	*/

	TDE2_API IComponent* CreateToggle(E_RESULT_CODE& result);


	/*!
		class CToggle

		\brief The implementation of on/off switching UI element
	*/

	class CToggle : public CBaseComponentT<CToggle, TToggleComponentData>
	{
		public:
			friend TDE2_API IComponent* CreateToggle(E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CToggle)

			/*!
				\brief The method is called after all entities of particular scene were loaded. It remaps all identifiers to
				make them correctly corresponds to saved state

				\param[in, out] pEntityManager A pointer to entities manager
				\param[in] entitiesIdentifiersRemapper A structure that maps saved identifier to current runtime equivalent
			*/

			TDE2_API E_RESULT_CODE PostLoad(CEntityManager* pEntityManager, const TEntitiesMapper& entitiesIdentifiersRemapper) override;

			TDE2_API void SetState(bool state);
			TDE2_API void SetMarkerEntityId(TEntityId markerId);

			TDE2_API bool GetState() const;
			TDE2_API TEntityId GetMarkerEntityId() const;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CToggle)
	};


	/*!
		struct TToggleParameters

		\brief The structure contains parameters for creation of CToggle
	*/

	typedef struct TToggleParameters : public TBaseComponentParameters
	{
	} TToggleParameters;


	TDE2_DECLARE_COMPONENT_FACTORY(Toggle, TToggleParameters);
}