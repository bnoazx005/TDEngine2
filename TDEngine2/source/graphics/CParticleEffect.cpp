#include "../../include/graphics/CParticleEffect.h"
#include "../../include/core/IFileSystem.h"
#include "../../include/core/IFile.h"
#include "../../include/utils/CFileLogger.h"
#include "../../include/core/IGraphicsContext.h"


namespace TDEngine2
{
	struct TParticleEffectClipKeys
	{
		static const std::string mDurationKeyId;
		static const std::string mLoopModeKeyId;
		static const std::string mMaxParticlesCountKeyId;
	};

	const std::string TParticleEffectClipKeys::mDurationKeyId = "duration";
	const std::string TParticleEffectClipKeys::mLoopModeKeyId = "loop-mode";
	const std::string TParticleEffectClipKeys::mMaxParticlesCountKeyId = "max-particles-count";


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