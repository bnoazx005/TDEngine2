#version 330 core

#include <TDEngine2Globals.inc>

#define VERTEX_ENTRY main
#define PIXEL_ENTRY main


#program vertex

layout (location = 0) in vec4 inPos;

out vec3 VertWorldPos;
out vec3 VertOutUV;

void main(void)
{
	mat4 viewMat = ViewMat;
	viewMat[3][0] = 0.0;
	viewMat[3][1] = 0.0;
	viewMat[3][2] = 0.0;

	vec4 pos = ProjMat * viewMat * inPos;
	VertWorldPos = inPos.xyz;

	gl_Position = pos.xyww;
	VertOutUV = inPos.xyz;
}

#endprogram


#program pixel

in vec3 VertWorldPos;
in vec3 VertOutUV;

out vec4 FragColor;


// Normalization factors from the precomputation phase.
static const float2 RAYLEIGH_NORM = float2(0.0, 0.05588319);
static const float2 MIE_NORM = float2(0.0, .02527083);

// Spectral irradiance and spectral to RGB conversion constants from
// the precomputation phase.
static const float3 SPECTRAL_IRRADIANCE = float3(1.526, 1.91, 2.08) / 10.0;
static const float3 SPECTRAL_TO_RGB = float3(133.3209, 88.51855, 112.7552);

static const float SUN_ANGULAR_RADIUS = 0.004675034;


DECLARE_TEX2D_EX(RayleighLUT, 0);
DECLARE_TEX2D_EX(MieLUT, 1);


float RayleighPhaseFunction(float cosTheta)
{
    // Original rayleigh phase function.
    // return 0.75 * (1 + pow(cosTheta, 2));

    // Modified to better account for sun-view azimuth as described in Section 4.1 of:
    // http://publications.lib.chalmers.se/records/fulltext/203057/203057.pdf
    return 0.8 * (1.4 + 0.5*cosTheta);
}

float MiePhaseFunction(float cosTheta, float g)
{
    float g2 = g * g;
    float t2 = cosTheta * cosTheta;
    float result = 3.0 / 2.0;
    result *= (1.0 - g2) / (2.0 + g2);
    result *= (1.0 + t2) / pow(1.0 + g2 - 2.0*g*t2, 3.0/2.0);
    return result;
}


CBUFFER_SECTION_EX(Parameters, 4)
    float mExposure;
    float mMieG;
CBUFFER_ENDSECTION


void main(void)
{
	vec3 viewDir = normalize(input.mWorldPos.xyz);

    // Calculate the view-zenith and sun-zenith angles.
    float cosV = dot(viewDir, vec3(0, -1, 0));
    float cosL = dot(SunLightDirection.xyz, vec3(0, 1, 0));

    // Convert the angles to texture coordinates using the parameterization function.
    // Note: we use abs+sign to avoid negative roots!
    float u = 0.5 * (1.0 + sign(cosV) * pow(abs(cosV), 1.0 / 3.0));
    float v = 0.5 * (1.0 + sign(cosL) * pow(abs(cosL), 1.0 / 3.0));

    // Sample the textures.
    vec3 rayleigh = TEX2D(RayleighLUT, vec2(u, v)).rgb;
    vec3 mie = TEX2D(MieLUT, vec2(u, v)).rgb;

    rayleigh = rayleigh * (RAYLEIGH_NORM.y - RAYLEIGH_NORM.x) + RAYLEIGH_NORM.x;
    mie = mie * (MIE_NORM.y - MIE_NORM.x) + MIE_NORM.x;

	// Calculate the view-sun angle for the phase function.
    // Note: we clamp it between [0, 1] or else we would get the sun
    // on both sides of the light direction.
    float cosTheta = dot(viewDir, SunLightDirection);
    cosTheta = saturate(cosTheta);

    // Apply the phase function.
    rayleigh *= RayleighPhaseFunction(cosTheta);
    mie *= MiePhaseFunction(cosTheta, mMieG);

    // Compute the scattering, and apply the spectral irradiance to
    // get the spectral radiance for this fragment.
    vec3 radiance = (rayleigh + mie) * SPECTRAL_IRRADIANCE * exposure;

    // Multiply by the SPECTRAL_TO_RGB conversion constants to convert
    // the spectral radiance to RGB values.
    vec3 rgb = radiance * SPECTRAL_TO_RGB;

    if (acos(cosTheta) < SUN_ANGULAR_RADIUS)
    {
        // TODO: this is not physically correct. It only works for exposure < 1.
        // Technically it should be multiplied by the transmittance.
        rgb /= SPECTRAL_IRRADIANCE * mExposure;
    }

	FragColor = GammaToLinear(float4(rgb, 1.0));
}

#endprogram
