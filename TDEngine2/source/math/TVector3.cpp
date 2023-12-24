#include "../../include/math/TVector3.h"
#include "../../include/math/MathUtils.h"
#include "../../include/utils/Utils.h"
#include "../../include/core/Serialization.h"
#include <cmath>
#include <limits>
#include <algorithm>


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

	TVector3::TVector3(const TVector2& vec2):
		x(vec2.x), y(vec2.y), z(0.0f)
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

	TVector3 TVector3::operator- () const
	{
		return { -x, -y, -z };
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
		if (fabs(lvec3.x - rvec3.x) < FloatEpsilon &&
			fabs(lvec3.y - rvec3.y) < FloatEpsilon &&
			fabs(lvec3.z - rvec3.z) < FloatEpsilon)
		{
			return true;
		}

		return false;
	}

	bool operator!= (const TVector3& lvec3, const TVector3& rvec3)
	{
		if (fabs(lvec3.x - rvec3.x) > FloatEpsilon ||
			fabs(lvec3.y - rvec3.y) > FloatEpsilon ||
			fabs(lvec3.z - rvec3.z) > FloatEpsilon)
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

	TVector3 Scale(const TVector3& vec3l, const TVector3& vec3r)
	{
		return TVector3(vec3l.x * vec3r.x, vec3l.y * vec3r.y, vec3l.z * vec3r.z);
	}

	TVector3 Lerp(const TVector3& a, const TVector3& b, F32 t)
	{
		return (1.0f - t) * a + t * b;
	}

	TVector3 RandVector3()
	{
		constexpr F32 leftBound  = (std::numeric_limits<F32>::min)();
		constexpr F32 rightBound = (std::numeric_limits<F32>::max)();

		constexpr F32 rangeWidth = rightBound - leftBound;

		F32 x = leftBound + static_cast<F32>(rand()) / (static_cast<F32>(RAND_MAX / rangeWidth));
		F32 y = leftBound + static_cast<F32>(rand()) / (static_cast<F32>(RAND_MAX / rangeWidth));
		F32 z = leftBound + static_cast<F32>(rand()) / (static_cast<F32>(RAND_MAX / rangeWidth));

		return TVector3(x, y, z);
	}

	TVector3 RandVector3(const TVector3& min, const TVector3& max)
	{
		F32 x = min.x + static_cast<F32>(rand()) / (static_cast<F32>(RAND_MAX / (std::max)(1.0f, max.x - min.x)));
		F32 y = min.y + static_cast<F32>(rand()) / (static_cast<F32>(RAND_MAX / (std::max)(1.0f, max.y - min.y)));
		F32 z = min.z + static_cast<F32>(rand()) / (static_cast<F32>(RAND_MAX / (std::max)(1.0f, max.z - min.z)));

		return TVector3(x, y, z);
	}


	TVector3 Reflect(const TVector3& vec, const TVector3& normal)
	{
		return 2.0f * Dot(vec, normal) * Normalize(normal) - vec;
	}

	TVector3 Fractional(const TVector3& vec)
	{
		return vec - Floor(vec);
	}	
	
	TVector3 Floor(const TVector3& vec)
	{
		return TVector3(floor(vec.x), floor(vec.y), floor(vec.z));
	}


	TResult<TVector3> LoadVector3(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}

		TVector3 vec;

		vec.x = pReader->GetFloat("x");
		vec.y = pReader->GetFloat("y");
		vec.z = pReader->GetFloat("z");

		return Wrench::TOkValue<TVector3>(vec);
	}

	E_RESULT_CODE SaveVector3(IArchiveWriter* pWriter, const TVector3& object)
	{
		if (!pWriter)
		{
			return RC_INVALID_ARGS;
		}

		bool anyWritten = false;
		
		if (std::fabs(object.y) > 1e-3f)
		{
			pWriter->SetFloat("y", object.y);
			anyWritten = true;
		}

		if (std::fabs(object.z) > 1e-3f)
		{
			pWriter->SetFloat("z", object.z);
			anyWritten = true;
		}

		if (!anyWritten || (anyWritten && std::fabs(object.x) > 1e-3f))
		{
			pWriter->SetFloat("x", object.x); /// \note Always write at least x component to prevent the bug of serialization in the Yaml library
		}

		return RC_OK;
	}


	template <> TDE2_API E_RESULT_CODE Serialize<TVector3>(IArchiveWriter* pWriter, TVector3 value)
	{
		return pWriter->SetUInt32("type_id", static_cast<U32>(GetTypeId<TVector3>::mValue)) | SaveVector3(pWriter, value);
	}

	template <> TDE2_API TResult<TVector3> Deserialize<TVector3>(IArchiveReader* pReader)
	{
		return LoadVector3(pReader);
	}
}