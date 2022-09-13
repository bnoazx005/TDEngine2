/*!
	\file ITransform.h
	\date 29.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Types.h"
#include "../utils/Utils.h"
#include "../math/TVector3.h"
#include "../math/TQuaternion.h"


namespace TDEngine2
{
	/*!
		interface ITransform

		\brief The interface represents a functionality of a
		transform component, which is a basic component for all entities in 
		the engine
	*/

	class ITransform
	{
		public:
			/*!
				\brief The method resets all values of this transform
			*/

			TDE2_API virtual void Reset() = 0;

			/*!
				\brief The method sets up a global position of an object

				\param[in] A new position's value
			*/

			TDE2_API virtual void SetPosition(const TVector3& position) = 0;

			/*!
				\brief The method sets up a global rotation of an object

				\param[in] A rotation that is represented in Euler angles
			*/

			TDE2_API virtual void SetRotation(const TVector3& eulerAngles) = 0;

			/*!
				\brief The method sets up a global rotation of an object

				\param[in] A rotation that is represented via quaternion
			*/

			TDE2_API virtual void SetRotation(const TQuaternion& q) = 0;

			/*!
				\brief The method sets up a global scale of an object

				\param[in] Per-axis scaling coefficients 
			*/

			TDE2_API virtual void SetScale(const TVector3& scale) = 0;

			/*!
				\brief The method sets up a world transform

				\param[in] transform matrix 4x4 that specifies local to world transformation
			*/

			TDE2_API virtual void SetTransform(const TMatrix4& local2World, const TMatrix4& child2Parent) = 0;

			TDE2_API virtual E_RESULT_CODE AttachChild(TEntityId childEntityId) = 0;

			TDE2_API virtual E_RESULT_CODE DettachChild(TEntityId childEntityId) = 0;

			/*!
				\brief The method assigns an identifier of an entity which will be corresponding as parent of it

				\param[in] parentEntityId An identifier of existing entity. Should not be the same with the current one

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE SetParent(TEntityId parentEntityId) = 0;

			TDE2_API virtual void SetDirtyFlag(bool value) = 0;

			TDE2_API virtual void SetHierarchyChangedFlag(bool value) = 0;

			TDE2_API virtual E_RESULT_CODE SetOwnerId(TEntityId id) = 0;

			TDE2_API virtual TEntityId GetOwnerId() const = 0;

			/*!
				\return The method returns an identifier of a parent or TEntityId::Invalid if the entity has no that
			*/

			TDE2_API virtual TEntityId GetParent() const = 0;

			TDE2_API virtual const std::vector<TEntityId>& GetChildren() const = 0;

			/*!
				\brief The method returns a global position's value

				\return The method returns a global position's value
			*/

			TDE2_API virtual const TVector3& GetPosition() const = 0;

			/*!
				\brief The method returns a global rotation's value

				\return The method returns a global rotation's value
			*/

			TDE2_API virtual const TQuaternion& GetRotation() const = 0;

			/*!
				\brief The method returns a global scale's value

				\return The method returns a global scale's value
			*/

			TDE2_API virtual const TVector3& GetScale() const = 0;

			/*!
				\brief The method returns local to world matrix

				\return The method returns local to world matrix
			*/

			TDE2_API virtual const TMatrix4& GetLocalToWorldTransform() const = 0;

			/*!
				\brief The method returns world to local matrix

				\return The method returns world to local matrix
			*/

			TDE2_API virtual const TMatrix4& GetWorldToLocalTransform() const = 0;

			TDE2_API virtual const TMatrix4& GetChildToParentTransform() const = 0;

			/*!
				\brief The method returns a basis vector which corresponds to Z axis in local space of the object

				\return The method returns a basis vector which corresponds to Z axis in local space of the object
			*/

			TDE2_API virtual TVector3 GetForwardVector() const = 0;

			/*!
				\brief The method returns a basis vector which corresponds to X axis in local space of the object

				\return The method returns a basis vector which corresponds to X axis in local space of the object
			*/

			TDE2_API virtual TVector3 GetRightVector() const = 0;

			/*!
				\brief The method returns a basis vector which corresponds to Y axis in local space of the object

				\return The method returns a basis vector which corresponds to Y axis in local space of the object
			*/

			TDE2_API virtual TVector3 GetUpVector() const = 0;

			/*!
				\brief The method returns true if a state of a component was changed

				\return The method returns true if a state of a component was changed
			*/

			TDE2_API virtual bool HasChanged() const = 0;

			TDE2_API virtual bool HasHierarchyChanged() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ITransform)
	};


	class ITransformFactory : public IGenericComponentFactory<> {};
}
