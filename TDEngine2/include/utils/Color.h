/*!
	\file Color.h
	\date 23.09.2018
	\authors Kasimov Ildar
*/
#pragma once


#include "Types.h"


namespace TDEngine2
{
	/*!
		structure TColor<T>

		\brief The structure is a representation of RGBA color's value
	*/

	template <typename T>
	struct TColor
	{
		TDE2_API TColor() :
			r(0), g(0), b(0), a(0)
		{
		}

		TDE2_API TColor(const T& initializer) :
			r(initializer), g(initializer), b(initializer), a(initializer)
		{
		}

		TDE2_API TColor(const T& r, const T& g, const T& b, const T& a) :
			r(r), g(g), b(b), a(a)
		{
		}

		T r, g, b, a;
	};


	typedef TColor<F32> TColor32F;


	/*!
		\brief The function packs a given color into single U32 value

		\param[in] color A color of TColor32F format, 4 bytes per channel

		\return A signle U32 value with 8 bits per channel
	*/

	TDE2_API constexpr U32 PackColor32F(const TColor32F& color)
	{
		return (static_cast<U8>(color.r * 255) << 24) |
			   (static_cast<U8>(color.g * 255) << 16) |
			   (static_cast<U8>(color.b * 255) << 8)  | 
				static_cast<U8>(color.a * 255);
	}
}