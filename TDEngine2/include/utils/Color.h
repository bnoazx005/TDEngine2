/*!
	\file Color.h
	\date 23.09.2018
	\authors Kasimov Ildar
*/
#pragma once


#include "Types.h"
#include "Utils.h"
#include "../core/Serialization.h"
#include "../math/MathUtils.h"
#include <cmath>


namespace TDEngine2
{
	/*!
		structure TColor<T>

		\brief The structure is a representation of RGBA color's value
	*/

	template <typename T>
	struct TColor
	{
		constexpr TDE2_API TColor() :
			r(0), g(0), b(0), a(0)
		{
		}

		constexpr explicit TDE2_API TColor(const T& initializer) :
			r(initializer), g(initializer), b(initializer), a(initializer)
		{
		}

		constexpr explicit TDE2_API TColor(const T arr[4]) :
			r(arr[0]), g(arr[1]), b(arr[2]), a(arr[3])
		{
		}

		constexpr TDE2_API TColor(const T& r, const T& g, const T& b, const T& a) :
			r(r), g(g), b(b), a(a)
		{
		}

		T r, g, b, a;
	};


	typedef TColor<F32> TColor32F;


	template <typename T>
	TColor<T> operator+ (const TColor<T>& lcolor, const TColor<T>& rcolor)
	{
		return TColor<T>(lcolor.r + rcolor.r, lcolor.g + rcolor.g, lcolor.b + rcolor.b, lcolor.a + rcolor.a);
	}
	
	template <>
	inline TColor<F32> operator+ <F32>(const TColor<F32>& lcolor, const TColor<F32>& rcolor)
	{
		return TColor<F32>(
			CMathUtils::Clamp01(lcolor.r + rcolor.r), 
			CMathUtils::Clamp01(lcolor.g + rcolor.g), 
			CMathUtils::Clamp01(lcolor.b + rcolor.b), 
			CMathUtils::Clamp01(lcolor.a + rcolor.a));
	}

	template <typename T>
	TColor<T> operator* (const TColor<T>& lcolor, const TColor<T>& rcolor)
	{
		return TColor<T>(lcolor.r * rcolor.r, lcolor.g * rcolor.g, lcolor.b * rcolor.b, lcolor.a * rcolor.a);
	}

	template <typename T>
	TColor<T> operator* (const TColor<T>& color, const T& value)
	{
		return TColor<T>(color.r * value, color.g * value, color.b * value, color.a * value);
	}

	template <typename T>
	TColor<T> operator* (const T& value, const TColor<T>& color)
	{
		return TColor<T>(color.r * value, color.g * value, color.b * value, color.a * value);
	}


	struct TColorUtils
	{
		TDE2_API TDE2_STATIC_CONSTEXPR TColor32F mWhite   = TColor32F(1.0f);
		TDE2_API TDE2_STATIC_CONSTEXPR TColor32F mBlack   = TColor32F(0.0f, 0.0f, 0.0f, 1.0f);
		TDE2_API TDE2_STATIC_CONSTEXPR TColor32F mRed     = TColor32F(1.0f, 0.0f, 0.0f, 1.0f);
		TDE2_API TDE2_STATIC_CONSTEXPR TColor32F mGreen   = TColor32F(0.0f, 1.0f, 0.0f, 1.0f);
		TDE2_API TDE2_STATIC_CONSTEXPR TColor32F mBlue    = TColor32F(0.0f, 0.0f, 1.0f, 1.0f);
		TDE2_API TDE2_STATIC_CONSTEXPR TColor32F mMagenta = TColor32F(1.0f, 0.0f, 1.0f, 1.0f);
		TDE2_API TDE2_STATIC_CONSTEXPR TColor32F mYellow  = TColor32F(1.0f, 1.0f, 0.0f, 1.0f);
		TDE2_API TDE2_STATIC_CONSTEXPR TColor32F mGray    = TColor32F(0.5f, 0.5f, 0.5f, 1.0f);
	};


	TDE2_API inline TColor32F RandColor32F()
	{
		const F32 invDenom = 1.0f / (static_cast<F32>(RAND_MAX) + 1.0f);

		return TColor32F(static_cast<F32>(rand()) * invDenom, static_cast<F32>(rand()) * invDenom, static_cast<F32>(rand()) * invDenom, static_cast<F32>(rand()) * invDenom);
	}


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


	/*!
		\brief The function packs a given color into single U32 value

		\param[in] color A color of TColor32F format, 4 bytes per channel

		\return A signle U32 value with 8 bits per channel
	*/

	TDE2_API constexpr U32 PackABGRColor32F(const TColor32F& color)
	{
		return (static_cast<U8>(color.a * 255) << 24) |
			(static_cast<U8>(color.b * 255) << 16) |
			(static_cast<U8>(color.g * 255) << 8) |
			static_cast<U8>(color.r * 255);
	}


	template <typename T>
	TColor<T> LerpColors(const TColor<T>& left, const TColor<T>& right, F32 t)
	{
		const F32 invT = (1.0f - t);

		return TColor<T>(left.r * invT + right.r * t,
						left.g * invT + right.g * t,
						left.b * invT + right.b * t,
						left.a * invT + right.a * t);
	}


	/*!
		\brief TColor32F's Serialization/Deserialization helpers
	*/

	TDE2_API inline TResult<TColor32F> LoadColor32F(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}

		TColor32F color;

		color.r = pReader->GetFloat("r");
		color.g = pReader->GetFloat("g");
		color.b = pReader->GetFloat("b");
		color.a = pReader->GetFloat("a");

		return Wrench::TOkValue<TColor32F>(color);
	}

	TDE2_API inline E_RESULT_CODE SaveColor32F(IArchiveWriter* pWriter, const TColor32F& object)
	{
		if (!pWriter)
		{
			return RC_INVALID_ARGS;
		}

		bool anyWritten = false;

		if (std::fabs(object.g) > 1e-3f) { pWriter->SetFloat("g", object.g); anyWritten = true; }
		if (std::fabs(object.b) > 1e-3f) { pWriter->SetFloat("b", object.b); anyWritten = true; }
		if (std::fabs(object.a) > 1e-3f) { pWriter->SetFloat("a", object.a); anyWritten = true; }

		if (!anyWritten || (anyWritten && std::fabs(object.r) > 1e-3f))
		{
			pWriter->SetFloat("r", object.r); /// \note Always write at least x component to prevent the bug of serialization in the Yaml library
		}

		return RC_OK;
	}


	TDE2_API inline TColor32F RandColor(bool allowTransparency = false)
	{
		constexpr F32 denominator = static_cast<F32>(RAND_MAX);

		return TColor32F(static_cast<F32>(rand()) / denominator,
						static_cast<F32>(rand()) / denominator,
						static_cast<F32>(rand()) / denominator,
						allowTransparency ? static_cast<F32>(rand()) / denominator : 1.0f);
	}


	template <> struct GetTypeId<TColor32F> { TDE2_API TDE2_STATIC_CONSTEXPR TypeId mValue = TDE2_TYPE_ID(TColor32F); };


	template <> TDE2_API inline E_RESULT_CODE Serialize<TColor32F>(class IArchiveWriter* pWriter, TColor32F value)
	{
		return pWriter->SetUInt32("type_id", static_cast<U32>(GetTypeId<TColor32F>::mValue)) | SaveColor32F(pWriter, value);
	}
}