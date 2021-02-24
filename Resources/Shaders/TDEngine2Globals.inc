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
	float4   Time; ///< The vector contains time related values, where x is current game time, y is delta time

	float4   SunLightDirection;
	float4   SunLightPosition;
	float4   SunLightColor;
	float4x4 SunLightMat; 

	PointLightData PointLights[MAX_POINT_LIGHTS_COUNT];

	int      ActivePointLightsCount; // yzw aren't used
	int3     UnusedPadding;
CBUFFER_ENDSECTION


CBUFFER_SECTION_EX(TDEngine2PerObject, 1)
	float4x4 ModelMat;
	float4x4 InvModelMat;
	uint     ObjectID; // used for object picking
CBUFFER_ENDSECTION


CBUFFER_SECTION_EX(TDEngine2RareUpdate, 2)
	float4x4 mUnused1;
CBUFFER_ENDSECTION


CBUFFER_SECTION_EX(TDEngine2Constants, 3)
	float4x4 mReserved;
CBUFFER_ENDSECTION


#endif