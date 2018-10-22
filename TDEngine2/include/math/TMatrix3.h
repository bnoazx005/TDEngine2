/*!
	/file TMatrix3.h
	/date 23.10.2018
	/authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Types.h"
#include "TVector3.h"


namespace TDEngine2
{
	/*!
		struct TMatrix3

		\brief The structure represents a squared matrix with size of 3.
	*/

	typedef struct TMatrix3
	{
		F32 m[3][3];

		/*!
			\brief The default constructor generates a zero matrix
		*/

		TDE2_API TMatrix3();

		/*!
			\brief The constructor that assigns values from the specified array
			in row wise order

			\param[in] arr An array of 9 floats
		*/

		TDE2_API TMatrix3(const F32 arr[9]);
		
		/*!
			\brief The constructor that assigns values from the specified arguments
		*/

		TDE2_API TMatrix3(F32 m00, F32 m01, F32 m02, F32 m10, F32 m11, F32 m12, F32 m20, F32 m21, F32 m22);

		/*!
			\brief The constructor that creates a diagonal matrix

			\param[in] diagElements A 3d vector that contains diagonal elements values
		*/

		TDE2_API TMatrix3(const TVector3& diagElements);

		/*!
			\brief Copy constructor

			\param[in] mat A 3x3 matrix that will be used as a copy's origin
		*/

		TDE2_API TMatrix3(const TMatrix3& mat);

		/*!
			\brief Move constructor

			\param[in] mat A 3x3 matrix that will be moved into the existing one
		*/

		TDE2_API TMatrix3(TMatrix3&& mat);

		/*!
			\brief An assigment operator for TMatrix3

			\param mat A reference to another 3x3 matrix

			\return A TMatrix3's instance, which equals to the input
		*/

		TDE2_API TMatrix3 operator= (const TMatrix3& mat);

		/*!
			\brief An assigment operator for TMatrix3

			\param mat A reference to another 3x3 matrix

			\return A TMatrix3's instance, which equals to the input
		*/

		TDE2_API TMatrix3& operator= (TMatrix3&& mat);
	} TMatrix3, *TMatrix3Ptr;


	/// TMatrix3's predefined constants

	static const TMatrix3 ZeroMatrix3     = TMatrix3();
	static const TMatrix3 IdentityMatrix3 = TMatrix3(TVector3(1.0f, 1.0f, 1.0f));


	/// TMatrix3's operators

	TDE2_API TMatrix3 operator+ (const TMatrix3& lmat3, const TMatrix3& rmat3);

	TDE2_API TMatrix3 operator- (const TMatrix3& lmat3, const TMatrix3& rmat3);

	TDE2_API TMatrix3 operator* (const TMatrix3& lmat3, const TMatrix3& rmat3);

	TDE2_API TVector3 operator* (const TMatrix3& mat3, const TVector3& vec3);

	TDE2_API TMatrix3 operator* (const TMatrix3& mat3, const F32& coeff);

	TDE2_API TMatrix3 operator* (const F32& coeff, const TMatrix3& mat3);


	///TMatrix3's functions

	/*!
		\brief The function implements matrix multiplication for TMatrix3

		\param[in] lmat3 Left 3x3 matrix
		\param[in] rmat3 Right 3x3 matrix

		\return The result of matrix multiplication
	*/

	TDE2_API TMatrix3 Mul(const TMatrix3& lmat3, const TMatrix3& rmat3);

	/*!
		\brief The function implements matrix-vector multiplication

		\param[in] mat3 3x3 matrix
		\param[in] vec3 3d vector

		\return The result of matrix-vector multiplication
	*/

	TDE2_API TVector3 Mul(const TMatrix3& mat3, const TVector3& vec3);

	/*!
		\brief The function computes an inversed matrix for the given one

		\param[in] mat3 3x3 matrix

		\return The function computes an inversed matrix for the given one
	*/

	TDE2_API TMatrix3 Inverse(const TMatrix3& mat3);

	/*!
		\brief The function computes a transposed matrix for the given one

		\param[in] mat3 3x3 matrix

		\return The function computes a transposed matrix for the given one
	*/

	TDE2_API TMatrix3 Transpose(const TMatrix3& mat3);

	/*!
		\brief The function computes a determinant of a 3x3 matrix

		\param[in] mat3 3x3 matrix

		\return The function computes a determinant of a 3x3 matrix
	*/

	TDE2_API F32 Det(const TMatrix3& mat3);
}
