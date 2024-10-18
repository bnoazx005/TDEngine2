/*!
	\file TDEngine2VertexFormats.inc
	\author Kasimov Ildar
	\date 15.10.2024

	\brief The file describes all the formats that are used in the shaders that based on programming vertex pulling technique.
*/

#ifndef TDENGINE2_SHADOW_MAPPING_UTILS_INC
#define TDENGINE2_SHADOW_MAPPING_UTILS_INC

struct TVertexFormat
{
	float4 mPosition;

	#ifndef TDE2_FVF_POSITION_ONLY

	float4 mColor;
	float2 mUV;

	#ifdef TDE2_FVF_USE_NORMALS
	float4 mNormal;
	#endif

	#ifdef TDE2_FVF_USE_TANGENT
	float4 mTangent;
	#endif

	#endif
};


#ifdef TDE2_DECLARE_DEFAULT_VERTEX_BUFFER
	
DECLARE_TYPED_BUFFER_EX(TVertexFormat, VertexBuffer, 20);
#ifdef TDE2_ENABLE_INDEX_BUFFER
DECLARE_TYPED_BUFFER_EX(uint, IndexBuffer, 21);
#endif

/// \todo Add support of using this function when there is no declared index buffer's resource
float4 GetVertPos(uint index, uint vertexOffset = 0, uint indexOffset = 0) { return TYPED_BUFFER_ACCESS(VertexBuffer)[TYPED_BUFFER_ACCESS(IndexBuffer)[index + indexOffset] + vertexOffset].mPosition; }

#endif


#ifdef TDE2_USE_UI_VERTEX_FORMAT

struct TUiVertexFormat
{
	float4 mPosUV;
	#ifdef TDE2_USE_PACKED_COLOR
	uint   mColor;
	#else
	float4 mColor;
	#endif
};

DECLARE_TYPED_BUFFER_EX(TUiVertexFormat, VertexBuffer, 20);
#ifdef TDE2_ENABLE_INDEX_BUFFER
DECLARE_TYPED_BUFFER_EX(uint, IndexBuffer, 21);
#endif


float4 GetUiVertPosUv(uint index, uint vertexOffset = 0, uint indexOffset = 0) 
{ 
	return TYPED_BUFFER_ACCESS(VertexBuffer)[
#ifdef TDE2_ENABLE_INDEX_BUFFER
	TYPED_BUFFER_ACCESS(IndexBuffer)[index + indexOffset] + vertexOffset
#else
	index
#endif
	].mPosUV;
}

float4 GetUiVertColor(uint index, uint vertexOffset = 0, uint indexOffset = 0) 
{	
	#ifdef TDE2_USE_PACKED_COLOR
	uint color = TYPED_BUFFER_ACCESS(VertexBuffer)[
#ifdef TDE2_ENABLE_INDEX_BUFFER
	TYPED_BUFFER_ACCESS(IndexBuffer)[index + indexOffset] + vertexOffset
#else
	index
#endif
	].mColor; 

	float4 output;

	output.r = float(color & 0xff);
	output.g = float((color >> 8) & 0xff);
	output.b = float((color >> 16) & 0xff);
	output.a = float((color >> 24) & 0xff);

	return output / 255.0;
	#else
	return TYPED_BUFFER_ACCESS(VertexBuffer)[
#ifdef TDE2_ENABLE_INDEX_BUFFER
	TYPED_BUFFER_ACCESS(IndexBuffer)[index + indexOffset] + vertexOffset
#else 
	index
#endif
	].mColor; 
	#endif
}

#endif


#ifdef TDE2_USE_SPRITE_VERTEX_FORMAT


struct TSpriteVertexFormat
{
	float4 mPosition;
	float2 mUV;
};


struct TSpriteInstanceData
{
	float4x4 mTransformMat;
	float4   mColor;
};

DECLARE_TYPED_BUFFER_EX(TSpriteVertexFormat, VertexBuffer, 20);
#ifdef TDE2_ENABLE_INDEX_BUFFER
DECLARE_TYPED_BUFFER_EX(uint, IndexBuffer, 21);
#endif
DECLARE_TYPED_BUFFER_EX(TSpriteInstanceData, InstancesBuffer, 22);


float4 GetSpriteVertPos(uint index, uint vertexOffset = 0, uint indexOffset = 0) 
{ 
	return TYPED_BUFFER_ACCESS(VertexBuffer)[
#ifdef TDE2_ENABLE_INDEX_BUFFER
	TYPED_BUFFER_ACCESS(IndexBuffer)[index + indexOffset] + vertexOffset
#else
	index
#endif
	].mPosition;
}

float2 GetSpriteVertUv(uint index, uint vertexOffset = 0, uint indexOffset = 0) 
{ 
	return TYPED_BUFFER_ACCESS(VertexBuffer)[
#ifdef TDE2_ENABLE_INDEX_BUFFER
	TYPED_BUFFER_ACCESS(IndexBuffer)[index + indexOffset] + vertexOffset
#else
	index
#endif
	].mUV;
}

float4 GetSpriteVertColor(uint index) 
{	
	return TYPED_BUFFER_ACCESS(InstancesBuffer)[index].mColor; 
}

float4x4 GetSpriteTransform(uint index) 
{	
	return TYPED_BUFFER_ACCESS(InstancesBuffer)[index].mTransformMat; 
}

#endif


#endif