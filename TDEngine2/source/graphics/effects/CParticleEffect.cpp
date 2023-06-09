#include "../../../include/graphics/effects/CParticleEffect.h"
#include "../../../include/core/IFileSystem.h"
#include "../../../include/core/IFile.h"
#include "../../../include/utils/CFileLogger.h"
#include "../../../include/core/IGraphicsContext.h"
#define META_EXPORT_GRAPHICS_SECTION
#include "../../../include/metadata.h"


namespace TDEngine2
{
	struct TParticleEffectClipKeys
	{
		static const std::string mDurationKeyId;
		static const std::string mLoopModeKeyId;
		static const std::string mMaxParticlesCountKeyId;
		static const std::string mMaterialNameKeyId;

		static const std::string mParticlesSettingsGroupId;
		static const std::string mLifeTimeKeyId;
		static const std::string mInitialSizeKeyId;
		static const std::string mInitialRotationKeyId;
		static const std::string mInitialColorKeyId;
		static const std::string mInitialMoveDirectionKeyId;
		static const std::string mInitialSpeedFactorKeyId;
		static const std::string mGravityModifierKeyId;

		static const std::string mSizeOverTimeKeyId;
		static const std::string mColorOverTimeKeyId;
		static const std::string mVelocityOverTimeKeyId;
		static const std::string mRotationOverTimeKeyId;
		static const std::string mForceOverTimeKeyId;

		static const std::string mEmitterDataGroupId;
		static const std::string mEmissionRateKeyId;
		static const std::string mSimulationSpaceTypeKeyId;

		static const std::string mModifiersFlagsKeyId;


		struct TInitialColorKeys
		{
			static const std::string mTypeKeyId;
			static const std::string mColorKeyId;
			static const std::string mAddColorKeyId;
		};


		struct TVelocityKeys
		{
			static const std::string mParamTypesKeyId;
			static const std::string mXCurveKeyId;
			static const std::string mYCurveKeyId;
			static const std::string mZCurveKeyId;
			static const std::string mSpeedCurveKeyId;
			static const std::string mVelocityConstKeyId;
			static const std::string mSpeedConstKeyId;
		};
	};

	const std::string TParticleEffectClipKeys::mDurationKeyId = "duration";
	const std::string TParticleEffectClipKeys::mLoopModeKeyId = "loop-mode";
	const std::string TParticleEffectClipKeys::mMaxParticlesCountKeyId = "max-particles-count";
	const std::string TParticleEffectClipKeys::mMaterialNameKeyId = "material-id";
	const std::string TParticleEffectClipKeys::mParticlesSettingsGroupId = "particles-settings";
	const std::string TParticleEffectClipKeys::mLifeTimeKeyId = "lifetime";
	const std::string TParticleEffectClipKeys::mInitialSizeKeyId = "initial-size";
	const std::string TParticleEffectClipKeys::mInitialRotationKeyId = "initial-rotation";
	const std::string TParticleEffectClipKeys::mInitialColorKeyId = "initial-color";
	const std::string TParticleEffectClipKeys::mInitialMoveDirectionKeyId = "initial-move-dir";
	const std::string TParticleEffectClipKeys::mInitialSpeedFactorKeyId = "initial-speed";
	const std::string TParticleEffectClipKeys::mGravityModifierKeyId = "gravity-modifier";
	const std::string TParticleEffectClipKeys::mSizeOverTimeKeyId = "size-over-time";
	const std::string TParticleEffectClipKeys::mColorOverTimeKeyId = "color-over-time";
	const std::string TParticleEffectClipKeys::mVelocityOverTimeKeyId = "velocity-over-time";
	const std::string TParticleEffectClipKeys::mRotationOverTimeKeyId = "rotation-over-time";
	const std::string TParticleEffectClipKeys::mForceOverTimeKeyId = "force-over-time";
	const std::string TParticleEffectClipKeys::mEmitterDataGroupId = "emitter-params";
	const std::string TParticleEffectClipKeys::mEmissionRateKeyId = "emission-rate";
	const std::string TParticleEffectClipKeys::mSimulationSpaceTypeKeyId = "simulation-space";
	const std::string TParticleEffectClipKeys::mModifiersFlagsKeyId = "modifiers-flags";
	
	const std::string TParticleEffectClipKeys::TInitialColorKeys::mTypeKeyId = "type";
	const std::string TParticleEffectClipKeys::TInitialColorKeys::mColorKeyId = "value0";
	const std::string TParticleEffectClipKeys::TInitialColorKeys::mAddColorKeyId = "value1";

	const std::string TParticleEffectClipKeys::TVelocityKeys::mParamTypesKeyId = "type";
	const std::string TParticleEffectClipKeys::TVelocityKeys::mXCurveKeyId = "x-vel-curve";
	const std::string TParticleEffectClipKeys::TVelocityKeys::mYCurveKeyId = "y-vel-curve";
	const std::string TParticleEffectClipKeys::TVelocityKeys::mZCurveKeyId = "z-vel-curve";
	const std::string TParticleEffectClipKeys::TVelocityKeys::mSpeedCurveKeyId = "speed-curve";
	const std::string TParticleEffectClipKeys::TVelocityKeys::mVelocityConstKeyId = "vel-const";
	const std::string TParticleEffectClipKeys::TVelocityKeys::mSpeedConstKeyId = "speed-const";


	static E_RESULT_CODE InitColorData(TParticleColorParameter& colorData)
	{
		/// \note Create a new object of gradient color if it's selected but doesn't exist yet
		if ((E_PARTICLE_COLOR_PARAMETER_TYPE::GRADIENT_LERP != colorData.mType || E_PARTICLE_COLOR_PARAMETER_TYPE::GRADIENT_LERP != colorData.mType) || colorData.mGradientColor)
		{
			return RC_OK;
		}

		E_RESULT_CODE result = RC_OK;

		colorData.mGradientColor = CreateGradientColor(TColorUtils::mWhite, TColorUtils::mWhite, result);

		return result;
	}


	static E_RESULT_CODE InitVelocityData(TParticleVelocityParameter& velocityData)
	{
		E_RESULT_CODE result = RC_OK;

		static const TRectF32 velocityCurve{ -1.0f, -1.0f, 2.0f, 2.0f };
		static const TRectF32 speedCurve{ 0.0f, 0.0f, 1.0f, 1.0f };

		// \note Create a new curves if the given are empty
		if (!velocityData.mXCurve)
		{
			velocityData.mXCurve = CreateAnimationCurve(velocityCurve, result);
		}

		if (!velocityData.mYCurve)
		{
			velocityData.mYCurve = CreateAnimationCurve(velocityCurve, result);
		}

		if (!velocityData.mZCurve)
		{
			velocityData.mZCurve = CreateAnimationCurve(velocityCurve, result);
		}

		if (!velocityData.mSpeedFactorCurve)
		{
			velocityData.mSpeedFactorCurve = CreateAnimationCurve(speedCurve, result);
		}

		return result;
	}


	CParticleEffect::CParticleEffect() :
		CBaseResource()
	{
	}

	E_RESULT_CODE CParticleEffect::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name)
	{
		E_RESULT_CODE result = _init(pResourceManager, name);

		if (result != RC_OK)
		{
			return result;
		}

		if (!pGraphicsContext)
		{
			return RC_INVALID_ARGS;
		}

		mState = E_RESOURCE_STATE_TYPE::RST_LOADED;

		mModifiersInfoFlags = E_PARTICLE_EFFECT_INFO_FLAGS::DEFAULT;

		mInitialColor = { E_PARTICLE_COLOR_PARAMETER_TYPE::SINGLE_COLOR, TColorUtils::mWhite, TColorUtils::mWhite };

		mpSharedEmitter = nullptr;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CParticleEffect::Reset()
	{
		return RC_NOT_IMPLEMENTED_YET;
	}

	E_RESULT_CODE CParticleEffect::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}

		mDuration = pReader->GetFloat(TParticleEffectClipKeys::mDurationKeyId);
		mIsLooped = pReader->GetBool(TParticleEffectClipKeys::mLoopModeKeyId);
		mMaxParticlesCount = pReader->GetUInt16(TParticleEffectClipKeys::mMaxParticlesCountKeyId);
		mMaterialName = pReader->GetString(TParticleEffectClipKeys::mMaterialNameKeyId);
		mSimulationSpaceType = Meta::EnumTrait<E_PARTICLE_SIMULATION_SPACE>::FromString(pReader->GetString(TParticleEffectClipKeys::mSimulationSpaceTypeKeyId));

		pReader->BeginGroup(TParticleEffectClipKeys::mParticlesSettingsGroupId);
		{
			pReader->BeginGroup(TParticleEffectClipKeys::mLifeTimeKeyId);
			{
				mLifeTime.mLeft = pReader->GetFloat("min");
				mLifeTime.mRight = pReader->GetFloat("max");
			}
			pReader->EndGroup();

			pReader->BeginGroup(TParticleEffectClipKeys::mInitialRotationKeyId);
			{
				mInitialRotation.mLeft = pReader->GetFloat("min");
				mInitialRotation.mRight = pReader->GetFloat("max");
			}
			pReader->EndGroup();

			pReader->BeginGroup(TParticleEffectClipKeys::mInitialSizeKeyId);
			{
				mInitialSize.mLeft = pReader->GetFloat("min");
				mInitialSize.mRight = pReader->GetFloat("max");
			}
			pReader->EndGroup();

			pReader->BeginGroup(TParticleEffectClipKeys::mInitialColorKeyId);
			{
				auto loadInitColorDataResult = _loadColorData(pReader);
				if (loadInitColorDataResult.HasError())
				{
					return loadInitColorDataResult.GetError();
				}

				mInitialColor = loadInitColorDataResult.Get();
			}
			pReader->EndGroup();

			pReader->BeginGroup(TParticleEffectClipKeys::mInitialMoveDirectionKeyId);
			{
				auto loadVecResult = LoadVector3(pReader);
				if (loadVecResult.HasError())
				{
					return loadVecResult.GetError();
				}

				mInitialMoveDirection = loadVecResult.Get();
			}
			pReader->EndGroup();

			mInitialSpeedFactor = pReader->GetFloat(TParticleEffectClipKeys::mInitialSpeedFactorKeyId);
			mGravityModifier = pReader->GetFloat(TParticleEffectClipKeys::mGravityModifierKeyId);

			/// \note Read dynamic parameters
			E_RESULT_CODE result = RC_OK;

			pReader->BeginGroup(TParticleEffectClipKeys::mSizeOverTimeKeyId);
			{
				mpSizeCurve = CreateAnimationCurve({ 0.0f, 0.0f, 1.0f, 1.0f }, result);

				if ((RC_OK != result) || (RC_OK != (result = mpSizeCurve->Load(pReader))))
				{
					return result;
				}
			}
			pReader->EndGroup();

			pReader->BeginGroup(TParticleEffectClipKeys::mColorOverTimeKeyId);
			{
				auto loadColorDataResult = _loadColorData(pReader);
				if (loadColorDataResult.HasError())
				{
					return loadColorDataResult.GetError();
				}

				mColorOverLifetimeData = loadColorDataResult.Get();
			}
			pReader->EndGroup();

			pReader->BeginGroup(TParticleEffectClipKeys::mVelocityOverTimeKeyId);
			{
				auto loadVelocityDataResult = _loadVelocityData(pReader);
				if (loadVelocityDataResult.HasError())
				{
					return loadVelocityDataResult.GetError();
				}

				mVelocityOverLifetimeData = loadVelocityDataResult.Get();
			}
			pReader->EndGroup();

			mRotationPerFrame = pReader->GetFloat(TParticleEffectClipKeys::mRotationOverTimeKeyId);

			pReader->BeginGroup(TParticleEffectClipKeys::mForceOverTimeKeyId);
			{
				auto loadVecResult = LoadVector3(pReader);
				if (loadVecResult.HasError())
				{
					return loadVecResult.GetError();
				}

				mForcePerFrame = loadVecResult.Get();
			}
			pReader->EndGroup();
		}
		pReader->EndGroup();

		pReader->BeginGroup(TParticleEffectClipKeys::mEmitterDataGroupId);
		{
			auto loadEmitterResult = CBaseParticlesEmitter::Deserialize(pReader);
			if (loadEmitterResult.HasError())
			{
				return loadEmitterResult.GetError();
			}

			mpSharedEmitter = loadEmitterResult.Get();
			if (mpSharedEmitter)
			{
				mpSharedEmitter->SetOwnerEffect(this);
			}

			mEmissionRate = pReader->GetUInt32(TParticleEffectClipKeys::mEmissionRateKeyId);
		}
		pReader->EndGroup();

		mModifiersInfoFlags = static_cast<E_PARTICLE_EFFECT_INFO_FLAGS>(pReader->GetUInt32(TParticleEffectClipKeys::mModifiersFlagsKeyId));

		return RC_OK;
	}

	E_RESULT_CODE CParticleEffect::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		pWriter->BeginGroup("meta");
		{
			pWriter->SetString("resource-type", "particle-effect");
			pWriter->SetUInt32("version-tag", mAssetsVersionTag);
		}
		pWriter->EndGroup();

		pWriter->SetFloat(TParticleEffectClipKeys::mDurationKeyId, mDuration);
		pWriter->SetBool(TParticleEffectClipKeys::mLoopModeKeyId, mIsLooped);
		pWriter->SetUInt16(TParticleEffectClipKeys::mMaxParticlesCountKeyId, mMaxParticlesCount);
		pWriter->SetString(TParticleEffectClipKeys::mMaterialNameKeyId, mMaterialName);
		pWriter->SetString(TParticleEffectClipKeys::mSimulationSpaceTypeKeyId, Meta::EnumTrait<E_PARTICLE_SIMULATION_SPACE>::ToString(mSimulationSpaceType));

		pWriter->BeginGroup(TParticleEffectClipKeys::mParticlesSettingsGroupId);
		{
			pWriter->BeginGroup(TParticleEffectClipKeys::mLifeTimeKeyId);
			{
				pWriter->SetFloat("min", mLifeTime.mLeft);
				pWriter->SetFloat("max", mLifeTime.mRight);
			}
			pWriter->EndGroup();

			pWriter->BeginGroup(TParticleEffectClipKeys::mInitialRotationKeyId);
			{
				pWriter->SetFloat("min", mInitialRotation.mLeft);
				pWriter->SetFloat("max", mInitialRotation.mRight);
			}
			pWriter->EndGroup();

			pWriter->BeginGroup(TParticleEffectClipKeys::mInitialSizeKeyId);
			{
				pWriter->SetFloat("min", mInitialSize.mLeft);
				pWriter->SetFloat("max", mInitialSize.mRight);
			}
			pWriter->EndGroup();

			pWriter->BeginGroup(TParticleEffectClipKeys::mInitialColorKeyId);
			{
				_saveColorData(pWriter, mInitialColor);
			}
			pWriter->EndGroup();

			pWriter->BeginGroup(TParticleEffectClipKeys::mInitialMoveDirectionKeyId);
			{
				SaveVector3(pWriter, mInitialMoveDirection);
			}
			pWriter->EndGroup();

			pWriter->SetFloat(TParticleEffectClipKeys::mInitialSpeedFactorKeyId, mInitialSpeedFactor);
			pWriter->SetFloat(TParticleEffectClipKeys::mGravityModifierKeyId, mGravityModifier);

			/// \note Dynamic parameters
			pWriter->BeginGroup(TParticleEffectClipKeys::mSizeOverTimeKeyId);
			{
				mpSizeCurve->Save(pWriter);
			}
			pWriter->EndGroup();

			pWriter->BeginGroup(TParticleEffectClipKeys::mColorOverTimeKeyId);
			{
				_saveColorData(pWriter, mColorOverLifetimeData);
			}
			pWriter->EndGroup();

			pWriter->BeginGroup(TParticleEffectClipKeys::mVelocityOverTimeKeyId);
			{
				_saveVelocityData(pWriter, mVelocityOverLifetimeData);
			}
			pWriter->EndGroup();

			pWriter->SetFloat(TParticleEffectClipKeys::mRotationOverTimeKeyId, mRotationPerFrame);

			pWriter->BeginGroup(TParticleEffectClipKeys::mForceOverTimeKeyId);
			{
				SaveVector3(pWriter, mForcePerFrame);
			}
			pWriter->EndGroup();
		}
		pWriter->EndGroup();

		pWriter->BeginGroup(TParticleEffectClipKeys::mEmitterDataGroupId);
		{
			mpSharedEmitter->Save(pWriter);

			pWriter->SetUInt32(TParticleEffectClipKeys::mEmissionRateKeyId, mEmissionRate);
		}
		pWriter->EndGroup();

		pWriter->SetUInt32(TParticleEffectClipKeys::mModifiersFlagsKeyId, static_cast<U32>(mModifiersInfoFlags));

		return RC_OK;
	}

	E_RESULT_CODE CParticleEffect::SetDuration(F32 duration)
	{
		if (duration < 0.0f)
		{
			return RC_INVALID_ARGS;
		}

		mDuration = duration;

		return RC_OK;
	}

	void CParticleEffect::SetLoopMode(bool value)
	{
		mIsLooped = value;
	}

	void CParticleEffect::SetMaxParticlesCount(U16 particlesCount)
	{
		mMaxParticlesCount = particlesCount;
	}

	E_RESULT_CODE CParticleEffect::SetMaterialName(const std::string& value)
	{
		if (value.empty())
		{
			return RC_INVALID_ARGS;
		}

		mMaterialName = value;

		return RC_OK;
	}

	E_RESULT_CODE CParticleEffect::SetLifetime(const TRangeF32& value)
	{
		mLifeTime = value;

		return RC_OK;
	}

	E_RESULT_CODE CParticleEffect::SetInitialSize(const TRangeF32& value)
	{
		mInitialSize = value;

		return RC_OK;
	}

	void CParticleEffect::SetInitialRotation(const TRangeF32& value)
	{
		mInitialRotation = value;
	}

	void CParticleEffect::SetInitialColor(const TParticleColorParameter& colorData)
	{
		mInitialColor = colorData;
		InitColorData(mInitialColor);
	}

	void CParticleEffect::SetInitialVelocityData(const TVector3& direction, F32 speedFactor)
	{
		mInitialMoveDirection = direction;
		mInitialSpeedFactor = speedFactor;
	}

	void CParticleEffect::SetGravityModifier(F32 value)
	{
		mGravityModifier = value;
	}

	void CParticleEffect::SetEmissionRate(U32 value)
	{
		mEmissionRate = value;
	}

	void CParticleEffect::SetSimulationSpaceType(E_PARTICLE_SIMULATION_SPACE spaceType)
	{
		mSimulationSpaceType = spaceType;
	}

	E_RESULT_CODE CParticleEffect::SetSharedEmitter(const CScopedPtr<CBaseParticlesEmitter>& pEmitter)
	{
		if (!pEmitter)
		{
			return RC_INVALID_ARGS;
		}

		mpSharedEmitter = pEmitter;

		return RC_OK;
	}

	E_RESULT_CODE CParticleEffect::SetSizeCurve(const CScopedPtr<CAnimationCurve>& pCurve)
	{
		if (!pCurve)
		{
			return RC_INVALID_ARGS;
		}

		mpSizeCurve = pCurve;

		return RC_OK;
	}

	E_RESULT_CODE CParticleEffect::SetColorOverLifeTime(const TParticleColorParameter& colorData)
	{
		mColorOverLifetimeData = colorData;
		return InitColorData(mColorOverLifetimeData);
	}

	E_RESULT_CODE CParticleEffect::SetVelocityOverTime(const TParticleVelocityParameter& velocityData)
	{
		mVelocityOverLifetimeData = velocityData;
		return InitVelocityData(mVelocityOverLifetimeData);
	}

	E_RESULT_CODE CParticleEffect::SetRotationOverTime(F32 angle)
	{
		mRotationPerFrame = angle;
		return RC_OK;
	}

	void CParticleEffect::SetForceOverTime(const TVector3& force)
	{
		mForcePerFrame = force;
	}

	E_RESULT_CODE CParticleEffect::SetModifiersFlags(E_PARTICLE_EFFECT_INFO_FLAGS value)
	{
		mModifiersInfoFlags = value;
		return RC_OK;
	}

	F32 CParticleEffect::GetDuration() const
	{
		return mDuration;
	}

	bool CParticleEffect::IsLoopModeActive() const
	{
		return mIsLooped;
	}

	U16 CParticleEffect::GetMaxParticlesCount() const
	{
		return mMaxParticlesCount;
	}

	const std::string& CParticleEffect::GetMaterialName() const
	{
		return mMaterialName;
	}

	const TRangeF32& CParticleEffect::GetLifetime() const
	{
		return mLifeTime;
	}

	const TRangeF32& CParticleEffect::GetInitialSize() const
	{
		return mInitialSize;
	}

	const TRangeF32& CParticleEffect::GetInitialRotation() const
	{
		return mInitialRotation;
	}

	const TParticleColorParameter& CParticleEffect::GetInitialColor() const
	{
		return mInitialColor;
	}

	const TVector3& CParticleEffect::GetInitialMoveDirection() const
	{
		return mInitialMoveDirection;
	}

	F32 CParticleEffect::GetInitialSpeedFactor() const
	{
		return mInitialSpeedFactor;
	}

	F32 CParticleEffect::GetGravityModifier() const
	{
		return mGravityModifier;
	}

	U32 CParticleEffect::GetEmissionRate() const
	{
		return mEmissionRate;
	}

	E_PARTICLE_SIMULATION_SPACE CParticleEffect::GetSimulationSpaceType() const
	{
		return mSimulationSpaceType;
	}

	CScopedPtr<CBaseParticlesEmitter> CParticleEffect::GetSharedEmitter() const
	{
		return mpSharedEmitter;
	}

	CScopedPtr<CAnimationCurve> CParticleEffect::GetSizeCurve() const
	{
		return mpSizeCurve;
	}

	const TParticleColorParameter& CParticleEffect::GetColorOverLifeTime() const
	{
		return mColorOverLifetimeData;
	}

	const TParticleVelocityParameter& CParticleEffect::GetVelocityOverTime() const
	{
		return mVelocityOverLifetimeData;
	}

	F32 CParticleEffect::GetRotationOverTime() const
	{
		return mRotationPerFrame;
	}

	const TVector3& CParticleEffect::GetForceOverTime() const
	{
		return mForcePerFrame;
	}

	E_PARTICLE_EFFECT_INFO_FLAGS CParticleEffect::GetEnabledModifiersFlags() const
	{
		return mModifiersInfoFlags;
	}

	E_RESULT_CODE CParticleEffect::_saveColorData(IArchiveWriter* pWriter, const TParticleColorParameter& colorData)
	{
		E_RESULT_CODE result = pWriter->SetUInt16(TParticleEffectClipKeys::TInitialColorKeys::mTypeKeyId, static_cast<U16>(colorData.mType));

		switch (colorData.mType)
		{
			case E_PARTICLE_COLOR_PARAMETER_TYPE::SINGLE_COLOR:
				
				result = result | pWriter->BeginGroup(TParticleEffectClipKeys::TInitialColorKeys::mColorKeyId);
				{
					result = result | SaveColor32F(pWriter, colorData.mFirstColor);
				}
				result = result | pWriter->EndGroup();
				
				break;

			case E_PARTICLE_COLOR_PARAMETER_TYPE::TWEEN_RANDOM:

				result = result | pWriter->BeginGroup(TParticleEffectClipKeys::TInitialColorKeys::mColorKeyId);
				{
					result = result | SaveColor32F(pWriter, colorData.mFirstColor);
				}
				result = result | pWriter->EndGroup();

				result = result | pWriter->BeginGroup(TParticleEffectClipKeys::TInitialColorKeys::mAddColorKeyId);
				{
					result = result | SaveColor32F(pWriter, colorData.mSecondColor);
				}
				result = result | pWriter->EndGroup();

				break;

			case E_PARTICLE_COLOR_PARAMETER_TYPE::GRADIENT_LERP:
			case E_PARTICLE_COLOR_PARAMETER_TYPE::GRADIENT_RANDOM:

				result = result | pWriter->BeginGroup(TParticleEffectClipKeys::TInitialColorKeys::mColorKeyId);
				{
					if (colorData.mGradientColor)
					{
						result = result | colorData.mGradientColor->Save(pWriter);
					}
				}
				result = result | pWriter->EndGroup();

				break;

			default:
				TDE2_UNREACHABLE();
				break;
		}

		return result;
	}

	TResult<TParticleColorParameter> CParticleEffect::_loadColorData(IArchiveReader* pReader)
	{
		TParticleColorParameter outputColorData;

		E_RESULT_CODE result = RC_OK;

		// note Create a gradient color's object anyway to prevent null dereferencing in the editor
		outputColorData.mGradientColor = CreateGradientColor(TColorUtils::mWhite, TColorUtils::mWhite, result);
		if (RC_OK != result)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		outputColorData.mType = static_cast<E_PARTICLE_COLOR_PARAMETER_TYPE>(pReader->GetUInt16(TParticleEffectClipKeys::TInitialColorKeys::mTypeKeyId));

		switch (outputColorData.mType)
		{
			case E_PARTICLE_COLOR_PARAMETER_TYPE::SINGLE_COLOR:

				pReader->BeginGroup(TParticleEffectClipKeys::TInitialColorKeys::mColorKeyId);
				{
					auto loadColorResult = LoadColor32F(pReader);
					if (loadColorResult.HasError())
					{
						return Wrench::TErrValue<E_RESULT_CODE>(loadColorResult.GetError());
					}

					outputColorData.mFirstColor = loadColorResult.Get();
				}
				pReader->EndGroup();

				return Wrench::TOkValue<TParticleColorParameter>(outputColorData);

			case E_PARTICLE_COLOR_PARAMETER_TYPE::TWEEN_RANDOM:

				// \node Read main color
				pReader->BeginGroup(TParticleEffectClipKeys::TInitialColorKeys::mColorKeyId);
				{
					auto loadColorResult = LoadColor32F(pReader);
					if (loadColorResult.HasError())
					{
						return Wrench::TErrValue<E_RESULT_CODE>(loadColorResult.GetError());
					}

					outputColorData.mFirstColor = loadColorResult.Get();
				}
				pReader->EndGroup();

				// \note Read additional color
				pReader->BeginGroup(TParticleEffectClipKeys::TInitialColorKeys::mAddColorKeyId);
				{
					auto loadColorResult = LoadColor32F(pReader);
					if (loadColorResult.HasError())
					{
						return Wrench::TErrValue<E_RESULT_CODE>(loadColorResult.GetError());
					}

					outputColorData.mSecondColor = loadColorResult.Get();
				}
				pReader->EndGroup();

				return Wrench::TOkValue<TParticleColorParameter>(outputColorData);

			case E_PARTICLE_COLOR_PARAMETER_TYPE::GRADIENT_LERP:
			case E_PARTICLE_COLOR_PARAMETER_TYPE::GRADIENT_RANDOM:

				pReader->BeginGroup(TParticleEffectClipKeys::TInitialColorKeys::mColorKeyId);
				{
					outputColorData.mGradientColor->Load(pReader);
				}
				pReader->EndGroup();

				return Wrench::TOkValue<TParticleColorParameter>(outputColorData);

			default:
				TDE2_UNREACHABLE();
				break;
		}

		return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
	}

	E_RESULT_CODE CParticleEffect::_saveVelocityData(IArchiveWriter* pWriter, const TParticleVelocityParameter& velocityData)
	{
		E_RESULT_CODE result = pWriter->SetUInt16(TParticleEffectClipKeys::TVelocityKeys::mParamTypesKeyId, static_cast<U16>(velocityData.mType));

		switch (velocityData.mType)
		{
			case E_PARTICLE_VELOCITY_PARAMETER_TYPE::CONSTANTS:
				result = result | pWriter->BeginGroup(TParticleEffectClipKeys::TVelocityKeys::mVelocityConstKeyId);
				result = result | SaveVector3(pWriter, velocityData.mVelocityConst);
				result = result | pWriter->EndGroup();

				result = pWriter->SetFloat(TParticleEffectClipKeys::TVelocityKeys::mSpeedConstKeyId, velocityData.mSpeedFactorConst);

				break;

			case E_PARTICLE_VELOCITY_PARAMETER_TYPE::CURVES:
				result = result | pWriter->BeginGroup(TParticleEffectClipKeys::TVelocityKeys::mXCurveKeyId);
				result = result | velocityData.mXCurve->Save(pWriter);
				result = result | pWriter->EndGroup();

				result = result | pWriter->BeginGroup(TParticleEffectClipKeys::TVelocityKeys::mYCurveKeyId);
				result = result | velocityData.mYCurve->Save(pWriter);
				result = result | pWriter->EndGroup();

				result = result | pWriter->BeginGroup(TParticleEffectClipKeys::TVelocityKeys::mZCurveKeyId);
				result = result | velocityData.mZCurve->Save(pWriter);
				result = result | pWriter->EndGroup();

				result = result | pWriter->BeginGroup(TParticleEffectClipKeys::TVelocityKeys::mSpeedCurveKeyId);
				result = result | velocityData.mSpeedFactorCurve->Save(pWriter);
				result = result | pWriter->EndGroup();

				break;
			default:
				TDE2_UNREACHABLE();
				break;
		}
		
		return result;
	}

	TResult<TParticleVelocityParameter> CParticleEffect::_loadVelocityData(IArchiveReader* pReader)
	{
		E_RESULT_CODE result = RC_OK;

		TParticleVelocityParameter velocityData;

		velocityData.mType = static_cast<E_PARTICLE_VELOCITY_PARAMETER_TYPE>(pReader->GetUInt16(TParticleEffectClipKeys::TVelocityKeys::mParamTypesKeyId));

		if (RC_OK != (result = InitVelocityData(velocityData)))
		{
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		switch (velocityData.mType)
		{
			case E_PARTICLE_VELOCITY_PARAMETER_TYPE::CONSTANTS:
				pReader->BeginGroup(TParticleEffectClipKeys::TVelocityKeys::mVelocityConstKeyId);
				{
					auto loadVelocityResult = LoadVector3(pReader);
					if (loadVelocityResult.HasError())
					{
						return Wrench::TErrValue<E_RESULT_CODE>(loadVelocityResult.GetError());
					}

					velocityData.mVelocityConst = loadVelocityResult.Get();
				}
				pReader->EndGroup();

				velocityData.mSpeedFactorConst = pReader->GetFloat(TParticleEffectClipKeys::TVelocityKeys::mSpeedConstKeyId);

				break;

			case E_PARTICLE_VELOCITY_PARAMETER_TYPE::CURVES:

				pReader->BeginGroup(TParticleEffectClipKeys::TVelocityKeys::mXCurveKeyId);
				{
					if (RC_OK != (result = velocityData.mXCurve->Load(pReader)))
					{
						return Wrench::TErrValue<E_RESULT_CODE>(result);
					}
				}
				pReader->EndGroup();

				pReader->BeginGroup(TParticleEffectClipKeys::TVelocityKeys::mYCurveKeyId);
				{
					if (RC_OK != (result = velocityData.mYCurve->Load(pReader)))
					{
						return Wrench::TErrValue<E_RESULT_CODE>(result);
					}
				}
				pReader->EndGroup();

				pReader->BeginGroup(TParticleEffectClipKeys::TVelocityKeys::mZCurveKeyId);
				{
					if (RC_OK != (result = velocityData.mZCurve->Load(pReader)))
					{
						return Wrench::TErrValue<E_RESULT_CODE>(result);
					}
				}
				pReader->EndGroup();

				pReader->BeginGroup(TParticleEffectClipKeys::TVelocityKeys::mSpeedCurveKeyId);
				{
					if (RC_OK != (result = velocityData.mSpeedFactorCurve->Load(pReader)))
					{
						return Wrench::TErrValue<E_RESULT_CODE>(result);
					}
				}
				pReader->EndGroup();

				break;

			default:
				TDE2_UNREACHABLE();
				break;
		}
		
		return Wrench::TOkValue<TParticleVelocityParameter>(velocityData);
	}

	const TPtr<IResourceLoader> CParticleEffect::_getResourceLoader()
	{
		return mpResourceManager->GetResourceLoader<IParticleEffect>();
	}


	TDE2_API IParticleEffect* CreateParticleEffect(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IParticleEffect, CParticleEffect, result, pResourceManager, pGraphicsContext, name);
	}


	CParticleEffectLoader::CParticleEffectLoader() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CParticleEffectLoader::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pResourceManager || !pGraphicsContext || !pFileSystem)
		{
			return RC_INVALID_ARGS;
		}

		mpResourceManager = pResourceManager;

		mpFileSystem = pFileSystem;

		mpGraphicsContext = pGraphicsContext;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CParticleEffectLoader::LoadResource(IResource* pResource) const
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		if (TResult<TFileEntryId> particleEffectFileId = mpFileSystem->Open<IYAMLFileReader>(pResource->GetName()))
		{
			return dynamic_cast<IParticleEffect*>(pResource)->Load(mpFileSystem->Get<IYAMLFileReader>(particleEffectFileId.Get()));
		}

		return RC_FILE_NOT_FOUND;
	}

	TypeId CParticleEffectLoader::GetResourceTypeId() const
	{
		return IParticleEffect::GetTypeId();
	}


	TDE2_API IResourceLoader* CreateParticleEffectLoader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IResourceLoader, CParticleEffectLoader, result, pResourceManager, pGraphicsContext, pFileSystem);
	}


	CParticleEffectFactory::CParticleEffectFactory() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CParticleEffectFactory::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pGraphicsContext || !pResourceManager)
		{
			return RC_INVALID_ARGS;
		}

		mpResourceManager = pResourceManager;

		mpGraphicsContext = pGraphicsContext;

		mIsInitialized = true;

		return RC_OK;
	}

	IResource* CParticleEffectFactory::Create(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		const TParticleEffectParameters& particleEffectsParams = dynamic_cast<const TParticleEffectParameters&>(params);

		IParticleEffect* pEffect = CreateParticleEffect(mpResourceManager, mpGraphicsContext, name, result);
		if (pEffect)
		{
			pEffect->SetDuration(particleEffectsParams.mDuration);
			pEffect->SetLoopMode(particleEffectsParams.mIsLooped);
			pEffect->SetMaxParticlesCount(particleEffectsParams.mMaxParticlesCount);
		}

		return dynamic_cast<IResource*>(pEffect);
	}

	IResource* CParticleEffectFactory::CreateDefault(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		return dynamic_cast<IResource*>(CreateParticleEffect(mpResourceManager, mpGraphicsContext, name, result));
	}

	TypeId CParticleEffectFactory::GetResourceTypeId() const
	{
		return IParticleEffect::GetTypeId();
	}


	TDE2_API IResourceFactory* CreateParticleEffectFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IResourceFactory, CParticleEffectFactory, result, pResourceManager, pGraphicsContext);
	}
}