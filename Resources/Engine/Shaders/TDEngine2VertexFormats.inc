/*!
	\file TDEngine2VertexFormats.inc
	\author Kasimov Ildar
	\date 15.10.2024

	\brief The file describes all the formats that are used in the shaders that based on programming vertex pulling technique.
*/

#ifndef TDENGINE2_VERTEX_FORMATS_INC
#define TDENGINE2_VERTEX_FORMATS_INC


// \note All elements are stored in same order as E_VERTEX_STREAM_TYPE enumeration in IMesh.h file
#define HAS_POSITIONS (VertexFormatFlags & (1 << 0))   
#define HAS_COLORS    (VertexFormatFlags & (1 << 1))
#define HAS_TEXCOORDS (VertexFormatFlags & (1 << 2))   
#define HAS_NORMALS   (VertexFormatFlags & (1 << 3)) 
#define HAS_TANGENTS  (VertexFormatFlags & (1 << 4))  
#define HAS_SKINNING  (VertexFormatFlags & (1 << 5))  


#ifdef TDE2_FVF_SKINNED_VERTEX

struct TSkinningVertexData
{
	float4 mJointWeights; 
	uint4  mJointIndices;
};

#endif


#ifdef TDE2_DECLARE_DEFAULT_VERTEX_BUFFER
	
DECLARE_TYPED_BUFFER_EX(float4, PositionsVertexBuffer, 22);
DECLARE_TYPED_BUFFER_EX(float4, ColorsVertexBuffer, 23);
DECLARE_TYPED_BUFFER_EX(float4, TexCoordsVertexBuffer, 24);
DECLARE_TYPED_BUFFER_EX(float4, NormalsVertexBuffer, 25);
DECLARE_TYPED_BUFFER_EX(float4, TangentsVertexBuffer, 26);
#ifdef TDE2_FVF_SKINNED_VERTEX
DECLARE_TYPED_BUFFER_EX(TSkinningVertexData, SkinningVertexBuffer, 27);
#endif

#ifdef TDE2_ENABLE_INDEX_BUFFER
DECLARE_TYPED_BUFFER_EX(uint, IndexBuffer, 20);
#endif

/// \todo Add support of using this function when there is no declared index buffer's resource
float4 GetVertPos(uint index, uint vertexOffset = 0, uint indexOffset = 0) { return TYPED_BUFFER_ACCESS(PositionsVertexBuffer)[TYPED_BUFFER_ACCESS(IndexBuffer)[index + indexOffset] + vertexOffset]; }
float4 GetVertColor(uint index, uint vertexOffset = 0, uint indexOffset = 0) { return TYPED_BUFFER_ACCESS(ColorsVertexBuffer)[TYPED_BUFFER_ACCESS(IndexBuffer)[index + indexOffset] + vertexOffset]; }
float4 GetVertTexCoords(uint index, uint vertexOffset = 0, uint indexOffset = 0) { return TYPED_BUFFER_ACCESS(TexCoordsVertexBuffer)[TYPED_BUFFER_ACCESS(IndexBuffer)[index + indexOffset] + vertexOffset]; }
float4 GetVertNormal(uint index, uint vertexOffset = 0, uint indexOffset = 0) { return TYPED_BUFFER_ACCESS(NormalsVertexBuffer)[TYPED_BUFFER_ACCESS(IndexBuffer)[index + indexOffset] + vertexOffset]; }
float4 GetVertTangent(uint index, uint vertexOffset = 0, uint indexOffset = 0) { return TYPED_BUFFER_ACCESS(TangentsVertexBuffer)[TYPED_BUFFER_ACCESS(IndexBuffer)[index + indexOffset] + vertexOffset]; }

#ifdef TDE2_FVF_SKINNED_VERTEX
float4 GetVertJointWeights(uint index, uint vertexOffset = 0, uint indexOffset = 0) { return TYPED_BUFFER_ACCESS(SkinningVertexBuffer)[TYPED_BUFFER_ACCESS(IndexBuffer)[index + indexOffset] + vertexOffset].mJointWeights; }
uint4 GetVertJointIndices(uint index, uint vertexOffset = 0, uint indexOffset = 0) { return TYPED_BUFFER_ACCESS(SkinningVertexBuffer)[TYPED_BUFFER_ACCESS(IndexBuffer)[index + indexOffset] + vertexOffset].mJointIndices; }
#endif

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

DECLARE_TYPED_BUFFER_EX(TUiVertexFormat, VertexBuffer, 22);
#ifdef TDE2_ENABLE_INDEX_BUFFER
DECLARE_TYPED_BUFFER_EX(uint, IndexBuffer, 20);
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

DECLARE_TYPED_BUFFER_EX(TSpriteVertexFormat, VertexBuffer, 22);
#ifdef TDE2_ENABLE_INDEX_BUFFER
DECLARE_TYPED_BUFFER_EX(uint, IndexBuffer, 20);
#endif
DECLARE_TYPED_BUFFER_EX(TSpriteInstanceData, InstancesBuffer, 21);


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


#ifdef TDE2_USE_PARTICLE_VERTEX_FORMAT


struct TParticleVertexFormat
{
	float4 mPosition;
	float2 mUV;
};


struct TParticleInstanceData
{
	float4 mColor;
	float4 mPositionAndSize;
	float4 mRotation;
};

DECLARE_TYPED_BUFFER_EX(TParticleVertexFormat, VertexBuffer, 22);
#ifdef TDE2_ENABLE_INDEX_BUFFER
DECLARE_TYPED_BUFFER_EX(uint, IndexBuffer, 20);
#endif
DECLARE_TYPED_BUFFER_EX(TParticleInstanceData, InstancesBuffer, 21);


float4 GetParticleVertPos(uint index, uint vertexOffset = 0, uint indexOffset = 0) 
{ 
	return TYPED_BUFFER_ACCESS(VertexBuffer)[
#ifdef TDE2_ENABLE_INDEX_BUFFER
	TYPED_BUFFER_ACCESS(IndexBuffer)[index + indexOffset] + vertexOffset
#else
	index
#endif
	].mPosition;
}

float2 GetParticleVertUv(uint index, uint vertexOffset = 0, uint indexOffset = 0) 
{ 
	return TYPED_BUFFER_ACCESS(VertexBuffer)[
#ifdef TDE2_ENABLE_INDEX_BUFFER
	TYPED_BUFFER_ACCESS(IndexBuffer)[index + indexOffset] + vertexOffset
#else
	index
#endif
	].mUV;
}

float4 GetParticleVertColor(uint index) { return TYPED_BUFFER_ACCESS(InstancesBuffer)[index].mColor; }
float4 GetParticlePositionAndSize(uint index) { return TYPED_BUFFER_ACCESS(InstancesBuffer)[index].mPositionAndSize; }
float4 GetParticleRotation(uint index) { return TYPED_BUFFER_ACCESS(InstancesBuffer)[index].mRotation; }

#endif


#endif