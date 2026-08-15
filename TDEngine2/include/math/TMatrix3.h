/*!
	/file TMatrix3.h
	/date 23.10.2018
	/authors Kasimov Ildar
*/

#pragma once


#include "../utils/Types.h"
#include "TVector3.h"


namespace TDEngine2
{
	/*!
		struct TMatrix3

		\brief The structure represents a squared matrix with size of 3.
	*/

	typedef struct TMatrix3
	{
		F32 m[3][3] 
		{ 
			{ 1.0f, 0.0f, 0.0f },
			{ 0.0f, 1.0f, 0.0f }, 
			{ 0.0f, 0.0f, 1.0f } 
		};

		/*!
			\brief The default constructor generates a zero matrix
		*/

		TDE2_API constexpr TMatrix3() = default;

		/*!
			\brief The constructor that assigns values from the specified array
			in row wise order

			\param[in] arr An array of 9 floats
		*/

		TDE2_API constexpr explicit TMatrix3(const F32 arr[9])
		{
			for (I32 i = 0; i < 3; ++i)
			{
				for (I32 j = 0; j < 3; ++j)
				{
					m[i][j] = arr[i * 3 + j];
				}
			}
		}
		
		/*!
			\brief The constructor that assigns values from the specified arguments
		*/

		TDE2_API constexpr TMatrix3(F32 m00, F32 m01, F32 m02, F32 m10, F32 m11, F32 m12, F32 m20, F32 m21, F32 m22)
		{
			m[0][0] = m00;
			m[0][1] = m01;
			m[0][2] = m02;

			m[1][0] = m10;
			m[1][1] = m11;
			m[1][2] = m12;

			m[2][0] = m20;
			m[2][1] = m21;
			m[2][2] = m22;
		}

		/*!
			\brief The constructor that creates a diagonal matrix

			\param[in] diagElements A 3d vector that contains diagonal elements values
		*/

		TDE2_API constexpr explicit TMatrix3(const TVector3& diagElements)
		{
			m[0][0] = diagElements.x;
			m[1][1] = diagElements.y;
			m[2][2] = diagElements.z;
		}

		TDE2_API constexpr TMatrix3(const TMatrix3& mat) = default;
		TDE2_API constexpr TMatrix3(TMatrix3&& mat) = default;

		TDE2_API constexpr TMatrix3& operator= (const TMatrix3& mat) = default;
		TDE2_API constexpr TMatrix3& operator= (TMatrix3&& mat) = default;
	} TMatrix3, *TMatrix3Ptr;


	static_assert(std::is_trivially_copyable_v<TMatrix3>, "TMatrix3 should be trivially copyable");


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


	TDE2_API bool operator== (const TMatrix3& lmat3, const TMatrix3& rmat3);
	TDE2_API bool operator!= (const TMatrix3& lmat3, const TMatrix3& rmat3);


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
