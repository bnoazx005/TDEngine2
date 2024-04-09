#version 330 core

#include <TDEngine2Globals.inc>

#define VERTEX_ENTRY main
#define PIXEL_ENTRY main

#program vertex

layout (location = 0) in vec4 inPosUV;

out vec2 VertOutUV;

void main(void)
{
	gl_Position = vec4(inPosUV.xy, 0.0f, 1.0);
	VertOutUV   = inPosUV.zw;
}

#endprogram

#program pixel

in vec2 VertOutUV;

out vec4 FragColor;


DECLARE_TEX2D(FrameTexture); // depth buffer
DECLARE_TEX2D(FrameTexture1);

void main(void)
{    
    vec4 cloudCol = TEX2D(FrameTexture1, VertOutUV);

    if (TEX2D(FrameTexture, VertOutUV).r < 0.999f)
        discard;
    
    FragColor = cloudCol;
}


#endprogram