#include "./../../include/math/TVector4.h"
#include "./../../include/math/TVector3.h"
#include <algorithm>
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

		vec4.x = 0.0f;
		vec4.y = 0.0f;
		vec4.z = 0.0f;
		vec4.w = 0.0f;

		return *this;
	}

	TVector4 TVector4::operator- () const
	{
		return { -x, -y, -z, -w };
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
		if (fabs(lvec4.x - rvec4.x) <= FloatEpsilon &&
			fabs(lvec4.y - rvec4.y) <= FloatEpsilon &&
			fabs(lvec4.z - rvec4.z) <= FloatEpsilon &&
			fabs(lvec4.w - rvec4.w) <= FloatEpsilon)
		{
			return true;
		}

		return false;
	}

	bool operator!= (const TVector4& lvec4, const TVector4& rvec4)
	{
		if (fabs(lvec4.x - rvec4.x) > FloatEpsilon ||
			fabs(lvec4.y - rvec4.y) > FloatEpsilon ||
			fabs(lvec4.z - rvec4.z) > FloatEpsilon ||
			fabs(lvec4.w - rvec4.w) > FloatEpsilon)
		{
			return true;
		}

		return false;
	}


	F32 Dot(const TVector4& lvec4, const TVector4& rvec4)
	{
		return lvec4.x * rvec4.x + lvec4.y * rvec4.y + lvec4.z * rvec4.z + lvec4.w * rvec4.w;
	}

	TVector4 Normalize(const TVector4& vec4)
	{
		F32 x = vec4.x;
		F32 y = vec4.y;
		F32 z = vec4.z;
		F32 w = vec4.w;

		F32 invLength = 1.0f / sqrtf(x * x + y * y + z * z + w * w);

		return TVector4(x * invLength, y * invLength, z * invLength, w * invLength);
	}

	F32 Length(const TVector4& vec4)
	{
		F32 x = vec4.x;
		F32 y = vec4.y;
		F32 z = vec4.z;
		F32 w = vec4.w;

		return sqrtf(x * x + y * y + z * z + w * w);
	}

	TVector4 Scale(const TVector4& lvec4, const TVector4& rvec4)
	{
		return TVector4(lvec4.x * rvec4.x, lvec4.y * rvec4.y, lvec4.z * rvec4.z, lvec4.w * rvec4.w);
	}

	TVector4 Min(const TVector4& lvec4, const TVector4& rvec4)
	{
		F32 x = std::min<F32>(lvec4.x, rvec4.x);
		F32 y = std::min<F32>(lvec4.y, rvec4.y);
		F32 z = std::min<F32>(lvec4.z, rvec4.z);
		F32 w = std::min<F32>(lvec4.w, rvec4.w);

		return TVector4(x, y, z, w);
	}

	TVector4 Max(const TVector4& lvec4, const TVector4& rvec4)
	{
		F32 x = std::max<F32>(lvec4.x, rvec4.x);
		F32 y = std::max<F32>(lvec4.y, rvec4.y);
		F32 z = std::max<F32>(lvec4.z, rvec4.z);
		F32 w = std::max<F32>(lvec4.w, rvec4.w);

		return TVector4(x, y, z, w);
	}


	TResult<TVector4> LoadVector4(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}

		TVector4 vec;

		vec.x = pReader->GetFloat("x");
		vec.y = pReader->GetFloat("y");
		vec.z = pReader->GetFloat("z");
		vec.w = pReader->GetFloat("w");

		return Wrench::TOkValue<TVector4>(vec);
	}

	E_RESULT_CODE SaveVector4(IArchiveWriter* pWriter, const TVector4& object)
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

		if (std::fabs(object.w) > 1e-3f)
		{
			pWriter->SetFloat("w", object.w);
			anyWritten = true;
		}

		if (!anyWritten || (anyWritten && std::fabs(object.x) > 1e-3f))
		{
			pWriter->SetFloat("x", object.x); /// \note Always write at least x component to prevent the bug of serialization in the Yaml library
		}

		return RC_OK;
	}


	template <> TDE2_API E_RESULT_CODE Serialize<TVector4>(IArchiveWriter* pWriter, TVector4 value)
	{
		return pWriter->SetUInt32("type_id", static_cast<U32>(GetTypeId<TVector4>::mValue)) | SaveVector4(pWriter, value);
	}
}