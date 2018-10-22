/*!
	/file TMatrix4.h
	/date 22.10.2018
	/authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Types.h"
#include "TVector4.h"


namespace TDEngine2
{
	/*!
		struct TMatrix4

		\brief The structure represents a squared matrix with size of 4.
		It has a lot of applications within the engine
	*/

	typedef struct TMatrix4
	{
		F32 m[4][4];

		/*!
			\brief The default constructor generates a zero matrix
		*/

		TDE2_API TMatrix4();

		/*!
			\brief The constructor that assigns values from the specified array
			in row wise order

			\param[in] arr An array of 16 floats
		*/

		TDE2_API TMatrix4(const F32 arr[16]);

		/*!
			\brief The constructor that creates a diagonal matrix

			\param[in] diagElements A 4d vector that contains diagonal elements values
		*/

		TDE2_API TMatrix4(const TVector4& diagElements);

		/*!
			\brief Copy constructor

			\param[in] mat A 4x4 matrix that will be used as a copy's origin
		*/

		TDE2_API TMatrix4(const TMatrix4& mat);

		/*!
			\brief Move constructor

			\param[in] mat A 4x4 matrix that will be moved into the existing one
		*/

		TDE2_API TMatrix4(TMatrix4&& mat);
		
		/*!
			\brief An assigment operator for TMatrix4

			\param mat A reference to another 4x4 matrix

			\return A TMatrix4's instance, which equals to the input
		*/

		TDE2_API TMatrix4 operator= (const TMatrix4& mat);
		
		/*!
			\brief An assigment operator for TMatrix4

			\param mat A reference to another 4x4 matrix

			\return A TMatrix4's instance, which equals to the input
		*/

		TDE2_API TMatrix4& operator= (TMatrix4&& mat);
	} TMatrix4, *TMatrix4Ptr;
	

	/// TMatrix4's predefined constants

	static const TMatrix4 ZeroMatrix4     = TMatrix4();
	static const TMatrix4 IdentityMatrix4 = TMatrix4(TVector4(1.0f, 1.0f, 1.0f, 1.0f));


	/// TMatrix4's operators
	
	TDE2_API TMatrix4 operator+ (const TMatrix4& lmat4, const TMatrix4& rmat4);

	TDE2_API TMatrix4 operator- (const TMatrix4& lmat4, const TMatrix4& rmat4);

	TDE2_API TMatrix4 operator* (const TMatrix4& lmat4, const TMatrix4& rmat4);

	TDE2_API TVector4 operator* (const TMatrix4& mat4, const TVector4& vec4);

	TDE2_API TMatrix4 operator* (const TMatrix4& mat4, const F32& coeff);

	TDE2_API TMatrix4 operator* (const F32& coeff, const TMatrix4& mat4);


	///TMatrix4's functions

	/*!
		\brief The function implements matrix multiplication for TMatrix4

		\param[in] lmat4 Left 4x4 matrix 
		\param[in] rmat4 Right 4x4 matrix 

		\return The result of matrix multiplication
	*/

	TDE2_API TMatrix4 Mul(const TMatrix4& lmat4, const TMatrix4& rmat4);

	/*!
		\brief The function implements matrix-vector multiplication

		\param[in] mat4 4x4 matrix
		\param[in] vec4 4d vector

		\return The result of matrix-vector multiplication
	*/

	TDE2_API TVector4 Mul(const TMatrix4& mat4, const TVector4& vec4);

	/*!
		\brief The function computes an inversed matrix for the given one

		\param[in] mat4 4x4 matrix

		\return The function computes an inversed matrix for the given one
	*/

	TDE2_API TMatrix4 Inverse(const TMatrix4& mat4);

	/*!
		\brief The function computes a transposed matrix for the given one

		\param[in] mat4 4x4 matrix

		\return The function computes a transposed matrix for the given one
	*/

	TDE2_API TMatrix4 Transpose(const TMatrix4& mat4);

	/*!
		\brief The function computes a determinant of a 4x4 matrix

		\param[in] mat4 4x4 matrix

		\return The function computes a determinant of a 4x4 matrix
	*/

	TDE2_API F32 Det(const TMatrix4& mat4);
}
