#include "./../../include/math/TQuaternion.h"
#include <math.h>


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
		F32 yaw   = eulerAngles.x;
		F32 roll  = eulerAngles.y;
		F32 pitch = eulerAngles.z;

		F32 cosYaw   = cosf(yaw * 0.5f);
		F32 sinYaw   = sinf(yaw * 0.5f);
		F32 cosRoll  = cosf(roll * 0.5f);
		F32 sinRoll  = sinf(roll * 0.5f);
		F32 cosPitch = cosf(pitch * 0.5f);
		F32 sinPitch = sinf(pitch * 0.5f);

		w = cosYaw * cosRoll * cosPitch + sinYaw * sinRoll * sinPitch;
		x = cosYaw * sinRoll * cosPitch - sinYaw * cosRoll * sinPitch;
		y = cosYaw * cosRoll * sinPitch + sinYaw * sinRoll * cosPitch;
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
}