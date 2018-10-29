#include "./../../include/math/TVector3.h"
#include <cmath>


namespace TDEngine2
{
	TVector3::TVector3():
		x(0.0f), y(0.0f), z(0.0f)
	{
	}

	TVector3::TVector3(float initializer) :
		x(initializer), y(initializer), z(initializer)
	{
	}

	TVector3::TVector3(const TVector3& vec3) :
		x(vec3.x), y(vec3.y), z(vec3.z)
	{
	}

	TVector3::TVector3(TVector3&& vec3) :
		x(vec3.x), y(vec3.y), z(vec3.z)
	{
	}

	TVector3::TVector3(const float pArray[3]):
		x(pArray[0]), y(pArray[1]), z(pArray[2])
	{
	}

	TVector3::TVector3(float x, float y, float z) :
		x(x), y(y), z(z)
	{
	}

	TVector3 TVector3::operator= (const TVector3& vec3)
	{
		x = vec3.x;
		y = vec3.y;
		z = vec3.z;

		return *this;
	}

	TVector3& TVector3::operator= (TVector3&& vec3)
	{
		x = vec3.x;
		y = vec3.y;
		z = vec3.z;

		vec3.x = 0.0f;
		vec3.y = 0.0f;
		vec3.z = 0.0f;

		return *this;
	}


	TVector3 operator+ (const TVector3& lvec3, const TVector3& rvec3)
	{
		return TVector3(lvec3.x + rvec3.x, lvec3.y + rvec3.y, lvec3.z + rvec3.z);
	}

	TVector3 operator- (const TVector3& lvec3, const TVector3& rvec3)
	{
		return TVector3(lvec3.x - rvec3.x, lvec3.y - rvec3.y, lvec3.z - rvec3.z);
	}

	TVector3 operator* (const TVector3& lvec3, const TVector3& rvec3)
	{
		F32 lx = lvec3.x, ly = lvec3.y, lz = lvec3.z;
		F32 rx = rvec3.x, ry = rvec3.y, rz = rvec3.z;

		F32 x = ly * rz - lz * ry;
		F32 y = lz * rx - lx * rz;
		F32 z = lx * ry - ly * rx;

		return TVector3(x, y, z);
	}

	TVector3 operator* (const TVector3& lvec3, const F32& coeff)
	{
		return TVector3(lvec3.x * coeff, lvec3.y * coeff, lvec3.z * coeff);
	}

	TVector3 operator* (const F32& coeff, const TVector3& lvec3)
	{
		return TVector3(lvec3.x * coeff, lvec3.y * coeff, lvec3.z * coeff);
	}

	bool operator== (const TVector3& lvec3, const TVector3& rvec3)
	{
		if (fabsf(lvec3.x - rvec3.x) <= FloatEpsilon &&
			fabsf(lvec3.y - rvec3.y) <= FloatEpsilon &&
			fabsf(lvec3.z - rvec3.z) <= FloatEpsilon)
		{
			return true;
		}

		return false;
	}

	bool operator!= (const TVector3& lvec3, const TVector3& rvec3)
	{
		if (fabsf(lvec3.x - rvec3.x) > FloatEpsilon ||
			fabsf(lvec3.y - rvec3.y) > FloatEpsilon ||
			fabsf(lvec3.z - rvec3.z) > FloatEpsilon)
		{
			return true;
		}

		return false;
	}

	TVector3 Cross(const TVector3& lvec3, const TVector3& rvec3)
	{
		return lvec3 * rvec3;
	}

	F32 Dot(const TVector3& lvec3, const TVector3& rvec3)
	{
		return lvec3.x * rvec3.x + lvec3.y * rvec3.y + lvec3.z * rvec3.z;
	}

	TVector3 Normalize(const TVector3& vec3)
	{
		F32 x = vec3.x;
		F32 y = vec3.y;
		F32 z = vec3.z;

		F32 invLength = 1.0f / sqrtf(x * x + y * y + z * z);

		return TVector3(x * invLength, y * invLength, z * invLength);
	}

	F32 Length(const TVector3& vec3)
	{
		F32 x = vec3.x;
		F32 y = vec3.y;
		F32 z = vec3.z;

		return sqrtf(x * x + y * y + z * z);
	}

	TVector3 Negative(const TVector3& vec3)
	{
		return TVector3(-vec3.x, -vec3.y, -vec3.z);
	}

	TVector3 Scale(const TVector3& vec3, const F32& coeff)
	{
		return TVector3(coeff * vec3.x, coeff * vec3.y, coeff * vec3.z);
	}

	TVector3 Scale(const TVector3& vec3l, const TVector3 vec3r)
	{
		return TVector3(vec3l.x * vec3r.x, vec3l.y * vec3r.y, vec3l.z * vec3r.z);
	}
}