// Based on work of Daniel Shervheim, 2019
// https://github.com/danielshervheim/atmosphere

#define VERTEX_ENTRY mainVS
#define PIXEL_ENTRY mainPS

#include <TDEngine2Globals.inc>


struct VertexOut
{
	float4 mPos : SV_POSITION;
	float4 mWorldPos : POSITION1;
	float3 mUV  : TEXCOORD0;
};


#program vertex

struct VertexIn
{
	float4 mPos : POSITION0;
	float4 mUV : TEXCOORD0;

};


VertexOut mainVS(in VertexIn input)
{
	VertexOut output;

	float4x4 transformedView = ViewMat;
	transformedView._14_34 = 0.0f;

	output.mWorldPos = input.mPos;
	output.mPos = mul(mul(ProjMat, transformedView), input.mPos).xyww;
	output.mUV  = input.mUV;

	return output;
}

#endprogram

#program pixel


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


float4 mainPS(VertexOut input): SV_TARGET0
{	
	float3 viewDir = normalize(input.mWorldPos.xyz);

    // Calculate the view-zenith and sun-zenith angles.
    float cosV = dot(viewDir, float3(0, -1, 0));
    float cosL = dot(SunLightDirection.xyz, float3(0, 1, 0));

    // Convert the angles to texture coordinates using the parameterization function.
    // Note: we use abs+sign to avoid negative roots!
    float u = 0.5 * (1.0 + sign(cosV) * pow(abs(cosV), 1.0 / 3.0));

#ifdef TDE2_HLSL_SHADER
    float v = 1.0 - 0.5 * (1.0 + sign(cosL) * pow(abs(cosL), 1.0 / 3.0));
#endif

#ifdef TDE2_GLSL_SHADER
    float v = 0.5 * (1.0 + sign(cosL) * pow(abs(cosL), 1.0 / 3.0));
#endif

    // Sample the textures.
    float3 rayleigh = TEX2D(RayleighLUT, float2(u, v)).rgb;
    float3 mie = TEX2D(MieLUT, float2(u, v)).rgb;

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
    float3 radiance = float3(0.0, 0.0, 0.0);
    radiance += rayleigh;
    radiance += mie;
    radiance *= SPECTRAL_IRRADIANCE * mExposure;

    // Multiply by the SPECTRAL_TO_RGB conversion constants to convert
    // the spectral radiance to RGB values.
    float3 rgb = radiance * SPECTRAL_TO_RGB;

    if (acos(cosTheta) < SUN_ANGULAR_RADIUS)
    {
        // TODO: this is not physically correct. It only works for exposure < 1.
        // Technically it should be multiplied by the transmittance.
        rgb /= SPECTRAL_IRRADIANCE * mExposure;
    }

    return GammaToLinear(float4(rgb, 1.0));
}

#endprogram
