#include "../../include/math/TPlane.h"
#include "../../include/math/TVector3.h"


namespace TDEngine2
{
	F32 CalcDistanceFromPlaneToPoint(const TPlaneF32& plane, const TVector3& point)
	{
		return Dot(Normalize(TVector3{ plane.a, plane.b, plane.c }), point) - plane.d;
	}
}