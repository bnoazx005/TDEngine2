#include "../../include/graphics/CBasePostProcessingProfile.h"
#include "../../include/core/IFileSystem.h"
#include "../../include/platform/CYAMLFile.h"


namespace TDEngine2
{
	struct TPostProcessingProfileIdentifiers
	{
		static const std::string mBloomGroupParameters;
		static const std::string mColorGradingGroupParameters;
		static const std::string mToneMappingGroupParameters;

		static const std::string mIsPassEnabledId;

		struct TBloomParameters
		{
			static const std::string mThresholdId;
			static const std::string mSmoothnessId;
			static const std::string mSamplesCountId;
		};

		struct TColorGradingParameters
		{
			static const std::string mColorLookUpTextureId;
		};

		struct TToneMappingParameters
		{
			static const std::string mExposureId;
		};
	};

	const std::string TPostProcessingProfileIdentifiers::mBloomGroupParameters = "bloom_parameters";
	const std::string TPostProcessingProfileIdentifiers::mColorGradingGroupParameters = "color_grading_params";
	const std::string TPostProcessingProfileIdentifiers::mToneMappingGroupParameters = "tone_mapping_parameters";
	const std::string TPostProcessingProfileIdentifiers::mIsPassEnabledId = "enabled";

	const std::string TPostProcessingProfileIdentifiers::TBloomParameters::mThresholdId = "threshold";
	const std::string TPostProcessingProfileIdentifiers::TBloomParameters::mSmoothnessId = "smoothness";
	const std::string TPostProcessingProfileIdentifiers::TBloomParameters::mSamplesCountId = "samples_count";

	const std::string TPostProcessingProfileIdentifiers::TColorGradingParameters::mColorLookUpTextureId = "color_lut_id";

	const std::string TPostProcessingProfileIdentifiers::TToneMappingParameters::mExposureId = "exposure";


	CBasePostProcessingProfile::CBasePostProcessingProfile():
		CBaseResource()
	{
	}

	E_RESULT_CODE CBasePostProcessingProfile::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name)
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

		mpGraphicsContext = pGraphicsContext;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CBasePostProcessingProfile::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
												   const TPostProcessingProfileParameters& params)
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

		mpGraphicsContext = pGraphicsContext;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CBasePostProcessingProfile::Reset()
	{
		return RC_NOT_IMPLEMENTED_YET;
	}

	E_RESULT_CODE CBasePostProcessingProfile::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_INVALID_ARGS;
		}

		// \note Bloom parameters
		pReader->BeginGroup(TPostProcessingProfileIdentifiers::mBloomGroupParameters);
		{
			mBloomParameters.mIsEnabled = pReader->GetBool(TPostProcessingProfileIdentifiers::mIsPassEnabledId);
			mBloomParameters.mThreshold = pReader->GetFloat(TPostProcessingProfileIdentifiers::TBloomParameters::mThresholdId);
			mBloomParameters.mSmoothness = pReader->GetFloat(TPostProcessingProfileIdentifiers::TBloomParameters::mSmoothnessId);
			mBloomParameters.mSamplesCount = pReader->GetUInt32(TPostProcessingProfileIdentifiers::TBloomParameters::mSamplesCountId);
		}
		pReader->EndGroup();

		// \note Color grading parameters
		pReader->BeginGroup(TPostProcessingProfileIdentifiers::mColorGradingGroupParameters);
		{
			mColorGradingParameters.mIsEnabled = pReader->GetBool(TPostProcessingProfileIdentifiers::mIsPassEnabledId);
			mColorGradingParameters.mLookUpTextureId = pReader->GetString(TPostProcessingProfileIdentifiers::TColorGradingParameters::mColorLookUpTextureId);
		}
		pReader->EndGroup();

		// \note Tone mapping's parameters
		pReader->BeginGroup(TPostProcessingProfileIdentifiers::mToneMappingGroupParameters);
		{
			mToneMappingParameters.mIsEnabled = pReader->GetBool(TPostProcessingProfileIdentifiers::mIsPassEnabledId);
			mToneMappingParameters.mExposure = pReader->GetFloat(TPostProcessingProfileIdentifiers::TToneMappingParameters::mExposureId);
		}
		pReader->EndGroup();

		return RC_OK;
	}

	E_RESULT_CODE CBasePostProcessingProfile::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_INVALID_ARGS;
		}

		// \note Bloom parameters
		pWriter->BeginGroup(TPostProcessingProfileIdentifiers::mBloomGroupParameters);
		{
			pWriter->SetBool(TPostProcessingProfileIdentifiers::mIsPassEnabledId, mBloomParameters.mIsEnabled);
			pWriter->SetFloat(TPostProcessingProfileIdentifiers::TBloomParameters::mThresholdId, mBloomParameters.mThreshold);
			pWriter->SetFloat(TPostProcessingProfileIdentifiers::TBloomParameters::mSmoothnessId, mBloomParameters.mSmoothness);
			pWriter->SetUInt32(TPostProcessingProfileIdentifiers::TBloomParameters::mSamplesCountId, mBloomParameters.mSamplesCount);
		}
		pWriter->EndGroup();

		// \note Color grading parameters
		pWriter->BeginGroup(TPostProcessingProfileIdentifiers::mColorGradingGroupParameters);
		{
			pWriter->SetBool(TPostProcessingProfileIdentifiers::mIsPassEnabledId, mColorGradingParameters.mIsEnabled);
			pWriter->SetString(TPostProcessingProfileIdentifiers::TColorGradingParameters::mColorLookUpTextureId, mColorGradingParameters.mLookUpTextureId);
		}
		pWriter->EndGroup();

		// \note Tone mapping's parameters
		pWriter->BeginGroup(TPostProcessingProfileIdentifiers::mToneMappingGroupParameters);
		{
			pWriter->SetBool(TPostProcessingProfileIdentifiers::mIsPassEnabledId, mToneMappingParameters.mIsEnabled);
			pWriter->SetFloat(TPostProcessingProfileIdentifiers::TToneMappingParameters::mExposureId, mToneMappingParameters.mExposure);
		}
		pWriter->EndGroup();

		return RC_OK;
	}

	const TPostProcessingProfileParameters::TToneMappingParameters& CBasePostProcessingProfile::GetToneMappingParameters() const
	{
		return mToneMappingParameters;
	}

	const TPostProcessingProfileParameters::TBloomParameters& CBasePostProcessingProfile::GetBloomParameters() const
	{
		return mBloomParameters;
	}

	const TPostProcessingProfileParameters::TColorGradingParameters& CBasePostProcessingProfile::GetColorGradingParameters() const
	{
		return mColorGradingParameters;
	}

	const IResourceLoader* CBasePostProcessingProfile::_getResourceLoader()
	{
		return mpResourceManager->GetResourceLoader<IPostProcessingProfile>();
	}


	TDE2_API IPostProcessingProfile* CreateBasePostProcessingProfile(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IPostProcessingProfile, CBasePostProcessingProfile, result, pResourceManager, pGraphicsContext, name);
	}


	TDE2_API IPostProcessingProfile* CreateBasePostProcessingProfile(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
																	 const TPostProcessingProfileParameters& params, E_RESULT_CODE& result)
	{
		CBasePostProcessingProfile* pPostProcessingProfileInstance = new (std::nothrow) CBasePostProcessingProfile();

		if (!pPostProcessingProfileInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pPostProcessingProfileInstance->Init(pResourceManager, pGraphicsContext, name);

		if (result != RC_OK)
		{
			delete pPostProcessingProfileInstance;

			pPostProcessingProfileInstance = nullptr;
		}
		else
		{
			// TODO: Assign parameters into the instance
		}

		return pPostProcessingProfileInstance;
	}


	CBasePostProcessingProfileLoader::CBasePostProcessingProfileLoader() :
		mIsInitialized(false)
	{
	}

	E_RESULT_CODE CBasePostProcessingProfileLoader::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem)
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
		mpFileSystem      = pFileSystem;
		mpGraphicsContext = pGraphicsContext;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CBasePostProcessingProfileLoader::LoadResource(IResource* pResource) const
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		if (TResult<TFileEntryId> materialFileId = mpFileSystem->Open<IYAMLFileReader>(pResource->GetName()))
		{
			return dynamic_cast<IPostProcessingProfile*>(pResource)->Load(mpFileSystem->Get<IYAMLFileReader>(materialFileId.Get()));
		}

		return RC_FILE_NOT_FOUND;
	}

	TypeId CBasePostProcessingProfileLoader::GetResourceTypeId() const
	{
		return IPostProcessingProfile::GetTypeId();
	}


	TDE2_API IResourceLoader* CreateBasePostProcessingProfileLoader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IResourceLoader, CBasePostProcessingProfileLoader, result, pResourceManager, pGraphicsContext, pFileSystem);
	}


	CBasePostProcessingProfileFactory::CBasePostProcessingProfileFactory() :
		mIsInitialized(false)
	{
	}

	E_RESULT_CODE CBasePostProcessingProfileFactory::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext)
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

	IResource* CBasePostProcessingProfileFactory::Create(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		const TPostProcessingProfileParameters& matParams = dynamic_cast<const TPostProcessingProfileParameters&>(params);

		return dynamic_cast<IResource*>(CreateBasePostProcessingProfile(mpResourceManager, mpGraphicsContext, name, matParams, result));
	}

	IResource* CBasePostProcessingProfileFactory::CreateDefault(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		return dynamic_cast<IResource*>(CreateBasePostProcessingProfile(mpResourceManager, mpGraphicsContext, name, result));
	}

	TypeId CBasePostProcessingProfileFactory::GetResourceTypeId() const
	{
		return IPostProcessingProfile::GetTypeId();
	}


	TDE2_API IResourceFactory* CreateBasePostProcessingProfileFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IResourceFactory, CBasePostProcessingProfileFactory, result, pResourceManager, pGraphicsContext);
	}
}