#version 330 core

#include <TDEngine2Globals.inc>

#define VERTEX_ENTRY main
#define PIXEL_ENTRY main
#define GEOMETRY_ENTRY main


CBUFFER_SECTION_EX(Parameters, 4)
	int mIsSunLight;
	int mPointLightIndex;
CBUFFER_ENDSECTION


#program vertex

layout (location = 0) in vec4 inlPos;

void main(void)
{
	gl_Position = ModelMat * inlPos;
}

#endprogram

#program geometry

layout (triangles) in;		
layout (triangle_strip, max_vertices = 18) out;

out vec4 FragPos;

void main() 
{
	if (mIsSunLight == 1)
	{
		for (int cascadeIndex = 0; cascadeIndex < ShadowCascadesCount; cascadeIndex++)
		{
			gl_Layer = cascadeIndex;

			for (int i = 0; i < 3; i++)
			{
				gl_Position = mul(SunLightMat[cascadeIndex], gl_in[i].gl_Position);
				FragPos = gl_in[i].gl_Position;
			
				EmitVertex();
			}

			EndPrimitive();
		}

		return;
	}

	for (int faceIndex = 0; faceIndex < 6; faceIndex++)
	{
		gl_Layer = faceIndex;

		for (int i = 0; i < 3; i++)
		{
			gl_Position = mul(mLights[mPointLightIndex].mLightMats[faceIndex], gl_in[i].gl_Position);
			FragPos = gl_in[i].gl_Position;

			EmitVertex();
		}

		EndPrimitive();
	}	
}

#endprogram

#program pixel

in vec4 FragPos;

void main(void) 
{
	if (mIsSunLight == 1)
	{
		gl_FragDepth = gl_FragCoord.z;
		return;
	}

	gl_FragDepth = length(FragPos.xyz - mLights[mPointLightIndex].mPosition.xyz) / mLights[mPointLightIndex].mRange;
}

#endprogram