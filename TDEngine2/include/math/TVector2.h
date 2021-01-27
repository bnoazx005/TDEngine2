/*!
	/file TVector2.h
	/date 17.12.2018
	/authors Kasimov Ildar
*/

#pragma once


#include "./../../include/utils/Types.h"
#include "../../include/core/Serialization.h"
#include "./stringUtils.hpp"
#include <string>


namespace TDEngine2
{
	/*!
		struct TVector2

		\brief The structure describes a 2 dimensional vector
	*/

	typedef struct TVector2
	{
		F32 x, y;

		TDE2_API TVector2();

		/*!
			\brief The constructor initializes a vector's components
			with the same specified value

			\param[in] initializer A value, which will be assigned to all the components
		*/

		TDE2_API TVector2(F32 initializer);

		/*!
			\brief Simple copy constructor
		*/

		TDE2_API TVector2(const TVector2& vec2);

		/*!
			\brief Move constructor
		*/

		TDE2_API TVector2(TVector2&& vec2);

		/*!
			\brief The constructor, which uses an array's values to initialize
			vector's components

			\param[in] pArray An array contains components' values
		*/

		TDE2_API TVector2(const F32 pArray[2]);

		/*!
			\brief The constructor with separate arguments for
			components initialization
		*/

		TDE2_API TVector2(F32 x, F32 y);

		/*!
			\brief Assigment operator for TVector2
			\param vec2 A reference to another vec2
			\return A TVector2's instance, which equals to the input
		*/

		TDE2_API TVector2 operator= (const TVector2& vec2);

		/*!
			\brief Assigment operator for TVector2
			\param vec2 A reference to another vec2
			\return A TVector2's instance, which equals to the input
		*/

		TDE2_API TVector2& operator= (TVector2&& vec2);

		/*!
			\brief The operator returns opposite vector to the given one

			\return The operator returns opposite vector to the given one
		*/

		TDE2_API TVector2 operator- () const;

		TDE2_API inline std::string ToString() const { return Wrench::StringUtils::Format("TVector2({0}, {1})", x, y); }
	} TVector2, *TVector2Ptr;


	/// TVector2's predefined constants

	static const TVector2 ZeroVector2  = TVector2();
	static const TVector2 RightVector2 = TVector2(1.0f, 0.0f);
	static const TVector2 UpVector2    = TVector2(0.0f, 1.0f);


	///TVector2's operators overloading
	TDE2_API TVector2 operator+ (const TVector2& lvec2, const TVector2& rvec2);

	TDE2_API TVector2 operator- (const TVector2& lvec2, const TVector2& rvec2);

	TDE2_API TVector2 operator* (const TVector2& lvec2, const TVector2& rvec2);

	TDE2_API TVector2 operator* (const TVector2& lvec2, const F32& coeff);

	TDE2_API TVector2 operator* (const F32& coeff, const TVector2& lvec2);

	TDE2_API bool operator== (const TVector2& lvec2, const TVector2& rvec2);

	TDE2_API bool operator!= (const TVector2& lvec2, const TVector2& rvec2);


	///TVector2's functions

	/*!
		\brief The method computes a dot product of two 2d vectors

		\param[in] A first 2d vector
		\param[in] A second 2d vector

		\return The method computes a dot product of two 2d vectors.
		A dot product can be computed by following formula
		dot = lvec2.x * rvec2.x + lvec2.y * rvec2.y
	*/

	TDE2_API F32 Dot(const TVector2& lvec2, const TVector2& rvec2);

	/*!
		\brief The method returns a length of a vector

		\param[in] A 2d vector

		\return The method returns a length of a vector
	*/

	TDE2_API F32 Length(const TVector2& vec2);

	/*!
		\brief The method returns a normalized copy of a specified vector

		\param[in] A 2d vector that should be normalized

		\return The method returns a normalized copy of a specified vector
	*/

	TDE2_API TVector2 Normalize(const TVector2& vec2);

	/*!
		\brief The method is particular case of Scale method with -1 as an argument

		\param[in] vec2 A 2d vector

		\return A negative vector of the specified one
	*/

	TDE2_API TVector2 Negative(const TVector2& vec2);

	/*!
		\brief The method scales specified vector using the second argument's value

		\param[in] vec2 A 2d vector
		\param[in] coeff A scale coefficient

		\return The result, which equals to (vec2.x * coeff, vec2.y * coeff)
	*/

	TDE2_API TVector2 Scale(const TVector2& vec2, const F32& coeff);

	/*!
		\brief The method multiplies two vectors component-wise

		\param[in] A first 2d vector
		\param[in] A second 2d vector

		\return The result, which equals to (vec2l.x * vec2r.x, vec2l.y * vec2r.y)
	*/

	TDE2_API TVector2 Scale(const TVector2& vec2l, const TVector2& vec2r);


	/*!
		\brief TVector3's Serialization/Deserialization helpers
	*/

	TDE2_API TResult<TVector2> LoadVector2(IArchiveReader* pReader);
	TDE2_API E_RESULT_CODE SaveVector2(IArchiveWriter* pWriter, const TVector2& object);
}
