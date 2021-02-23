/*!
	\file Color.h
	\date 23.09.2018
	\authors Kasimov Ildar
*/
#pragma once


#include "Types.h"
#include "Utils.h"
#include "../core/Serialization.h"


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

		constexpr TDE2_API TColor(const T& initializer) :
			r(initializer), g(initializer), b(initializer), a(initializer)
		{
		}

		constexpr TDE2_API TColor(const T& r, const T& g, const T& b, const T& a) :
			r(r), g(g), b(b), a(a)
		{
		}

		T r, g, b, a;
	};


	typedef TColor<F32> TColor32F;


	struct TColorUtils
	{
		static constexpr TColor32F mWhite   = TColor32F(1.0f);
		static constexpr TColor32F mBlack   = TColor32F(0.0f, 0.0f, 0.0f, 1.0f);
		static constexpr TColor32F mRed     = TColor32F(1.0f, 0.0f, 0.0f, 1.0f);
		static constexpr TColor32F mGreen   = TColor32F(0.0f, 1.0f, 0.0f, 1.0f);
		static constexpr TColor32F mBlue    = TColor32F(0.0f, 0.0f, 1.0f, 1.0f);
		static constexpr TColor32F mMagenta = TColor32F(1.0f, 0.0f, 1.0f, 1.0f);
		static constexpr TColor32F mYellow  = TColor32F(1.0f, 1.0f, 0.0f, 1.0f);
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

		pWriter->SetFloat("r", object.r);
		pWriter->SetFloat("g", object.g);
		pWriter->SetFloat("b", object.b);
		pWriter->SetFloat("a", object.a);

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
}