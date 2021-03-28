#include "../../../include/graphics/effects/CParticleEffect.h"
#include "../../../include/core/IFileSystem.h"
#include "../../../include/core/IFile.h"
#include "../../../include/utils/CFileLogger.h"
#include "../../../include/core/IGraphicsContext.h"


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

		static const std::string mSizeOverTimeKeyId;

		static const std::string mEmitterDataGroupId;
		static const std::string mEmissionRateKeyId;

		static const std::string mModifiersFlagsKeyId;


		struct TInitialColorKeys
		{
			static const std::string mTypeKeyId;
			static const std::string mColorKeyId;
			static const std::string mAddColorKeyId;
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
	const std::string TParticleEffectClipKeys::mSizeOverTimeKeyId = "size-over-time";
	const std::string TParticleEffectClipKeys::mEmitterDataGroupId = "emitter-params";
	const std::string TParticleEffectClipKeys::mEmissionRateKeyId = "emission-rate";
	const std::string TParticleEffectClipKeys::mModifiersFlagsKeyId = "modifiers-flags";
	
	const std::string TParticleEffectClipKeys::TInitialColorKeys::mTypeKeyId = "type";
	const std::string TParticleEffectClipKeys::TInitialColorKeys::mColorKeyId = "value0";
	const std::string TParticleEffectClipKeys::TInitialColorKeys::mAddColorKeyId = "value1";


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
				mInitialColor.mType = static_cast<E_PARTICLE_COLOR_PARAMETER_TYPE>(pReader->GetUInt16(TParticleEffectClipKeys::TInitialColorKeys::mTypeKeyId));
				
				pReader->BeginGroup(TParticleEffectClipKeys::TInitialColorKeys::mColorKeyId);
				{
					auto loadColorResult = LoadColor32F(pReader);
					if (loadColorResult.HasError())
					{
						return loadColorResult.GetError();
					}

					mInitialColor.mFirstColor = loadColorResult.Get();
				}
				pReader->EndGroup();

				if (E_PARTICLE_COLOR_PARAMETER_TYPE::TWEEN_RANDOM == mInitialColor.mType)
				{
					// \note Read additional color
					pReader->BeginGroup(TParticleEffectClipKeys::TInitialColorKeys::mAddColorKeyId);
					{
						auto loadColorResult = LoadColor32F(pReader);
						if (loadColorResult.HasError())
						{
							return loadColorResult.GetError();
						}

						mInitialColor.mSecondColor = loadColorResult.Get();
					}
					pReader->EndGroup();
				}
			}
			pReader->EndGroup();

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
		}
		pReader->EndGroup();

		pReader->BeginGroup(TParticleEffectClipKeys::mEmitterDataGroupId);
		{
			auto loadEmitterResult = CBaseParticlesEmitter::Load(pReader);
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
				pWriter->SetUInt16(TParticleEffectClipKeys::TInitialColorKeys::mTypeKeyId, static_cast<U16>(mInitialColor.mType));
				
				pWriter->BeginGroup(TParticleEffectClipKeys::TInitialColorKeys::mColorKeyId);
				{
					SaveColor32F(pWriter, mInitialColor.mFirstColor);
				}
				pWriter->EndGroup();

				if (E_PARTICLE_COLOR_PARAMETER_TYPE::TWEEN_RANDOM == mInitialColor.mType)
				{
					pWriter->BeginGroup(TParticleEffectClipKeys::TInitialColorKeys::mAddColorKeyId);
					{
						SaveColor32F(pWriter, mInitialColor.mSecondColor);
					}
					pWriter->EndGroup();
				}
			}
			pWriter->EndGroup();

			pWriter->BeginGroup(TParticleEffectClipKeys::mSizeOverTimeKeyId);
			{
				mpSizeCurve->Save(pWriter);
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
	}

	void CParticleEffect::SetEmissionRate(U32 value)
	{
		mEmissionRate = value;
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

	U32 CParticleEffect::GetEmissionRate() const
	{
		return mEmissionRate;
	}

	CScopedPtr<CBaseParticlesEmitter> CParticleEffect::GetSharedEmitter() const
	{
		return mpSharedEmitter;
	}

	CScopedPtr<CAnimationCurve> CParticleEffect::GetSizeCurve() const
	{
		return mpSizeCurve;
	}

	E_PARTICLE_EFFECT_INFO_FLAGS CParticleEffect::GetEnabledModifiersFlags() const
	{
		return mModifiersInfoFlags;
	}

	const IResourceLoader* CParticleEffect::_getResourceLoader()
	{
		return mpResourceManager->GetResourceLoader<CParticleEffect>();
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

	E_RESULT_CODE CParticleEffectLoader::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

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
		return CParticleEffect::GetTypeId();
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

	E_RESULT_CODE CParticleEffectFactory::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

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
		return CParticleEffect::GetTypeId();
	}


	TDE2_API IResourceFactory* CreateParticleEffectFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IResourceFactory, CParticleEffectFactory, result, pResourceManager, pGraphicsContext);
	}
}