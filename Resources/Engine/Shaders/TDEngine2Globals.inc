/*!
	\file TDEngine2Globals.inc
	\author Kasimov Ildar
	\date 03.12.2018

	\brief The file contains declarations of global available uniform variables,
	which are initialized and set up automatically by the engine
*/

#ifndef TDENGINE2GLOBALS_INC
#define TDENGINE2GLOBALS_INC


#include <TDEngine2Utils.inc>
#include <TDEngine2Lighting.inc>


CBUFFER_SECTION_EX(TDEngine2PerFrame, 0)
	float4x4 ProjMat;
	float4x4 ViewMat;
	float4x4 InvProjMat;
	float4x4 InvViewMat;
	float4x4 InvViewProjMat;

	float4   CameraPosition;

	float4   Time; ///< The vector contains time related values, where x is current game time, y is delta time

	float4   SunLightDirection;
	float4   SunLightPosition;
	float4   SunLightColor;
	float4   ShadowCascadesSplits;
	float4x4 SunLightMat[MAX_SHADOW_CASCADES_COUNT]; 

	int      ActivePointLightsCount; 
	int      ShadowCascadesCount;
	int      ShadowMapsEnabled;
	int      ActiveSpotLightsCount; 

	PointLightData PointLights[MAX_POINT_LIGHTS_COUNT];
	SpotLightData  SpotLights[MAX_SPOT_LIGHTS_COUNT];
CBUFFER_ENDSECTION


CBUFFER_SECTION_EX(TDEngine2PerObject, 1)
	float4x4 ModelMat;
	float4x4 InvModelMat;
	uint     ObjectID; // used for object picking
	float4   MainTexture_TransformDesc; /// x, y - origin of uv space; z, w - scale of u and v axes
CBUFFER_ENDSECTION


CBUFFER_SECTION_EX(TDEngine2RareUpdate, 2)
	float4x4 mUnused1;
CBUFFER_ENDSECTION


CBUFFER_SECTION_EX(TDEngine2Constants, 3)
	float4x4 mReserved;
CBUFFER_ENDSECTION


DECLARE_TYPED_BUFFER_EX(PointLightData, Lights, 10);


#endif