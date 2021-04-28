#include "./../../include/math/TVector2.h"
#include <cmath>


namespace TDEngine2
{
	TVector2::TVector2():
		x(0.0f), y(0.0f)
	{
	}

	TVector2::TVector2(float initializer):
		x(initializer), y(initializer)
	{
	}

	TVector2::TVector2(const TVector2& vec2):
		x(vec2.x), y(vec2.y)
	{
	}

	TVector2::TVector2(TVector2&& vec2):
		x(vec2.x), y(vec2.y)
	{
	}

	TVector2::TVector2(const float pArray[2]):
		x(pArray[0]), y(pArray[1])
	{
	}

	TVector2::TVector2(float x, float y):
		x(x), y(y)
	{
	}

	TVector2 TVector2::operator= (const TVector2& vec2)
	{
		x = vec2.x;
		y = vec2.y;

		return *this;
	}

	TVector2& TVector2::operator= (TVector2&& vec2)
	{
		x = vec2.x;
		y = vec2.y;

		vec2.x = 0.0f;
		vec2.y = 0.0f;

		return *this;
	}

	TVector2 TVector2::operator- () const
	{
		return { -x, -y };
	}


	TVector2 operator+ (const TVector2& lvec2, const TVector2& rvec2)
	{
		return TVector2(lvec2.x + rvec2.x, lvec2.y + rvec2.y);
	}

	TVector2 operator- (const TVector2& lvec2, const TVector2& rvec2)
	{
		return TVector2(lvec2.x - rvec2.x, lvec2.y - rvec2.y);
	}

	TVector2 operator* (const TVector2& lvec2, const F32& coeff)
	{
		return TVector2(lvec2.x * coeff, lvec2.y * coeff);
	}

	TVector2 operator* (const F32& coeff, const TVector2& lvec2)
	{
		return TVector2(lvec2.x * coeff, lvec2.y * coeff);
	}

	bool operator== (const TVector2& lvec2, const TVector2& rvec2)
	{
		if (fabsf(lvec2.x - rvec2.x) <= FloatEpsilon &&
			fabsf(lvec2.y - rvec2.y) <= FloatEpsilon)
		{
			return true;
		}

		return false;
	}

	bool operator!= (const TVector2& lvec2, const TVector2& rvec2)
	{
		if (fabsf(lvec2.x - rvec2.x) > FloatEpsilon ||
			fabsf(lvec2.y - rvec2.y) > FloatEpsilon)
		{
			return true;
		}

		return false;
	}

	F32 Dot(const TVector2& lvec2, const TVector2& rvec2)
	{
		return lvec2.x * rvec2.x + lvec2.y * rvec2.y;
	}

	TVector2 Normalize(const TVector2& vec2)
	{
		F32 x = vec2.x;
		F32 y = vec2.y;

		F32 invLength = 1.0f / sqrtf(x * x + y * y);

		return TVector2(x * invLength, y * invLength);
	}

	F32 Length(const TVector2& vec2)
	{
		F32 x = vec2.x;
		F32 y = vec2.y;

		return sqrtf(x * x + y * y);
	}

	TVector2 Negative(const TVector2& vec2)
	{
		return TVector2(-vec2.x, -vec2.y);
	}

	TVector2 Scale(const TVector2& vec2, const F32& coeff)
	{
		return TVector2(coeff * vec2.x, coeff * vec2.y);
	}

	TVector2 Scale(const TVector2& vec2l, const TVector2& vec2r)
	{
		return TVector2(vec2l.x * vec2r.x, vec2l.y * vec2r.y);
	}


	TResult<TVector2> LoadVector2(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}

		TVector2 vec;

		vec.x = pReader->GetFloat("x");
		vec.y = pReader->GetFloat("y");

		return Wrench::TOkValue<TVector2>(vec);
	}

	E_RESULT_CODE SaveVector2(IArchiveWriter* pWriter, const TVector2& object)
	{
		if (!pWriter)
		{
			return RC_INVALID_ARGS;
		}

		pWriter->SetFloat("x", object.x);
		pWriter->SetFloat("y", object.y);

		return RC_OK;
	}
}