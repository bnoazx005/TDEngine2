/*!
	/file TRect.h
	/date 20.11.2018
	/authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Types.h"
#include "TVector2.h"
#include <tuple>


namespace TDEngine2
{
	/*!
		struct TRect

		\brief The structure represents a 2D rectangle
	*/

	template <typename T>
	struct TRect
	{
		T x, y, width, height;

		TDE2_API constexpr TRect() = default;
		TDE2_API constexpr TRect(const TRect&) = default;
		TDE2_API constexpr TRect(TRect&&) = default;

		TDE2_API constexpr TRect(T x, T y, T width, T height):
			x(x), y(y), width(width), height(height)
		{
		}

		TDE2_API TRect(const TVector2& leftBottom, const TVector2& rightTop):
			x(static_cast<T>(leftBottom.x)), y(static_cast<T>(leftBottom.y))
		{
			const TVector2 sizes = rightTop - leftBottom;

			width = static_cast<T>(std::abs(sizes.x));
			height = static_cast<T>(std::abs(sizes.y));
		}

		TDE2_API TRect& operator= (TRect<T> rect)
		{
			_swap(rect);
			return *this;
		}

		TDE2_API TVector2 GetLeftBottom() const { return TVector2(static_cast<F32>(x), static_cast<F32>(y)); }
		TDE2_API TVector2 GetRightTop() const { return TVector2(static_cast<F32>(x + width), static_cast<F32>(y + height)); }
		TDE2_API TVector2 GetSizes() const { return TVector2(static_cast<F32>(width), static_cast<F32>(height)); }

		protected:
			TDE2_API void _swap(TRect<T>& rect)
			{
				std::swap(x, rect.x);
				std::swap(y, rect.y);
				std::swap(width, rect.width);
				std::swap(height, rect.height);
			}
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


	template <typename T>
	bool operator== (const TRect<T>& leftRect, const TRect<T>& rightRect)
	{
		return (leftRect.x == rightRect.x) &&
				(leftRect.y == rightRect.y) &&
				(leftRect.width == rightRect.width) &&
				(leftRect.height == rightRect.height);
	}


	template <typename T>
	bool operator!= (const TRect<T>& leftRect, const TRect<T>& rightRect)
	{
		return (leftRect.x != rightRect.x) ||
				(leftRect.y != rightRect.y) ||
				(leftRect.width != rightRect.width) ||
				(leftRect.height != rightRect.height);
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

	
	/*!
		\brief The function splits a single rectangle in two based on a given cutting line

		\param[in] rect An original rectangle
		\param[in] pos A position should be normalized relatively to rectangle
		\param[in] isVertical A flag that defines a direction of spliting 

		\return A tuple which consists of two rectangles
	*/

	template <typename T>
	std::tuple<TRect<T>, TRect<T>> SplitRectWithLine(const TRect<T>& rect, const TVector2& pos, bool isVertical = false)
	{
		T ox = static_cast<T>(isVertical ? rect.x + pos.x : rect.x);
		T oy = static_cast<T>(isVertical ? rect.y : rect.y + pos.y);

		T firstRectWidth   = static_cast<T>(isVertical ? pos.x : rect.width);
		T firstRectHeight  = static_cast<T>(isVertical ? rect.height - pos.y : pos.y);
		T secondRectWidth  = static_cast<T>(isVertical ? rect.width - pos.x : rect.width);
		T secondRectHeight = static_cast<T>(isVertical ? rect.height : rect.height - pos.y);

		return { {rect.x, rect.y, firstRectWidth, firstRectHeight}, {ox, oy, secondRectWidth, secondRectHeight} };
	}
}
