/*!
	/file TQuaternion.h
	/date 23.10.2018
	/authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Types.h"
#include "TVector3.h"


namespace TDEngine2
{
	/*!
		struct TQuaternion

		\brief The structure represents a quaternion q = (w x y z), which
		in nutshell is a rotation around v = (x y z) axis with w angle
	*/

	typedef struct TQuaternion
	{
		F32 w, x, y, z;

		/*!
			\brief Default constructor
		*/

		TDE2_API TQuaternion();

		TDE2_API TQuaternion(F32 x, F32 y, F32 z, F32 w);

		/*!
			\brief The constructor creates a quaternion from direction v
			and angle w. q = (w v.x v.y v.z)

			\param[in] v Rotation axis
			\param[in] w Rotation angle
		*/

		TDE2_API TQuaternion(const TVector3& v, F32 w);

		/*!
			\brief The constructor creates a quaternion from Euler angles

			\param[in] eulerAngles Euler angles representation
		*/

		TDE2_API TQuaternion(const TVector3& eulerAngles);

		TDE2_API TQuaternion(const TQuaternion& q);
		
		TDE2_API TQuaternion(TQuaternion&& q);
		
		/*!
			\brief An assigment operator for TQuaternion

			\param[in] mat A reference to another quaternion

			\return A TQuaternion's instance, which equals to the input
		*/

		TDE2_API TQuaternion operator= (const TQuaternion& q);

		/*!
			\brief An assigment operator for TQuaternion

			\param[in, out] mat A reference to another quaternion

			\return A TQuaternion's instance, which equals to the input
		*/

		TDE2_API TQuaternion& operator= (TQuaternion&& q);
	} TQuaternion, *TQuaternionPtr;


	/// TQuaternion's predefined constants
	static const TQuaternion ZeroQuaternion = TQuaternion();
	static const TQuaternion UnitQuaternion = TQuaternion(0.0f, 0.0f, 0.0f, 1.0f);
}