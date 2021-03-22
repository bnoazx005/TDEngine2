/*!
	/file TParticle.h
	/date 22.03.2021
	/authors Kasimov Ildar
*/

#pragma once


#include "../../utils/Types.h"
#include "../../utils/Color.h"
#include "../../math/TVector3.h"
#include "../../math/TVector4.h"


namespace TDEngine2
{
	typedef struct TParticle
	{
		TVector3  mPosition;
		TVector3  mVelocity;
		TVector4  mSize;
		TColor32F mColor;
		F32       mAge;
	} TParticleInfo, *TParticleInfoPtr;
}
