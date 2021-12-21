/*!
	\file CBoundsComponent.h
	\date 28.04.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "../../core/memory/CPoolAllocator.h"
#include "../../ecs/CBaseComponent.h"
#include "../../math/TAABB.h"


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CBoundsComponent's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CBoundsComponent's implementation
	*/

	TDE2_API IComponent* CreateBoundsComponent(E_RESULT_CODE& result);


	/*!
		class CBoundsComponent

		\brief The class represents a component that stores information about object's boundaries
	*/

	class CBoundsComponent: public CBaseComponent, public CPoolMemoryAllocPolicy<CBoundsComponent, 1 << 20>
	{
		public:
			friend TDE2_API IComponent* CreateBoundsComponent(E_RESULT_CODE&);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CBoundsComponent)

			/*!
				\brief The method defines boundaries for the object based on AABB

				\param[in] aabbBounds An object that holds information about boundaries
			*/

			TDE2_API void SetBounds(const TAABB& aabbBounds);

			/*!
				\brief The method changes so called 'dirty' flag of the object that means that it was modified

				\param[in] value A new state of the flag, true means that the object has been changed
			*/

			TDE2_API void SetDirty(bool value);

			/*!
				\brief The method returns a boundaries of the object

				\return The method returns AABB object that contains boundaries of the object
			*/

			TDE2_API const TAABB& GetBounds() const;

			/*!
				\brief The method returns state of the object

				\return The method returns true if the object has been changed, but not been updated yet
			*/

			TDE2_API bool IsDirty() const;

			/*!
				\return The method returns type name (lowercase is preffered)
			*/

			TDE2_API const std::string& GetTypeName() const override;

			/*!
				\return The method returns a pointer to a type's property if the latter does exist or null pointer in other cases
			*/

			TDE2_API IPropertyWrapperPtr GetProperty(const std::string& propertyName) override;

			/*!
				\brief The method returns an array of properties names that are available for usage
			*/

			TDE2_API const std::vector<std::string>& GetAllProperties() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBoundsComponent)
		protected:
			TAABB mBounds;

			bool  mIsDirty;
	};


	/*!
		struct TBoundsComponentParameters

		\brief The structure contains parameters for creation of CBoundsComponent
	*/

	typedef struct TBoundsComponentParameters : public TBaseComponentParameters
	{
		TAABB mBounds;
	} TBoundsComponentParameters;


	TDE2_DECLARE_COMPONENT_FACTORY(BoundsComponent, TBoundsComponentParameters);
}