#define VERTEX_ENTRY mainVS
#define PIXEL_ENTRY mainPS
#define GEOMETRY_ENTRY mainGS

#include <TDEngine2Globals.inc>

struct VertexOutput
{
	float4 mPos : SV_POSITION;
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

#define TDE2_FVF_POSITION_ONLY
#define TDE2_DECLARE_DEFAULT_VERTEX_BUFFER
#define TDE2_ENABLE_INDEX_BUFFER
#include <TDEngine2VertexFormats.inc>

VertexOutput mainVS(uint vertexId : SV_VertexID)
{
	VertexOutput output;
	output.mPos = mul(ModelMat, GetVertPos(vertexId, StartVertexOffset, StartIndexOffset));
	
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
			output.mPos = mul(Lights[mPointLightIndex].mLightMats[faceIndex], gin[i].mPos);
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

	depth = length(input.mLPos.xyz - Lights[mPointLightIndex].mPosition.xyz) / Lights[mPointLightIndex].mRange;
}
#endprogram