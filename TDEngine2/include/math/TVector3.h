/*!
	/file TVector3.h
	/date 01.10.2018
	/authors Kasimov Ildar
*/

#pragma once


#include "./../../include/utils/Types.h"
#include "./../../include/utils/Utils.h"
#include <string>


namespace TDEngine2
{
	/*!
		struct TVector3

		\brief The structure describes a 3 dimensional vector
	*/

	typedef struct TVector3
	{
		F32 x, y, z;

		TDE2_API TVector3();

		/*!
			\brief The constructor initializes a vector's components
			with the same specified value

			\param[in] initializer A value, which will be assigned to all the components
		*/

		TDE2_API TVector3(F32 initializer);

		/*!
			\brief Simple copy constructor
		*/

		TDE2_API TVector3(const TVector3& vec3);

		/*!
			\brief Move constructor
		*/

		TDE2_API TVector3(TVector3&& vec3);

		/*!
			\brief The constructor, which uses an array's values to initialize
			vector's components

			\param[in] pArray An array contains components' values
		*/

		TDE2_API TVector3(const F32 pArray[3]);

		/*!
			\brief The constructor with separate arguments for 
			components initialization
		*/

		TDE2_API TVector3(F32 x, F32 y, F32 z);

		/*!
			\brief Assigment operator for TVector3
			\param vec3 A reference to another vec3
			\return A TVector3's instance, which equals to the input
		*/

		TDE2_API TVector3 operator= (const TVector3& vec3);

		/*!
			\brief Assigment operator for TVector3
			\param vec3 A reference to another vec3
			\return A TVector3's instance, which equals to the input
		*/

		TDE2_API TVector3& operator= (TVector3&& vec3);

		/*!
			\brief The operator returns opposite vector to the given one

			\return The operator returns opposite vector to the given one
		*/

		TDE2_API TVector3 operator- () const;

		TDE2_API inline std::string ToString() const { return CStringUtils::Format("TVector3({0}, {1}, {2})", x, y, z); }
	} TVector3, *TVector3Ptr;


	/// TVector3's predefined constants

	static const TVector3 ZeroVector3    = TVector3();
	static const TVector3 RightVector3   = TVector3(1.0f, 0.0f, 0.0f);
	static const TVector3 UpVector3      = TVector3(0.0f, 1.0f, 0.0f);
	static const TVector3 ForwardVector3 = TVector3(0.0f, 0.0f, 1.0f);


	///TVector3's operators overloading
	TDE2_API TVector3 operator+ (const TVector3& lvec3, const TVector3& rvec3);

	TDE2_API TVector3 operator- (const TVector3& lvec3, const TVector3& rvec3);

	TDE2_API TVector3 operator* (const TVector3& lvec3, const TVector3& rvec3);

	TDE2_API TVector3 operator* (const TVector3& lvec3, const F32& coeff);

	TDE2_API TVector3 operator* (const F32& coeff, const TVector3& lvec3);

	TDE2_API bool operator== (const TVector3& lvec3, const TVector3& rvec3);

	TDE2_API bool operator!= (const TVector3& lvec3, const TVector3& rvec3);


	///TVector3's functions

	/*!
		\brief The method computes a cross product of two 3d vectors

		\param[in] A first 3d vector
		\param[in] A second 3d vector

		\return The method computes a cross product of two 3d vectors.
	*/

	TDE2_API TVector3 Cross(const TVector3& lvec3, const TVector3& rvec3);

	/*!
		\brief The method computes a dot product of two 3d vectors

		\param[in] A first 3d vector
		\param[in] A second 3d vector

		\return The method computes a dot product of two 3d vectors.
		A dot product can be computed by following formula 
		dot = lvec3.x * rvec3.x + lvec3.y * rvec3.y + lvec3.z * rvec3.z
	*/

	TDE2_API F32 Dot(const TVector3& lvec3, const TVector3& rvec3);

	/*!
		\brief The method returns a length of a vector

		\param[in] A 3d vector

		\return The method returns a length of a vector
	*/

	TDE2_API F32 Length(const TVector3& vec3);

	/*!
		\brief The method returns a normalized copy of a specified vector

		\param[in] A 3d vector that should be normalized

		\return The method returns a normalized copy of a specified vector
	*/

	TDE2_API TVector3 Normalize(const TVector3& vec3);

	/*!
		\brief The method is particular case of Scale method with -1 as an argument

		\param[in] vec3 A 3d vector

		\return A negative vector of the specified one
	*/

	TDE2_API TVector3 Negative(const TVector3& vec3);

	/*!
		\brief The method scales specified vector using the second argument's value

		\param[in] vec3 A 3d vector
		\param[in] coeff A scale coefficient

		\return The result, which equals to (vec3.x * coeff, vec3.y * coeff, vec3.z * coeff)
	*/

	TDE2_API TVector3 Scale(const TVector3& vec3, const F32& coeff);

	/*!
		\brief The method multiplies two vectors component-wise

		\param[in] A first 3d vector
		\param[in] A second 3d vector

		\return The result, which equals to (vec3l.x * vec3r.x, vec3l.y * vec3r.y, vec3l.z * vec3r.z)
	*/

	TDE2_API TVector3 Scale(const TVector3& vec3l, const TVector3& vec3r);

	/*!
		\brief The method returns linear interpolation of two 3d vectors

		\param[in] a First 3d vector
		\param[in] b Second 3d vector

		\return A 3d vector which is a linear combination of two given vectors based on third parameter
	*/

	TDE2_API TVector3 Lerp(const TVector3& a, const TVector3& b, F32 t);

	/*!
		\brief The functions returns unclamped random 3d vector. Each component can lie in range of from MIN_FLOAT to MAX_FLOAT
		\return The functions returns unclamped random 3d vector
	*/

	TDE2_API TVector3 RandVector3();
}
