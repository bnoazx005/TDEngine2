#define VERTEX_ENTRY mainVS
#define PIXEL_ENTRY mainPS
#define GEOMETRY_ENTRY mainGS

#include <TDEngine2Globals.inc>

struct VertexOutput
{
	float4 mPos : SV_POSITION;
};


#program vertex

#include <TDEngine2SkinningUtils.inc>

struct VertexIn
{
	float4 mPos          : POSITION0;
	float4 mJointWeights : BLENDWEIGHT; 
	uint4  mJointIndices : BLENDINDICES;
};

float4 mainVS(in VertexIn input): SV_POSITION
{
	return mul(SunLightMat[0], mul(ModelMat, ComputeSkinnedVertexPos(input.mPos, input.mJointWeights, input.mJointIndices)));
}

#endprogram

#program pixel

void mainPS(VertexOutput input){}

#endprogram