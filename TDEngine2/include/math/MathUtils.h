/*!
	/file MathUtils.h
	/date 22.11.2018
	/authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Types.h"


namespace TDEngine2
{
	/*!
		\brief The static class contains common mathematical constants
		that are often used
	*/

	class CMathConstants
	{
		public:
			constexpr static F32 Pi      = 3.141592f;

			constexpr static F32 Exp     = 2.728281f;

			constexpr static F32 Deg2Rad = Pi / 180.0f;

			constexpr static F32 Rad2Deg = 180.0f / Pi;
	};


}