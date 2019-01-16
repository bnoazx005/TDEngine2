/*!
	/file TRect.h
	/date 20.11.2018
	/authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Types.h"
#include "TVector2.h"


namespace TDEngine2
{
	/*!
		struct TRect

		\brief The structure represents a 2D rectangle
	*/

	template <typename T>
	struct TRect
	{
		TRect():
			x(0), y(0), width(0), height(0)
		{
		}

		TRect(T x, T y, T width, T height):
			x(x), y(y), width(width), height(height)
		{
		}
		
		TRect(TRect<T>&& rect):
			x(rect.x), y(rect.y), width(rect.width), height(rect.height)
		{
			rect.x      = 0;
			rect.y      = 0;
			rect.width  = 0;
			rect.height = 0;
		}

		T x, y, width, height;
	};


	typedef TRect<F32> TRectF32;
	typedef TRect<U32> TRectU32;
	typedef TRect<I32> TRectI32;


	/*!
		\brief The operator implements an operation of translation a rectangle
		using an offset vector

		\param[in] rect A rectangle

		\param[in] translation Translation vector

		\return A new translated rectangle with the same sizes
	*/

	template <typename T>
	TRect<T> operator+ (const TRect<T>& rect, const TVector2& translation)
	{
		return { rect.x + translation.x, rect.y + translation.y, rect.width, rect.height };
	}


	/*!
		\brief The function checks up whether a given point lies within a particular rect or not

		\note (the function's result is only correct when the Y axis of a given coordinate space
		is aligned upwards (0; 1)

		\param[in] rect A rectangle's object

		\param[in] point A 2d point

		\return true if a given point lies within a particular rect, false in other cases
	*/

	template <typename T>
	bool ContainsPoint(const TRect<T>& rect, const TVector2& point)
	{
		if (point.x > rect.x && point.x < (rect.x + rect.height) &&
			point.y > (rect.y - rect.width) && point.y < rect.y)
		{
			return true;
		}

		return false;
	}


	/*!
		\brief The function checks up whether a given rect overlapes another rect

		\param[in] leftRect A first rect

		\param[in] rightRect A second rect

		\return Returns true if a given rect overlapes another rect
	*/

	template <typename T>
	bool IsOverlapped(const TRect<T>& leftRect, const TRect<T>& rightRect)
	{
		F32 leftRectX2  = leftRect.x + leftRect.height;
		F32 rightRectX2 = rightRect.x + rightRect.width;
		F32 leftRectY2  = leftRect.y - leftRect.height;
		F32 rightRectY2 = rightRect.y - rightRect.height;

		bool isXOverlapped = (rightRect.x + FloatEpsilon > leftRect.x && rightRect.x - FloatEpsilon < leftRectX2) ||
							 (rightRectX2 + FloatEpsilon> leftRect.x && rightRectX2 - FloatEpsilon < leftRectX2);
		
		bool isYOverlapped = (rightRect.y + FloatEpsilon > leftRectY2 && rightRect.y - FloatEpsilon < leftRect.y) ||
							 (rightRectY2 + FloatEpsilon > leftRectY2 && rightRectY2 - FloatEpsilon < leftRect.y);

		return isXOverlapped && isYOverlapped;
	}


	/*
		\brief The function converts a given point to normalized coordinates which are
		related to a given rectangle

		\param[in] rect A rectangle

		\param[in] point A 2d point

		\return A 2d point which has normalized coordinates relative to a given rectangle.
		If a given point lies outside of the rectangle the function returns a zero vector
	*/

	template <typename T>
	TVector2 PointToNormalizedCoords(const TRect<T>& rect, const TVector2& point)
	{
		TVector2 normalizedPoint { point.x - rect.x, rect.y - point.y };

		if (!ContainsPoint(rect, point))
		{
			return ZeroVector2;
		}

		normalizedPoint.x /= rect.width;
		normalizedPoint.y /= rect.height;

		return normalizedPoint;
	}
}
