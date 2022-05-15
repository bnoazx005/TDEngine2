/*!
	\file CTransform.h
	\date 29.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "CBaseComponent.h"
#include "ITransform.h"
#include "IComponentFactory.h"
#include "../core/memory/CPoolAllocator.h"
#include "../math/TMatrix4.h"


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CTransform's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to IComponent's implementation
	*/

	TDE2_API IComponent* CreateTransform(E_RESULT_CODE& result);


	/*!
		class CTransform

		\brief The class is a basic component of all entities
		It stores all transformations that were applied to
		an object
	*/

	class CTransform: public ITransform, public CBaseComponent, public CPoolMemoryAllocPolicy<CTransform, 1 << 20>
	{
		public:
			friend TDE2_API IComponent* CreateTransform(E_RESULT_CODE&);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CTransform)

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

				\param[in, out] pWorld A pointer to game world
				\param[in] entitiesIdentifiersRemapper A structure that maps saved identifier to current runtime equivalent

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE PostLoad(IWorld* pWorld, const TEntitiesMapper& entitiesIdentifiersRemapper) override;

			/*!
				\brief The method creates a new deep copy of the instance and returns a smart pointer to it.
				The original state of the object stays the same

				\param[in] pDestObject A valid pointer to an object which the properties will be assigned into
			*/

			TDE2_API E_RESULT_CODE Clone(IComponent*& pDestObject) const override;

			/*!
				\brief The method resets all values of this transform
			*/

			TDE2_API void Reset() override;

			/*!
				\brief The method sets up a global position of an object

				\param[in] A new position's value
			*/

			TDE2_API void SetPosition(const TVector3& position) override;

			/*!
				\brief The method sets up a global rotation of an object

				\param[in] A rotation that is represented in Euler angles
			*/

			TDE2_API void SetRotation(const TVector3& eulerAngles) override;

			/*!
				\brief The method sets up a global rotation of an object

				\param[in] A rotation that is represented via quaternion
			*/

			TDE2_API void SetRotation(const TQuaternion& q) override;

			/*!
				\brief The method sets up a global scale of an object

				\param[in] Per-axis scaling coefficients
			*/

			TDE2_API void SetScale(const TVector3& scale) override;

			/*!
				\brief The method sets up a world transform

				\param[in] transform matrix 4x4 that specifies local to world transformation
			*/

			TDE2_API void SetTransform(const TMatrix4& transform) override;

			TDE2_API E_RESULT_CODE AttachChild(TEntityId childEntityId) override;

			TDE2_API E_RESULT_CODE DettachChild(TEntityId childEntityId) override;

			/*!
				\brief The method assigns an identifier of an entity which will be corresponding as parent of it

				\param[in] parentEntityId An identifier of existing entity. Should not be the same with the current one

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE SetParent(TEntityId parentEntityId) override;

			TDE2_API void SetDirtyFlag(bool value) override;

			TDE2_API E_RESULT_CODE SetOwnerId(TEntityId id) override;

			TDE2_API TEntityId GetOwnerId() const override;

			/*!
				\return The method returns an identifier of a parent or TEntityId::Invalid if the entity has no that
			*/

			TDE2_API TEntityId GetParent() const override;

			TDE2_API const std::vector<TEntityId>& GetChildren() const override;

			/*!
				\brief The method returns a global position's value

				\return The method returns a global position's value
			*/

			TDE2_API const TVector3& GetPosition() const override;

			/*!
				\brief The method returns a global rotation's value

				\return The method returns a global rotation's value
			*/

			TDE2_API const TQuaternion& GetRotation() const override;

			/*!
				\brief The method returns a global scale's value

				\return The method returns a global scale's value
			*/

			TDE2_API const TVector3& GetScale() const override;

			/*!
				\brief The method returns local to world matrix

				\return The method returns local to world matrix
			*/

			TDE2_API const TMatrix4& GetLocalToWorldTransform() const override;

			/*!
				\brief The method returns world to local matrix

				\return The method returns world to local matrix
			*/

			TDE2_API const TMatrix4& GetWorldToLocalTransform() const override;

			/*!
				\brief The method returns a basis vector which corresponds to Z axis in local space of the object

				\return The method returns a basis vector which corresponds to Z axis in local space of the object
			*/

			TDE2_API TVector3 GetForwardVector() const override;

			/*!
				\brief The method returns a basis vector which corresponds to X axis in local space of the object

				\return The method returns a basis vector which corresponds to X axis in local space of the object
			*/

			TDE2_API TVector3 GetRightVector() const override;

			/*!
				\brief The method returns a basis vector which corresponds to Y axis in local space of the object

				\return The method returns a basis vector which corresponds to Y axis in local space of the object
			*/

			TDE2_API TVector3 GetUpVector() const override;
			/*!
				\brief The method returns true if a state of a component was changed

				\return The method returns true if a state of a component was changed
			*/

			TDE2_API bool HasChanged() const override;

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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CTransform)
		protected:
			TVector3    mPosition;

			TQuaternion mRotation;

			TVector3    mScale;

			TEntityId   mParentEntityId = TEntityId::Invalid;
			TEntityId   mOwnerId = TEntityId::Invalid;

			TMatrix4    mLocalToWorldMatrix;
			TMatrix4    mWorldToLocalMatrix;

			bool        mHasChanged;

			std::vector<TEntityId> mChildrenEntities;
	};


	TDE2_DECLARE_COMPONENT_FACTORY(Transform, TTransformParameters);
}
