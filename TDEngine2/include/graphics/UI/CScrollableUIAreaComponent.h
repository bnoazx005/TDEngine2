/*!
	\file CScrollableUIAreaComponent.h
	\date 28.04.2023
	\authors Kasimov Ildar
*/

#pragma once


#include "../../ecs/CBaseComponent.h"
#include "../../ecs/CEntity.h"


namespace TDEngine2
{
	CLASS_META(SECTION = ecs, flags = SERIALIZE_MARKED_ONLY_FIELDS)
	struct TScrollableUIAreaComponentData
	{
		FIELD_META(name = content_entity_ref) TEntityId   mContentEntityRef = TEntityId::Invalid;
		FIELD_META(name = content_entity_ref) F32         mScrollSpeedFactor = 25.0f;

		FIELD_META(name = normalized_scroll_pos) TVector2 mNormalizedScrollPosition = ZeroVector2;

		bool                                              mIsLayoutPrepared = false;
		FIELD_META(name = is_horizontal) bool             mIsHorizontal = true;
		FIELD_META(name = is_vertical) bool               mIsVertical = true;
		FIELD_META(name = enabled) bool                   mIsEnabled = true;

		TDE2_DECLARE_COMPONENT_META(TScrollableUIAreaComponentData);
	};


	/*!
		\brief A factory function for creation objects of CScrollableUIArea's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CScrollableUIArea's implementation
	*/

	TDE2_API IComponent* CreateScrollableUIArea(E_RESULT_CODE& result);


	/*!
		class CScrollableUIArea
	*/

	class CScrollableUIArea : public CBaseComponentT<CScrollableUIArea, TScrollableUIAreaComponentData>
	{
		public:
			friend TDE2_API IComponent* CreateScrollableUIArea(E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CScrollableUIArea)

			/*!
				\brief The method is called after all entities of particular scene were loaded. It remaps all identifiers to
				make them correctly corresponds to saved state

				\param[in, out] pEntityManager A pointer to entities manager
				\param[in] entitiesIdentifiersRemapper A structure that maps saved identifier to current runtime equivalent
			*/

			TDE2_API E_RESULT_CODE PostLoad(CEntityManager* pEntityManager, const TEntitiesMapper& entitiesIdentifiersRemapper) override;

			TDE2_API void SetContentEntityId(TEntityId cursorId);

			TDE2_API void SetScrollSpeedFactor(F32 value);

			TDE2_API void SetLayoutPrepared(bool value);
			TDE2_API void SetHorizontal(bool state);
			TDE2_API void SetVertical(bool state);
			
			TDE2_API void SetEnabled(bool value);

			TDE2_API void SetNormalizedScrollPosition(const TVector2& value);

			TDE2_API TEntityId GetContentEntityId() const;

			TDE2_API F32 GetScrollSpeedFactor() const;
			
			TDE2_API bool IsLayoutInitialized() const;
			TDE2_API bool IsHorizontal() const;
			TDE2_API bool IsVertical() const;

			TDE2_API bool IsEnabled() const;

			TDE2_API const TVector2& GetNormalizedScrollPosition() const;

		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CScrollableUIArea)
	};


	/*!
		struct TScrollableUIAreaParameters

		\brief The structure contains parameters for creation of CScrollableUIArea
	*/

	typedef struct TScrollableUIAreaParameters : public TBaseComponentParameters
	{
	} TScrollableUIAreaParameters;


	TDE2_DECLARE_COMPONENT_FACTORY(ScrollableUIArea, TScrollableUIAreaParameters);
}