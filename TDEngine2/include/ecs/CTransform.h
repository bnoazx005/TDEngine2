/*!
	\file CTransform.h
	\date 29.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "CBaseComponent.h"
#include "ITransform.h"
#include "IComponentFactory.h"
#include "./../math/TMatrix4.h"


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CTransform's type.

		\param[in] position A global position of an object

		\param[in] rotation A global rotation of an object

		\param[in] scale A global scale of an object

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to IComponent's implementation
	*/

	TDE2_API IComponent* CreateTransform(const TVector3& position, const TQuaternion& rotation, const TVector3& scale, E_RESULT_CODE& result);


	/*!
		class CTransform

		\brief The class is a basic component of all entities
		It stores all transformations that were applied to
		an object
	*/

	class CTransform: public ITransform, public CBaseComponent
	{
		public:
			friend TDE2_API IComponent* CreateTransform(const TVector3& position, const TQuaternion& rotation, const TVector3& scale, E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_TYPE(CTransform)

			/*!
				\brief The method initializes an internal state of a transform

				\param[in] position A global position of an object

				\param[in] rotation A global rotation of an object

				\param[in] scale A global scale of an object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(const TVector3& position, const TQuaternion& rotation, const TVector3& scale) override;

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

			TDE2_API const TMatrix4& GetTransform() const override;

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
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CTransform)
		protected:
			TVector3    mPosition;

			TQuaternion mRotation;

			TVector3    mScale;

			TMatrix4    mLocalToWorldMatrix;

			bool        mHasChanged;
	};


	/*!
		\brief A factory function for creation objects of CTransformFactory's type.
		
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CTransformFactory's implementation
	*/

	TDE2_API IComponentFactory* CreateTransformFactory(E_RESULT_CODE& result);


	/*!
		class CTransformFactory

		\brief The class is factory facility to create a new objects of CTransform type
	*/

	class CTransformFactory: public ITransformFactory, public CBaseObject
	{
		public:
			friend TDE2_API IComponentFactory* CreateTransformFactory(E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an internal state of a factory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init() override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method creates a new instance of a component based on passed parameters

				\param[in] pParams An object that contains parameters that are needed for the component's creation

				\return A pointer to a new instance of IComponent type
			*/

			TDE2_API IComponent* Create(const TBaseComponentParameters* pParams) const override;

			/*!
				\brief The method creates a new instance of a component based on passed parameters

				\param[in] pParams An object that contains parameters that are needed for the component's creation

				\return A pointer to a new instance of IComponent type
			*/

			TDE2_API IComponent* CreateDefault(const TBaseComponentParameters& params) const override;

			/*!
				\brief The method returns an identifier of a component's type, which
				the factory serves

				\return The method returns an identifier of a component's type, which
				the factory serves
			*/

			TDE2_API TypeId GetComponentTypeId() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CTransformFactory)
	};
}
