/*!
	/file TRay.h
	/date 05.04.2020
	/authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Types.h"
#include "TVector2.h"
#include "TVector3.h"
#include <tuple>


namespace TDEngine2
{
	/*!
		struct TRay

		\brief The structure represents template implementation of a ray
	*/

	template <typename T>
	struct TRay
	{
		TRay(const T& origin, const T& direction) :
			origin(origin), dir(Normalize(direction))
		{
		}

		T origin;
		T dir;
	};


	typedef TRay<TVector2> TRay2D;
	typedef TRay<TVector3> TRay3D;
}