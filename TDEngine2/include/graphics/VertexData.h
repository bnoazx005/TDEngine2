/*!
	\file VertexData.h
	\date 11.10.2019
	\authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Types.h"
#include "./../utils/Config.h"
#include "./../math/TVector4.h"
#include "./../math/TVector2.h"


namespace TDEngine2
{
	typedef struct TStandardVertex
	{
		TVector4 mPosition;
		TVector4 mNormal;
		TVector4 mTangent;
		TVector2 mUV0;
	} TStandardVertex, *TStandardVertexPtr;
}