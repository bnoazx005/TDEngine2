/*!
	/file TQuaternion.h
	/date 23.10.2018
	/authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Types.h"
#include "TVector3.h"
#include "TMatrix4.h"


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


	/// TQuaternion's operators

	TDE2_API TQuaternion operator+ (const TQuaternion& q1, const TQuaternion& q2);

	TDE2_API TQuaternion operator- (const TQuaternion& q1, const TQuaternion& q2);

	TDE2_API TQuaternion operator* (const TQuaternion& q1, const TQuaternion& q2);

	TDE2_API TQuaternion operator* (F32 scalar, const TQuaternion& q);

	TDE2_API TQuaternion operator* (const TQuaternion& q, F32 scalar);


	/*!
		\brief The method computes a conjugate quaternion to specified one

		\param[in] An input quaternion

		\return The method returns a quaternion that conjugated to specified one
	*/

	TDE2_API TQuaternion Conjugate(const TQuaternion& q);

	/*!
		\brief The method returns a length of a quaternion

		\param[in] An input quaternion
		
		\return The method returns a length of specified quaternion
	*/

	TDE2_API F32 Length(const TQuaternion& q);

	/*!
		\brief The method normalizes specified quaternion

		\param[in] An input quaternion

		\return Normalized quaternion (||q|| = 1, where ||.|| means length)
	*/

	TDE2_API TQuaternion Normalize(const TQuaternion& q);

	/*!
		\brief The method computes an inversed quaternion to a given one

		\param[in] An input quaternion

		\return The method returns an inversed quaternion based on the input
	*/

	TDE2_API TQuaternion Inverse(const TQuaternion& q);

	/*!
		\brief The method computes a mid quaternion based via linear interpolation algorithm
		and specified parameter t. The following formula is used q' = (1 - t) q1 + t q2

		\param[in] An input quaternion

		\return The method returns a lineary interpolated quaternion 
	*/

	TDE2_API TQuaternion Lerp(const TQuaternion& q1, const TQuaternion& q2, F32 t);

	/*!
		\brief The method computes a mid quaternion based via spherical linear interpolation algorithm
		and specified parameter t. The following formula is used 
		q' = [sin [Theta(1 - t)] / sin Theta] q1 + [Sin [Theta t] / Sin Theta] q2, where
		Theta = cos ^ -1 (q1 . q2)

		\param[in] An input quaternion

		\return The method returns a lineary interpolated quaternion
	*/

	TDE2_API TQuaternion Slerp(const TQuaternion& q1, const TQuaternion& q2, F32 t);

	/*!
		\brief The method converts a given quaternion to a matrix 4x4
		
		\param[in] An input quaternion

		\return The method returns a rotation matrix 4x4 which was computed from a given quaternion
	*/

	TDE2_API TMatrix4 RotationMatrix(const TQuaternion& q);

	/*!
		\brief The method converts a given quaternion into Euler angles

		\param[in] An input quaternion

		\return The method returns a 3d vector each component of which represents Euler angles
	*/

	TDE2_API TVector3 ToEulerAngles(const TQuaternion& q);


	/*!
		\brief TQuaternion's Serialization/Deserialization helpers
	*/

	TDE2_API TResult<TQuaternion> LoadQuaternion(IArchiveReader* pReader);
	TDE2_API E_RESULT_CODE SaveQuaternion(IArchiveWriter* pWriter, const TQuaternion& object);
}