/*!
	/file InternalShaderData.h
	/date 27.11.2018
	/authors Kasimov Ildar
*/

#pragma once


#include "../utils/Types.h"
#include "../math/TMatrix4.h"
#include "../math/TVector4.h"
#include "../utils/Color.h"


namespace TDEngine2
{
#pragma pack(push, 16)


	constexpr USIZE MaxLightsCount = 1024;
	constexpr U32 MaxShadowCascadesCount = 4;


	enum class E_LIGHT_SOURCE_TYPE : I32
	{
		POINT = 0,
		SPOT = 1,
	};


	typedef struct TLightData
	{
		TVector4  mPosition;
		TVector4  mDirection;
		TColor32F mColor;
		F32       mIntensity;
		F32       mRange;
		F32       mAngle;
		F32       mUnused;
		I32       mLightType = 0;
		I32       mUnused2[3] { 0, 0, 0 };
		TMatrix4  mLightMatrix[6]; /// For each side of a cubemap
	} TLightData, *TLightDataPtr;


	/*!
		\brief The structure contains all the information about light sources in the scene
	*/

	struct TLightingShaderData
	{
		TVector4  mSunLightDirection;
		TVector4  mSunLightPosition;
		TColor32F mSunLightColor;
		TVector4  mShadowCascadesSplits;
		TMatrix4  mSunLightMatrix[MaxShadowCascadesCount];

		U32       mActiveLightSourcesCount = 0;
		U32       mShadowCascadesCount = 0;
		U32       mIsShadowMappingEnabled = 1;
		U32       mUnused;
	};

	/*!
		struct TPerFrameShaderData

		\brief The structure contains shader uniform data that are updated each frame
	*/

	typedef struct TPerFrameShaderData
	{
		TMatrix4            mProjMatrix = IdentityMatrix4;
		TMatrix4            mViewMatrix = IdentityMatrix4;
		TMatrix4            mInvProjMatrix = IdentityMatrix4;
		TMatrix4            mInvViewMatrix = IdentityMatrix4;
		TMatrix4            mInvViewProjMatrix = IdentityMatrix4;

		TVector4            mCameraPosition = TVector4(ZeroVector3, 1.0f);
		TVector4            mCameraProjectionParams = TVector4(0.01f, 1000.0f, 0.0f, 0.0f);  /// Contains parameters of current active camera: x - near plane, y - far plane

		TVector4            mTime = TVector4(0.0f); ///< The vector contains time related values, where x is current game time, y is delta time 

		TLightingShaderData mLightingData;
	} TPerFrameShaderData, *TPerFrameShaderDataPtr;


	/*!
		struct TPerObjectShaderData

		\brief The structure contains shader uniform data that are changed per object
	*/

	typedef struct TPerObjectShaderData
	{
		TMatrix4 mModelMatrix;

		TMatrix4 mInvModelMatrix;
		
		U32      mObjectID;	///< The field is used by selection manager

		U32      mUnused[3];

		TVector4 mTextureTransformDesc; ///< The field contains information about main texture's rect to correctly compute uv space transformations
	} TPerObjectShaderData, *TPerObjectShaderDataPtr;


	/*!
		struct TRareUpdateShaderData

		\brief The structure contains shader uniform data that are rare updated
	*/

	typedef struct TRareUpdateShaderData
	{
		U32 mScreenWidth = 0;
		U32 mScreenHeight = 0;
		U32 mPadding[2];
	} TRareUpdateShaderData, *TRareUpdateShaderDataPtr;


	/*!
		struct TConstantShaderData

		\brief The structure contains shader uniform data that stays constant during the application's execution
	*/

	typedef struct TConstantShaderData
	{
		U32      mIsGPUParticlesEnabled = 0;
		U32      mUnused0[3];
		TMatrix4 mUnused;
	} TConstantShaderData, *TConstantShaderDataPtr;

#pragma pack(pop)
}