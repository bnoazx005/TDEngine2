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
		const F32 halfYaw   = 0.5f * eulerAngles.z;
		const F32 halfRoll  = 0.5f * eulerAngles.x;
		const F32 halfPitch = 0.5f * eulerAngles.y;

		const F32 cosYaw   = cosf(halfYaw);
		const F32 sinYaw   = sinf(halfYaw);
		const F32 cosRoll  = cosf(halfRoll);
		const F32 sinRoll  = sinf(halfRoll);
		const F32 cosPitch = cosf(halfPitch);
		const F32 sinPitch = sinf(halfPitch);

		x = sinRoll * cosYaw * cosPitch + cosRoll * sinYaw * sinPitch;
		y = cosRoll * cosYaw * sinPitch - sinRoll * sinYaw * cosPitch;
		z = cosRoll * sinYaw * cosPitch + sinRoll * cosYaw * sinPitch;
		w = cosRoll * cosYaw * cosPitch + sinRoll * sinYaw * sinPitch;
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

	TDE2_API TVector4 operator* (const TQuaternion& q, const TVector4& v)
	{
		return RotationMatrix(q) * v;
	}

	TDE2_API bool operator== (const TQuaternion& lq, const TQuaternion& rq)
	{
		return	(CMathUtils::Abs(lq.x - rq.x) < FloatEpsilon) && 
				(CMathUtils::Abs(lq.y - rq.y) < FloatEpsilon) && 
				(CMathUtils::Abs(lq.z - rq.z) < FloatEpsilon) && 
				(CMathUtils::Abs(lq.w - rq.w) < FloatEpsilon);
	}

	TDE2_API bool operator!= (const TQuaternion& lq, const TQuaternion& rq)
	{
		return	(CMathUtils::Abs(lq.x - rq.x) > FloatEpsilon) ||
				(CMathUtils::Abs(lq.y - rq.y) > FloatEpsilon) ||
				(CMathUtils::Abs(lq.z - rq.z) > FloatEpsilon) ||
				(CMathUtils::Abs(lq.w - rq.w) > FloatEpsilon);
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
		t = CMathUtils::Clamp01(t);

		const F32 theta = (q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w);
		if (CMathUtils::IsGreatOrEqual(theta, 1.0f, FloatEpsilon))
		{
			return q1;
		}

		const F32 acosTheta = acosf(theta);
		const F32 sinTheta = sinf(acosTheta);

		return (sinf(acosTheta * (1.0f - t)) / sinTheta) * q1 + (sinf(acosTheta * t) / sinTheta) * q2;
	}

	TMatrix4 RotationMatrix(const TQuaternion& q)
	{
		const TQuaternion rot = Normalize(q);

		const F32 squaredX = rot.x * rot.x;
		const F32 squaredY = rot.y * rot.y;
		const F32 squaredZ = rot.z * rot.z;

		F32 elements[16] { 1.0f - 2.0f * (squaredY + squaredZ), 2.0f * (rot.x * rot.y - rot.w * rot.z), 2.0f * (rot.x * rot.z + rot.w * rot.y), 0.0f,
						   2.0f * (rot.x * rot.y + rot.w * rot.z), 1.0f - 2.0f * (squaredX + squaredZ), 2.0f * (rot.y * rot.z - rot.w * rot.x), 0.0f,
						   2.0f * (rot.x * rot.z - rot.w * rot.y), 2.0f * (rot.y * rot.z + rot.w * rot.x), 1.0f - 2.0f * (squaredX + squaredY), 0.0f,
						   0.0f, 0.0f, 0.0f, 1.0f };

		return TMatrix4(elements);
	}

	TVector3 ToEulerAngles(const TQuaternion& q)
	{
		const TMatrix4 m = RotationMatrix(q);

		const bool test = fabs(m.m[0][2]) < 0.99f;

		return TVector3(test ? atan2f(-m.m[1][2], m.m[2][2]) : atan2f(m.m[2][1], m.m[1][1]),
						asinf(CMathUtils::Clamp(-1.0f, 1.0f, m.m[0][2])),
						test ? atan2f(-m.m[0][1], m.m[0][0]) : 0.0f);
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

		bool anyWritten = false;

		if (std::fabs(object.y) > 1e-3f) { pWriter->SetFloat("y", object.y); anyWritten = true; }
		if (std::fabs(object.z) > 1e-3f) { pWriter->SetFloat("z", object.z); anyWritten = true; }
		if (std::fabs(object.w) > 1e-3f) { pWriter->SetFloat("w", object.w); anyWritten = true; }

		if (!anyWritten || (anyWritten && std::fabs(object.x) > 1e-3f))
		{
			pWriter->SetFloat("x", object.x);  /// \note Always write at least x component to prevent the bug of serialization in the Yaml library
		}

		return RC_OK;
	}
}