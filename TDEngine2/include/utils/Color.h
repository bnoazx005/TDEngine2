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
}