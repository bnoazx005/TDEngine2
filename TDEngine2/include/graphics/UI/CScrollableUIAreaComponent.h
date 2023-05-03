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
	/*!
		\brief A factory function for creation objects of CScrollableUIArea's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CScrollableUIArea's implementation
	*/

	TDE2_API IComponent* CreateScrollableUIArea(E_RESULT_CODE& result);


	/*!
		class CScrollableUIArea
	*/

	class CScrollableUIArea : public CBaseComponent, public CPoolMemoryAllocPolicy<CScrollableUIArea, 1 << 20>
	{
		public:
			friend TDE2_API IComponent* CreateScrollableUIArea(E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CScrollableUIArea)

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

			TDE2_API void SetContentEntityId(TEntityId cursorId);

			TDE2_API void SetScrollSpeedFactor(F32 value);

			TDE2_API void SetLayoutPrepared(bool value);
			TDE2_API void SetHorizontal(bool state);
			TDE2_API void SetVertical(bool state);

			TDE2_API TEntityId GetContentEntityId() const;

			TDE2_API F32 GetScrollSpeedFactor() const;
			
			TDE2_API bool IsLayoutInitialized() const;
			TDE2_API bool IsHorizontal() const;
			TDE2_API bool IsVertical() const;

		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CScrollableUIArea)
		protected:
			TEntityId mContentEntityRef;
			F32 mScrollSpeedFactor = 100.0f;

			bool mIsLayoutPrepared = false;
			bool mIsHorizontal = true;
			bool mIsVertical = true;
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