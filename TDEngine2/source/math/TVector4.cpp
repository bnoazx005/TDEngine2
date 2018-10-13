#include "./../../include/math/TVector4.h"
#include "./../../include/math/TVector3.h"
#include <cmath>


namespace TDEngine2
{
	TVector4::TVector4():
		x(0.0f), y(0.0f), z(0.0f), w(0.0f)
	{
	}

	TVector4::TVector4(float initializer) :
		x(initializer), y(initializer), z(initializer), w(initializer)
	{
	}
	 
	TVector4::TVector4(const TVector4& vec4) :
		x(vec4.x), y(vec4.y), z(vec4.z), w(vec4.w)
	{
	}

	TVector4::TVector4(TVector4&& vec4) :
		x(vec4.x), y(vec4.y), z(vec4.z), w(vec4.w)
	{
	}

	TVector4::TVector4(const float pArray[4]) :
		x(pArray[0]), y(pArray[1]), z(pArray[2]), w(pArray[3])
	{
	}

	TVector4::TVector4(float x, float y, float z, float w) :
		x(x), y(y), z(z), w(w)
	{
	}

	TVector4::TVector4(const TVector3& vec3, float w) :
		x(vec3.x), y(vec3.y), z(vec3.z), w(w)
	{
	}

	TVector4  TVector4::operator= (const TVector4& vec4)
	{
		x = vec4.x;
		y = vec4.y;
		z = vec4.z;
		w = vec4.w;

		return *this;
	}

	TVector4&  TVector4::operator= (TVector4&& vec4)
	{
		x = vec4.x;
		y = vec4.y;
		z = vec4.z;
		w = vec4.w;

		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
		w = 0.0f;

		return *this;
	}


	TVector4 operator+ (const TVector4& lvec4, const TVector4& rvec4)
	{
		return TVector4(lvec4.x + rvec4.x, lvec4.y + rvec4.y, lvec4.z + rvec4.z, lvec4.w + rvec4.w);
	}

	TVector4 operator- (const TVector4& lvec4, const TVector4& rvec4)
	{
		return TVector4(lvec4.x - rvec4.x, lvec4.y - rvec4.y, lvec4.z - rvec4.z, lvec4.w - rvec4.w);
	}

	TVector4 operator* (const TVector4& lvec4, const float& coeff)
	{
		return TVector4(lvec4.x * coeff, lvec4.y * coeff, lvec4.z * coeff, lvec4.w * coeff);
	}
	
	TVector4 operator* (const float& coeff, const TVector4& lvec4)
	{
		return TVector4(lvec4.x * coeff, lvec4.y * coeff, lvec4.z * coeff, lvec4.w * coeff);
	}

	TVector4 operator* (const TVector4& lvec4, const TVector4& rvec4)
	{
		return TVector4(lvec4.x * rvec4.x, lvec4.y * rvec4.y, lvec4.z * rvec4.z, lvec4.w * rvec4.w);
	}

	bool operator== (const TVector4& lvec4, const TVector4& rvec4)
	{
		if (fabsf(lvec4.x - rvec4.x) <= FloatEpsilon &&
			fabsf(lvec4.y - rvec4.y) <= FloatEpsilon &&
			fabsf(lvec4.z - rvec4.z) <= FloatEpsilon &&
			fabsf(lvec4.w - rvec4.w) <= FloatEpsilon)
		{
			return true;
		}

		return false;
	}

	bool operator!= (const TVector4& lvec4, const TVector4& rvec4)
	{
		if (fabsf(lvec4.x - rvec4.x) > FloatEpsilon ||
			fabsf(lvec4.y - rvec4.y) > FloatEpsilon ||
			fabsf(lvec4.z - rvec4.z) > FloatEpsilon ||
			fabsf(lvec4.w - rvec4.w) > FloatEpsilon)
		{
			return true;
		}

		return false;
	}


	float Dot(const TVector4& lvec4, const TVector4& rvec4)
	{
		return lvec4.x * rvec4.x + lvec4.y * rvec4.y + lvec4.z * rvec4.z + lvec4.w * rvec4.w;
	}

	TVector4 Normalize(const TVector4& vec4)
	{
		float x = vec4.x;
		float y = vec4.y;
		float z = vec4.z;
		float w = vec4.w;

		float invLength = 1.0f / sqrtf(x * x + y * y + z * z + w * w);

		return TVector4(x * invLength, y * invLength, z * invLength, w * invLength);
	}

	float Length(const TVector4& vec4)
	{
		float x = vec4.x;
		float y = vec4.y;
		float z = vec4.z;
		float w = vec4.w;

		return sqrtf(x * x + y * y + z * z + w * w);
	}

	TVector4 Scale(const TVector4& lvec4, const TVector4& rvec4)
	{
		return TVector4(lvec4.x * rvec4.x, lvec4.y * rvec4.y, lvec4.z * rvec4.z, lvec4.w * rvec4.w);
	}
}