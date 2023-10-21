#define VERTEX_ENTRY mainVS
#define PIXEL_ENTRY mainPS
#define GEOMETRY_ENTRY mainGS

#include <TDEngine2Globals.inc>

struct VertexOutput
{
	float4 mPos : POSITION0;
};

struct GeometryOutput
{
	float4 mLPos : POSITION;
	float4 mPos : SV_POSITION;
	uint mFaceIndex : SV_RenderTargetArrayIndex;
};

CBUFFER_SECTION_EX(Parameters, 4)
	int mIsSunLight;
	int mPointLightIndex;
CBUFFER_ENDSECTION


#program vertex

VertexOutput mainVS(float4 lPos : POSITION0)
{
	VertexOutput output;
	output.mPos = mul(ModelMat, lPos);
	
	return output;
}

#endprogram

#program geometry

[maxvertexcount(18)]				
void mainGS(triangle VertexOutput gin[3], inout TriangleStream<GeometryOutput> triStream)
{
	GeometryOutput output;

	if (mIsSunLight == 1)
	{
		[unroll]
		for (int cascadeIndex = 0; cascadeIndex < ShadowCascadesCount; cascadeIndex++)
		{
			[unroll]
			for (int i = 0; i < 3; i++)
			{
				output.mPos = mul(SunLightMat[cascadeIndex], gin[i].mPos);
				output.mLPos = gin[i].mPos;
				output.mFaceIndex = cascadeIndex;
			
				triStream.Append(output);
			}

			triStream.RestartStrip();
		}

		return;
	}

	[unroll]
	for (int faceIndex = 0; faceIndex < 6; faceIndex++)
	{
		output.mFaceIndex = faceIndex;

		[unroll]
		for (int i = 0; i < 3; i++)
		{
			output.mPos = mul(PointLights[mPointLightIndex].mLightMats[faceIndex], gin[i].mPos);
			output.mLPos = gin[i].mPos;
			triStream.Append(output);
		}

		triStream.RestartStrip();
	}						
}

#endprogram

#program pixel

void mainPS(GeometryOutput input, out float depth : SV_Depth) 
{
	if (mIsSunLight == 1)
	{
		depth = input.mPos.z / input.mPos.w;
		return;
	}

	depth = length(input.mLPos.xyz - PointLights[mPointLightIndex].mPosition.xyz) / PointLights[mPointLightIndex].mRange;
}
#endprogram