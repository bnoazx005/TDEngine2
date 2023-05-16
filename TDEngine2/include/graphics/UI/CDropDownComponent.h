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

	class CDropDown : public CBaseComponent, public CPoolMemoryAllocPolicy<CDropDown, 1 << 20>
	{
		public:
			friend TDE2_API IComponent* CreateDropDown(E_RESULT_CODE& result);
		public:
			typedef std::vector<std::string> TOptionsArray;
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CDropDown)

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

			TDE2_API void SetLabelEntityId(TEntityId labelId);
			TDE2_API void SetPopupRootEntityId(TEntityId entityId);
			TDE2_API void SetContentEntityId(TEntityId contentEntityId);
			TDE2_API void SetItemPrefabEntityId(TEntityId entityId);

			TDE2_API E_RESULT_CODE SetSelectedItem(U32 index);
			TDE2_API void SetItems(const TOptionsArray& items);

			TDE2_API void SetExpanded(bool state);

			TDE2_API TEntityId GetLabelEntityId() const;
			TDE2_API TEntityId GetPopupRootEntityId() const;
			TDE2_API TEntityId GetContentEntityId() const;
			TDE2_API TEntityId GetItemPrefabEntityId() const;

			TDE2_API const TOptionsArray& GetItems() const;
			TDE2_API U32 GetSelectedItem() const;

			TDE2_API std::vector<TEntityId>& GetItemsEntities();

			TDE2_API bool IsExpanded() const;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CDropDown)
		protected:
			TOptionsArray mItems;

			TEntityId     mLabelEntityRef;
			TEntityId     mPopupRootEntityRef;
			TEntityId     mContentEntityRef;
			TEntityId     mItemPrefabEntityRef;
			
			std::vector<TEntityId> mItemsEntities;

			U32           mSelectedItemIndex = 0;

			bool          mIsExpanded = false;
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