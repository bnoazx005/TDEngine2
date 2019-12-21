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


CBUFFER_SECTION(TDEngine2PerFrame)
	float4x4 ProjMat;
	float4x4 ViewMat;
CBUFFER_ENDSECTION


CBUFFER_SECTION(TDEngine2PerObject)
	float4x4 ModelMat;
CBUFFER_ENDSECTION


CBUFFER_SECTION(TDEngine2RareUpdate)
	float4x4 mUnused1;
CBUFFER_ENDSECTION


CBUFFER_SECTION(TDEngine2Constants)
	float4x4 mReserved;
CBUFFER_ENDSECTION


#endif