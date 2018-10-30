/*!
	\file CTransform.h
	\date 29.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "CBaseComponent.h"
#include "ITransform.h"


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
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CTransform)
		protected:
			TVector3    mPosition;

			TQuaternion mRotation;

			TVector3    mScale;
	};
}
