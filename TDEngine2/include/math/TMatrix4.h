/*!
	/file TMatrix4.h
	/date 22.10.2018
	/authors Kasimov Ildar
*/

#pragma once


#include "../utils/Types.h"
#include "../utils/Utils.h"
#include "TVector4.h"
#include "TVector3.h"
#include "TVector2.h"
#include <string>


namespace TDEngine2
{
	/*!
		union TMatrix4

		\brief The structure represents a squared matrix with size of 4.
		It has a lot of applications within the engine
	*/

	typedef union TMatrix4
	{
		F32 m[4][4];
		F32 arr[16];

		/*!
			\brief The default constructor generates a zero matrix
		*/

		TDE2_API TMatrix4();

		/*!
			\brief The constructor that assigns values from the specified array
			in row wise order

			\param[in] arr An array of 16 floats
		*/

		TDE2_API explicit TMatrix4(const F32 arr[16]);

		TDE2_API TMatrix4(F32 _11, F32 _12, F32 _13, F32 _14, 
						  F32 _21, F32 _22, F32 _23, F32 _24, 
						  F32 _31, F32 _32, F32 _33, F32 _34,
						  F32 _41, F32 _42, F32 _43, F32 _44);

		/*!
			\brief The constructor that creates a diagonal matrix

			\param[in] diagElements A 4d vector that contains diagonal elements values
		*/

		TDE2_API explicit TMatrix4(const TVector4& diagElements);

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

		TDE2_API std::string ToString() const;
	} TMatrix4, *TMatrix4Ptr;
	

	/// TMatrix4's predefined constants

	static const TMatrix4 ZeroMatrix4     = TMatrix4();
	static const TMatrix4 IdentityMatrix4 = TMatrix4(TVector4(1.0f, 1.0f, 1.0f, 1.0f));


	/// TMatrix4's operators
	
	TDE2_API TMatrix4 operator+ (const TMatrix4& lmat4, const TMatrix4& rmat4);

	TDE2_API TMatrix4 operator- (const TMatrix4& lmat4, const TMatrix4& rmat4);

	TDE2_API TMatrix4 operator* (const TMatrix4& lmat4, const TMatrix4& rmat4);

	TDE2_API TVector4 operator* (const TMatrix4& mat4, const TVector4& vec4);
	TDE2_API TVector3 operator* (const TMatrix4& mat4, const TVector3& vec3);
	TDE2_API TVector2 operator* (const TMatrix4& mat4, const TVector2& vec2);

	TDE2_API TMatrix4 operator* (const TMatrix4& mat4, const F32& coeff);

	TDE2_API TMatrix4 operator* (const F32& coeff, const TMatrix4& mat4);


	TDE2_API bool operator== (const TMatrix4& lmat4, const TMatrix4& rmat4);
	TDE2_API bool operator!= (const TMatrix4& lmat4, const TMatrix4& rmat4);


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

	/*!
		\brief The function makes orthonormal matrix from the input one 
	*/

	TDE2_API TMatrix4 Normalize(const TMatrix4& mat4);

	/*!
		\brief The function computes translation matrix for a given shift

		\param[in] t A 3d vector that contains shift values

		\return The function computes translation matrix 
	*/

	TDE2_API TMatrix4 TranslationMatrix(const TVector3& t);

	/*!
		\brief The function computes a matrix of nonuniform scaling

		\param[in] s A 3d vector that contains scaling coefficient that are relative to XYZ axis

		\return The function computes a matrix of nonuniform scaling
	*/

	TDE2_API TMatrix4 ScaleMatrix(const TVector3& s);

	/*!
		\brief The function computes GAPI independed matrix of a perspective projection	
		
		\param[in] fov A field of view

		\param[in] aspect An aspect ratio of a screen

		\param[in] zn A z value of a near clip plance

		\param[in] zf A z value of a far clip plane

		\param[in] zNDCMin A minimal value along Z axis within NDC

		\param[in] zNDCMax A maximum value  along Z axis within NDC

		\return The function computes GAPI independed matrix of a perspective projection	
	*/

	TDE2_API TMatrix4 PerspectiveProj(F32 fov, F32 aspect, F32 zn, F32 zf, F32 zNDCMin, F32 zNDCMax, F32 handedness);

	/*!
		\brief The function computes GAPI independed matrix of an orthographic projection

		\param[in] left X coordinate of a left corner of a screen

		\param[in] top Y coordinate of a left top corner of a screen

		\param[in] right X coordinate of a right corner of a screen

		\param[in] bottom Y coordinate of a right bottom corner of a screen

		\param[in] zn A z value of a near clip plance

		\param[in] zf A z value of a far clip plane

		\param[in] zNDCMin A minimal value along Z axis within NDC

		\param[in] zNDCMax A maximum value  along Z axis within NDC

		\param[in] handedness Equals to 1 for a right-handed, -1 for a left-handed

		\param[in] isDepthless The flag determines whether the output matrix will process z axis or just to project onto some constant, which
		equals to average position between zn and zf

		\return The function computes GAPI independed matrix of a orthographic projection
	*/

	TDE2_API TMatrix4 OrthographicProj(F32 left, F32 top, F32 right, F32 bottom, F32 zn, F32 zf, F32 zNDCMin, F32 zNDCMax, F32 handedness, bool isDepthless = false);


	/*!
		\brief The function generates a view matrix based on given parameters

		\param[in] eye A position of a camera
		\param[in] up An up vector of the camera
		\param[in] target A direction of sight
		\param[in] handedness Equals to 1 for a right-handed, -1 for a left-handed

		\return The function returns a view matrix 
	*/

	TDE2_API TMatrix4 LookAt(const TVector3& eye, const TVector3& up, const TVector3& target, F32 handedness);


	/*!
		\brief TMatrix4's Serialization/Deserialization helpers
	*/

	TDE2_API TResult<TMatrix4> LoadMatrix4(class IArchiveReader* pReader);
	TDE2_API E_RESULT_CODE SaveMatrix4(class IArchiveWriter* pWriter, const TMatrix4& object);
}
