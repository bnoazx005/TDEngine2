/*!
	\file ITransform.h
	\date 29.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Types.h"
#include "./../utils/Utils.h"
#include "./../math/TVector3.h"
#include "./../math/TQuaternion.h"


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
				\brief The method initializes an internal state of a transform

				\param[in] position A global position of an object

				\param[in] rotation A global rotation of an object

				\param[in] scale A global scale of an object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(const TVector3& position, const TQuaternion& rotation, const TVector3& scale) = 0;

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
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ITransform)
	};
}
