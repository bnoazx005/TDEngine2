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

	class CToggle : public CBaseComponent, public CPoolMemoryAllocPolicy<CToggle, 1 << 20>
	{
		public:
			friend TDE2_API IComponent* CreateToggle(E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CToggle)

			/*!
				\brief The method deserializes object's state from given reader

				\param[in, out] pReader An input stream of data that contains information about the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Load(IArchiveReader* pReader) override;

			/*!
				\brief The method serializes object's state into given stream

				\param[in, out] pWriter An output stream of data that writes information about the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Save(IArchiveWriter* pWriter) override;

			/*!
				\brief The method is called after all entities of particular scene were loaded. It remaps all identifiers to
				make them correctly corresponds to saved state

				\param[in, out] pEntityManager A pointer to entities manager
				\param[in] entitiesIdentifiersRemapper A structure that maps saved identifier to current runtime equivalent
			*/

			TDE2_API E_RESULT_CODE PostLoad(CEntityManager* pEntityManager, const TEntitiesMapper& entitiesIdentifiersRemapper) override;

			/*!
				\brief The method creates a new deep copy of the instance and returns a smart pointer to it.
				The original state of the object stays the same

				\param[in] pDestObject A valid pointer to an object which the properties will be assigned into
			*/

			TDE2_API E_RESULT_CODE Clone(IComponent*& pDestObject) const override;

			void SetState(bool state);
			void SetMarkerEntityId(TEntityId markerId);

			bool GetState() const;
			TEntityId GetMarkerEntityId() const;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CToggle)
		protected:
			TEntityId mMarkerEntityRef;
			bool mCurrState = true;
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