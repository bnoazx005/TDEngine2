/*!
	/file TRect.h
	/date 20.11.2018
	/authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Types.h"


namespace TDEngine2
{
	/*!
		struct TRect

		\brief The structure represents a 2D rectangle
	*/

	template <typename T>
	struct TRect
	{
		TRect(T left, T top, T right, T bottom):
			left(left), top(top), right(right), bottom(bottom)
		{
		}
		
		TRect(TRect<T>&& rect):
			left(rect.left), top(rect.top), right(rect.right), bottom(rect.bottom)
		{
			rect.left   = 0;
			rect.top    = 0;
			rect.right  = 0;
			rect.bottom = 0;
		}

		T left, top, right, bottom;
	};


	typedef TRect<F32> TRectF32;
	typedef TRect<U32> TRectU32;
	typedef TRect<I32> TRectI32;
}
