/*!
	/file TVector4.h
	/date 01.10.2018
	/authors Kasimov Ildar
*/

#pragma once


#include "./../../include/utils/Types.h"


namespace TDEngine2
{
	struct TVector3;


	/*!
		struct TVector4

		\brief The structure describes a 4-dimensional vector.
		Note that (x, y, z, 1) means a point and (x, y, z, 0) is a direction (real vector).
		In the engine TVector4 is used to represent both points and vectors.
	*/

	typedef struct TVector4
	{
		F32 x, y, z, w;

		TDE2_API TVector4();

		/*!
			\brief The constructor initializes a vector's components
			with the same specified value

			\param[in] initializer A value, which will be assigned to all the components
		*/

		TDE2_API TVector4(F32 initializer);

		/*!
			\brief Simple copy constructor
		*/

		TDE2_API TVector4(const TVector4& vec4);

		/*!
			\brief Move constructor
		*/

		TDE2_API TVector4(TVector4&& vec4);

		/*!
			\brief The constructor, which uses an array's values to initialize
			vector's components

			\param[in] pArray An array contains components' values
		*/

		TDE2_API TVector4(const F32 pArray[4]);

		/*!
			\brief The constructor with separate arguments for
			components initialization
		*/

		TDE2_API TVector4(F32 x, F32 y, F32 z, F32 w);

		/*!
			\brief The constructor, which uses a 3d vector and w value
			to initialize a 4d vector

			\param[in] vec3 A 3d vector
			\param[in] w A homogeneous coordinate
		*/

		TDE2_API TVector4(const TVector3& vec3, F32 w);

		/*!
			\brief Assigment operator for TVector4
			\param vec4 A reference to another vec4
			\return A TVector4's instance, which equals to the input
		*/

		TDE2_API TVector4 operator= (const TVector4& vec4);

		/*!
			\brief Assigment operator for TVector4
			\param vec4 A reference to another vec4
			\return A TVector4's instance, which equals to the input
		*/

		TDE2_API TVector4& operator= (TVector4&& vec4);
	} TVector4, *TVector4Ptr;
	

	///TVector4's operators overloading

	TDE2_API TVector4 operator+ (const TVector4& lvec4, const TVector4& rvec4);

	TDE2_API TVector4 operator- (const TVector4& lvec4, const TVector4& rvec4);

	TDE2_API TVector4 operator* (const TVector4& lvec4, const F32& coeff);
	
	TDE2_API TVector4 operator* (const F32& coeff, const TVector4& lvec4);

	TDE2_API TVector4 operator* (const TVector4& lvec4, const TVector4& rvec4);

	TDE2_API bool operator== (const TVector4& lvec4, const TVector4& rvec4);

	TDE2_API bool operator!= (const TVector4& lvec4, const TVector4& rvec4);


	///TVector4's functions

	/*!
		\brief The method computes a dot product of two 3d vectors

		\param[in] A first 4d vector
		\param[in] A second 4d vector

		\return The method computes a dot product of two 4d vectors.
		A dot product can be computed by following formula
		dot = lvec4.x * rvec4.x + lvec4.y * rvec4.y + lvec4.z * rvec4.z + lvec4.w * rvec4.w
	*/

	TDE2_API F32 Dot(const TVector4& lvec4, const TVector4& rvec4);

	/*!
		\brief The method returns a length of a vector

		\param[in] A 4d vector

		\return The method returns a length of a vector
	*/

	TDE2_API F32 Length(const TVector4& vec4);

	/*!
		\brief The method returns a normalized copy of a specified vector

		\param[in] A 4d vector that should be normalized

		\return The method returns a normalized copy of a specified vector
	*/

	TDE2_API TVector4 Normalize(const TVector4& vec4);

	/*!
		\brief The method multiplies two vectors component-wise

		\param[in] A first 4d vector
		\param[in] A second 4d vector

		\return The result, which equals to (lvec4.x * rvec4.x, lvec4.y * rvec4.y, lvec4.z * rvec4.z, lvec4.w * rvec4.w)
	*/

	TDE2_API TVector4 Scale(const TVector4& lvec4, const TVector4& rvec4);
}