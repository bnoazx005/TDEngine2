/*!
	\file TDEngine2MathUtils.inc
	\author Kasimov Ildar
	\date 12.01.2024
*/

#ifndef TDENGINE2MATHUTILS_INC
#define TDENGINE2MATHUTILS_INC


struct Ray3
{
	float3 origin;
	float3 dir;	
};


Ray3 MakeRay(float3 origin, float3 dir)
{
	Ray3 ray;

	ray.origin = origin;
	ray.dir = normalize(dir);

	return ray;
}


float IntersectRaySphere(Ray3 ray, float4 sphere)
{
	float3 l = ray.origin - sphere.xyz;

	float a = dot(ray.dir, ray.dir);
	float b = 2.0 * dot(ray.dir, l);
	float c = dot(l, l) - sphere.w * sphere.w;

	float discr = b * b - 4.0 * a * c;
	if (discr < 0.0) 
	{
		return -1.0;
	}

	float t0 = (-b - sqrt(discr)) / (2.0 * a);
	float t1 = (-b + sqrt(discr)) / (2.0 * a);

	if (t0 > 0)
	{
		return t0;
	}

	return t1;
}


float Remap(float a, float oldMin, float oldMax, float newMin, float newMax)
{
	return (((a - oldMin) / (oldMax - oldMin)) * (newMax - newMin)) + newMin;
};


float3 EaseInOut(float3 t)
{
	return ((6.0 * t - 15.0) * t + 10.0) * t * t * t;
}


float3 RandVec3(float3 p)
{
#ifdef TDE2_GLSL_SHADER

	vec3 p1 = fract(p * vec3(0.1031, 0.11369, 0.13787));
	p1 += dot(p1, p1.yxz + 19.19);

	return -1.0 + 2.0 * fract(vec3((p1.x + p1.y) * p1.z, (p1.x + p1.z) * p1.y, (p1.z + p1.y) * p1.x));

#else

	float3 p1 = frac(p * float3(0.1031, 0.11369, 0.13787));
	p1 += dot(p1, p1.yxz + 19.19);

	return -1.0 + 2.0 * frac(float3((p1.x + p1.y) * p1.z, (p1.x + p1.z) * p1.y, (p1.z + p1.y) * p1.x));

#endif
}


/*!
	Perlin 3D Noise implementation
*/


float CalcPerlinNoise3D(float3 p, float frequency)
{
	float3 gridPos = floor(p);
	float3 localPos = frac(p);

	float3 u = EaseInOut(localPos);

	// gradients
	float3 ga = RandVec3(fmod(gridPos + float3(0.0, 0.0, 0.0), frequency));
	float3 gb = RandVec3(fmod(gridPos + float3(1.0, 0.0, 0.0), frequency));
	float3 gc = RandVec3(fmod(gridPos + float3(0.0, 1.0, 0.0), frequency));
	float3 gd = RandVec3(fmod(gridPos + float3(1.0, 1.0, 0.0), frequency));
	float3 ge = RandVec3(fmod(gridPos + float3(0.0, 0.0, 1.0), frequency));
	float3 gf = RandVec3(fmod(gridPos + float3(1.0, 0.0, 1.0), frequency));
	float3 gg = RandVec3(fmod(gridPos + float3(0.0, 1.0, 1.0), frequency));
	float3 gh = RandVec3(fmod(gridPos + float3(1.0, 1.0, 1.0), frequency));

	// projections
	float va = dot(ga, localPos - float3(0.0, 0.0, 0.0));
	float vb = dot(gb, localPos - float3(1.0, 0.0, 0.0));
	float vc = dot(gc, localPos - float3(0.0, 1.0, 0.0));
	float vd = dot(gd, localPos - float3(1.0, 1.0, 0.0));
	float ve = dot(ge, localPos - float3(0.0, 0.0, 1.0));
	float vf = dot(gf, localPos - float3(1.0, 0.0, 1.0));
	float vg = dot(gg, localPos - float3(0.0, 1.0, 1.0));
	float vh = dot(gh, localPos - float3(1.0, 1.0, 1.0));

	return va + 
		   u.x * (vb - va) + 
		   u.y * (vc - va) + 
		   u.z * (ve - va) + 
		   u.x * u.y * (va - vb - vc + vd) + 
		   u.y * u.z * (va - vc - ve + vg) + 
		   u.z * u.x * (va - vb - ve + vf) + 
		   u.x * u.y * u.z * (-va + vb + vc - vd + ve - vf - vg + vh);
}


float CalcPerlinFbmNoise3D(float3 p, int octavesCount, float frequency)
{
	float amplitude = 1.0;
	float result = 0.0;

	for (int i = 0; i < octavesCount; i++)
	{
		result += amplitude * CalcPerlinNoise3D(p * frequency, frequency);

		amplitude *= 0.5;
		frequency *= 2.0;
	}

	return result;
}


/*!
	Worley 3D Noise implementation
*/

float CalcWorleyNoise3D(float3 p, float frequency)
{
	float3 gridPos = floor(p * frequency);
	float3 localPos = frac(p * frequency);

	float minDist = 1.0;

	for (float z = -1.0; z <= 1.0; z += 1.0)
	{
		for (float y = -1.0; y <= 1.0; y += 1.0)
		{
			for(float x = -1.0; x <= 1.0; x += 1.0)
			{
				float3 offset = float3(x, y, z);

				float3 rand = 0.5 * RandVec3(fmod(gridPos + offset, frequency)) + 0.5;
				float3 diff = rand + offset - localPos;

				minDist = min(minDist, dot(diff, diff));
			}
		}
	}

	return 1.0 - minDist;
}


float CalcWorleyFbmNoise3D(float3 p, int octavesCount, float frequency)
{
	float amplitude = 0.625;
	float result = 0.0;

	for (int i = 0; i < octavesCount; i++)
	{
		result += amplitude * CalcWorleyNoise3D(p, frequency);

		amplitude *= 0.4;
		frequency *= 2.0;
	}

	return result;
}


#ifdef ENABLE_MATH

struct Plane
{
	float3 normal;
	float dist;
};


struct TileFrustum
{
	Plane mPlanes[4];	// near and far planes are computed right before lights culling
};


struct Cone
{
	float3 T;   // Cone tip.
	float  h;   // Height of the cone.
	float3 d;   // Direction of the cone.
	float  r;   // bottom radius of the cone.
};


// Creates a plane based on three points and packs it into float4 variable where first three x, y, z form plane's normal, w is distance to origin 
Plane PackPlaneIntoFloat4(float3 p0, float3 p1, float3 p2)
{
	Plane result;

	float3 v0 = p1 - p0;
	float3 v2 = p2 - p0;
 
#ifdef TDE2_HLSL_SHADER // left handed
	result.normal = normalize(cross(v2, v0));
	result.dist = -dot(result.normal, p0);
#else // GL uses right handed
	result.normal = normalize(cross(v0, v2));
	result.dist = dot(result.normal, p0);
#endif
	
	return result;
}


// Convert clip space coordinates to view space
float4 ClipToView(float4 clipPos)
{
	float4 view = mul(InvProjMat, clipPos);
	view = view / view.w;
 
	return view;
}


float CalcDistanceFromPlaneToPoint(Plane plane, float3 p)
{
	return dot(plane.normal, p) - plane.dist;
}


bool SphereInsidePlane(float4 sphere, Plane plane)
{
	return CalcDistanceFromPlaneToPoint(plane, sphere.xyz) < -sphere.w;
}


bool SphereInsideFrustum(float4 sphere, TileFrustum frustum, float zNear, float zFar)
{
#ifdef TDE2_DEPTH_CLIP_ENABLED
	// First check depth
	// Note: Here, the view vector points in the -Z axis so the 
	// far depth value will be approaching -infinity.
	if (sphere.z - sphere.w > zFar || sphere.z + sphere.w < zNear) 
	{
		return false;
	}
#endif
 
	// Then check frustum planes
	for (int i = 0; i < 4; i++)
	{
		if (SphereInsidePlane(sphere, frustum.mPlanes[i]))
		{
			return false;
		}
	}
 
	return true;
}


bool PointInsidePlane(float3 p, Plane plane) 
{ 
	return CalcDistanceFromPlaneToPoint(plane, p) < 0.0; 
}


bool ConeInsidePlane(Cone cone, Plane plane)
{
	// Compute the farthest point on the end of the cone to the positive space of the plane.
	float3 Q = cone.T + cone.d * cone.h - cross(cross(plane.normal, cone.d), cone.d) * cone.r;

	// The cone is in the negative halfspace of the plane if both
	// the tip of the cone and the farthest point on the end of the cone to the 
	// positive halfspace of the plane are both inside the negative halfspace 
	// of the plane.
	return PointInsidePlane(cone.T, plane) && PointInsidePlane(Q, plane);
}


bool ConeInsideFrustum(Cone cone, TileFrustum frustum, float zNear, float zFar)
{
#ifdef TDE2_DEPTH_CLIP_ENABLED
	Plane nearPlane = { float3(0.0, 0.0, 1.0), zNear };
	Plane farPlane = { float3(0.0, 0.0, -1.0), -zFar };

	// First check the near and far clipping planes.
	if (ConeInsidePlane(cone, nearPlane) || ConeInsidePlane(cone, farPlane))
	{
		return false;
	}
#endif

	// Then check frustum planes
	for (int i = 0; i < 4; i++)
	{
		if (ConeInsidePlane(cone, frustum.mPlanes[i]))
		{
			return false;
		}
	}

	return true;
}

#endif


#endif