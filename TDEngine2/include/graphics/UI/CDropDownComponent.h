/*!
	\file CDropDownComponent.h
	\date 06.05.2023
	\authors Kasimov Ildar
*/

#pragma once


#include "../../ecs/CBaseComponent.h"
#include "../../ecs/CEntity.h"
#include <vector>
#include <string>


namespace TDEngine2
{
	CLASS_META(SECTION = ecs, flags = SERIALIZE_MARKED_ONLY_FIELDS)
	struct TDropDownComponentData
	{
		typedef std::vector<std::string> TOptionsArray;

		FIELD_META(name = items) TOptionsArray             mItems;

		FIELD_META(name = label_entity_ref) TEntityId      mLabelEntityRef = TEntityId::Invalid;
		FIELD_META(name = popup_root_entity_ref) TEntityId mPopupRootEntityRef = TEntityId::Invalid;
		FIELD_META(name = content_entity_ref) TEntityId    mContentEntityRef = TEntityId::Invalid;
		FIELD_META(name = item_prefab_ref) TEntityId       mItemPrefabEntityRef = TEntityId::Invalid;

		TEntitiesArray mItemsEntities;

		FIELD_META(name = selected_item) U32               mSelectedItemIndex = 0;

		bool                                               mIsExpanded = false;

		TDE2_DECLARE_COMPONENT_META(TDropDownComponentData);
	};


	/*!
		\brief A factory function for creation objects of CDropDown's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CDropDown's implementation
	*/

	TDE2_API IComponent* CreateDropDown(E_RESULT_CODE& result);


	/*!
		class CDropDown

		\brief The implementation of a combo box UI element
	*/

	class CDropDown : public CBaseComponentT<CDropDown, TDropDownComponentData>
	{
		public:
			friend TDE2_API IComponent* CreateDropDown(E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CDropDown)

			/*!
				\brief The method is called after all entities of particular scene were loaded. It remaps all identifiers to
				make them correctly corresponds to saved state

				\param[in, out] pEntityManager A pointer to entities manager
				\param[in] entitiesIdentifiersRemapper A structure that maps saved identifier to current runtime equivalent
			*/

			TDE2_API E_RESULT_CODE PostLoad(CEntityManager* pEntityManager, const TEntitiesMapper& entitiesIdentifiersRemapper) override;

			TDE2_API void SetLabelEntityId(TEntityId labelId);
			TDE2_API void SetPopupRootEntityId(TEntityId entityId);
			TDE2_API void SetContentEntityId(TEntityId contentEntityId);
			TDE2_API void SetItemPrefabEntityId(TEntityId entityId);

			TDE2_API E_RESULT_CODE SetSelectedItem(U32 index);
			TDE2_API void SetItems(const TDropDownComponentData::TOptionsArray& items);

			TDE2_API void SetExpanded(bool state);

			TDE2_API TEntityId GetLabelEntityId() const;
			TDE2_API TEntityId GetPopupRootEntityId() const;
			TDE2_API TEntityId GetContentEntityId() const;
			TDE2_API TEntityId GetItemPrefabEntityId() const;

			TDE2_API const TDropDownComponentData::TOptionsArray& GetItems() const;
			TDE2_API U32 GetSelectedItem() const;

			TDE2_API TEntitiesArray& GetItemsEntities();

			TDE2_API bool IsExpanded() const;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CDropDown)
		protected:
			TDE2_MULTI_THREAD_CHECK_LOCK;
	};


	/*!
		struct TDropDownParameters

		\brief The structure contains parameters for creation of CDropDown
	*/

	typedef struct TDropDownParameters : public TBaseComponentParameters
	{
	} TDropDownParameters;


	TDE2_DECLARE_COMPONENT_FACTORY(DropDown, TDropDownParameters);
}