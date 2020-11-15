#include "./../../include/math/TQuaternion.h"
#include "./../../include/math/MathUtils.h"
#include <cmath>
#include <algorithm>


namespace TDEngine2
{
	TQuaternion::TQuaternion() :
		x(0.0f), y(0.0f), z(0.0f), w(0.0f)
	{
	}

	TQuaternion::TQuaternion(F32 x, F32 y, F32 z, F32 w) :
		x(x), y(y), z(z), w(w)
	{
	}
	
	TQuaternion::TQuaternion(const TVector3& v, F32 w) :
		x(v.x), y(v.y), z(v.z), w(w)
	{
	}

	TQuaternion::TQuaternion(const TVector3& eulerAngles)
	{
		F32 yaw   = eulerAngles.z;
		F32 roll  = eulerAngles.x;
		F32 pitch = eulerAngles.y;

		F32 cosYaw   = cosf(yaw * 0.5f);
		F32 sinYaw   = sinf(yaw * 0.5f);
		F32 cosRoll  = cosf(roll * 0.5f);
		F32 sinRoll  = sinf(roll * 0.5f);
		F32 cosPitch = cosf(pitch * 0.5f);
		F32 sinPitch = sinf(pitch * 0.5f);

		w = cosYaw * cosRoll * cosPitch + sinYaw * sinRoll * sinPitch;
		x = cosYaw * sinRoll * cosPitch + sinYaw * cosRoll * sinPitch;
		y = cosYaw * cosRoll * sinPitch - sinYaw * sinRoll * cosPitch;
		z = sinYaw * cosRoll * cosPitch - cosYaw * sinRoll * sinPitch;
	}

	TQuaternion::TQuaternion(const TQuaternion& q):
		x(q.x), y(q.y), z(q.z), w(q.w)
	{
	}

	TQuaternion::TQuaternion(TQuaternion&& q) :
		x(q.x), y(q.y), z(q.z), w(q.w)
	{
		q.x = 0.0f;
		q.y = 0.0f;
		q.z = 0.0f;
		q.w = 0.0f;
	}

	TQuaternion TQuaternion::operator= (const TQuaternion& q)
	{
		x = q.x;
		y = q.y;
		z = q.z;
		w = q.w;

		return *this;
	}

	TQuaternion& TQuaternion::operator= (TQuaternion&& q)
	{
		x = q.x;
		y = q.y;
		z = q.z;
		w = q.w;

		q.x = 0.0f;
		q.y = 0.0f;
		q.z = 0.0f;
		q.w = 0.0f;

		return *this;
	}


	TDE2_API TQuaternion operator+ (const TQuaternion& q1, const TQuaternion& q2)
	{
		return TQuaternion(q1.x + q2.x, q1.y + q2.y, q1.z + q2.z, q1.w + q2.w);
	}

	TDE2_API TQuaternion operator- (const TQuaternion& q1, const TQuaternion& q2)
	{
		return TQuaternion(q1.x - q2.x, q1.y - q2.y, q1.z - q2.z, q1.w - q2.w);
	}

	TDE2_API TQuaternion operator* (const TQuaternion& q1, const TQuaternion& q2)
	{
		F32 w = q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z;

		F32 x = q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y;
		F32 y = q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x;
		F32 z = q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w;

		return TQuaternion(x, y, z, w);
	}

	TDE2_API TQuaternion operator* (F32 scalar, const TQuaternion& q)
	{
		return TQuaternion(q.x * scalar, q.y * scalar, q.z * scalar, q.w * scalar);
	}

	TDE2_API TQuaternion operator* (const TQuaternion& q, F32 scalar)
	{
		return TQuaternion(q.x * scalar, q.y * scalar, q.z * scalar, q.w * scalar);
	}


	TDE2_API TQuaternion Conjugate(const TQuaternion& q)
	{
		return TQuaternion(-q.x, -q.y, -q.z, q.w);
	}
	
	TDE2_API F32 Length(const TQuaternion& q)
	{
		return sqrtf(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
	}

	TDE2_API TQuaternion Normalize(const TQuaternion& q)
	{
		F32 invLength = 1.0f / sqrtf(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);

		return TQuaternion(q.x * invLength, q.y * invLength, q.z * invLength, q.w * invLength);
	}

	TDE2_API TQuaternion Inverse(const TQuaternion& q)
	{
		F32 invNorm = 1.0f / (q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
		
		return TQuaternion(-q.x * invNorm, -q.y * invNorm, -q.z * invNorm, q.w * invNorm);
	}

	TDE2_API TQuaternion Lerp(const TQuaternion& q1, const TQuaternion& q2, F32 t)
	{
		t = (std::max)((std::min)(1.0f, t), 0.0f); // clamp t to range [0; 1]

		return (1 - t) * q1 + t * q2;
	}

	TDE2_API TQuaternion Slerp(const TQuaternion& q1, const TQuaternion& q2, F32 t)
	{
		t = (std::max)((std::min)(1.0f, t), 0.0f); // clamp t to range [0; 1]

		F32 theta = 1.0f / cosf(q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w);

		F32 sinTheta = sinf(theta);

		return (sinf(theta * (1.0f - t)) / sinTheta) * q1 + (sinf(theta * t) / sinTheta) * q2;
	}

	TMatrix4 RotationMatrix(const TQuaternion& q)
	{
		F32 squaredX = q.x * q.x;
		F32 squaredY = q.y * q.y;
		F32 squaredZ = q.z * q.z;

		F32 elements[16] { 1.0f - 2.0f * (squaredY + squaredZ), 2.0f * (q.x * q.y - q.w * q.z), 2.0f * (q.x * q.z + q.w * q.y), 0.0f,
						   2.0f * (q.x * q.y + q.w * q.z), 1.0f - 2.0f * (squaredX + squaredZ), 2.0f * (q.y * q.z - q.w * q.x), 0.0f,
						   2.0f * (q.x * q.z - q.w * q.y), 2.0f * (q.y * q.z + q.w * q.x), 1.0f - 2.0f * (squaredX + squaredY), 0.0f,
						   0.0f, 0.0f, 0.0f, 1.0f };

		return TMatrix4(elements);
	}

	TVector3 ToEulerAngles(const TQuaternion& q)
	{
		F32 sinp = 2.0f * (q.y * q.w - q.x * q.z);

		return TVector3(atan2f(2.0f * (q.x * q.w + q.y * q.z), 1.0f - 2.0f * (q.x * q.x + q.y * q.y)),
						CMathUtils::IsGreatOrEqual(sinp, 1.0f) ? std::copysign(CMathConstants::Pi * 0.5f, sinp) : asinf(sinp),
						atan2(2.0f * (q.z * q.w + q.y * q.x), 1.0f - 2.0f * (q.z * q.z + q.y * q.y)));
	}


	TResult<TQuaternion> LoadQuaternion(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}

		TQuaternion q;

		q.x = pReader->GetFloat("x");
		q.y = pReader->GetFloat("y");
		q.z = pReader->GetFloat("z");
		q.w = pReader->GetFloat("w");

		return Wrench::TOkValue<TQuaternion>(q);
	}

	E_RESULT_CODE SaveQuaternion(IArchiveWriter* pWriter, const TQuaternion& object)
	{
		if (!pWriter)
		{
			return RC_INVALID_ARGS;
		}

		pWriter->SetFloat("x", object.x);
		pWriter->SetFloat("y", object.y);
		pWriter->SetFloat("z", object.z);
		pWriter->SetFloat("w", object.w);

		return RC_OK;
	}
}