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
#include <climits>


namespace TDEngine2
{
	typedef struct TParticle
	{
		TVector3  mPosition = ZeroVector3;
		TVector3  mVelocity = ZeroVector3;
		TVector4  mSize = TVector4(1.0f);
		TColor32F mColor = TColorUtils::mWhite;
		F32       mAge = (std::numeric_limits<F32>::max)();
		F32       mLifeTime = 1.0f;
		F32       mRotation = 0.0f;
		bool      mHasBeenUsed = false;
	} TParticleInfo, *TParticleInfoPtr;
}
