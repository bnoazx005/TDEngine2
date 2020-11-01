#include "../../include/graphics/CBasePostProcessingProfile.h"
#include "../../include/core/IFileSystem.h"
#include "../../include/platform/CYAMLFile.h"


namespace TDEngine2
{
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

	E_RESULT_CODE CBasePostProcessingProfile::Load()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		const IResourceLoader* pResourceLoader = mpResourceManager->GetResourceLoader<CBasePostProcessingProfile>();

		if (!pResourceLoader)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = pResourceLoader->LoadResource(this);

		if (result != RC_OK)
		{
			mState = RST_PENDING;

			return result;
		}

		mState = RST_LOADED;

		return result;
	}

	E_RESULT_CODE CBasePostProcessingProfile::Unload()
	{
		return Reset();
	}

	E_RESULT_CODE CBasePostProcessingProfile::Reset()
	{
		return RC_NOT_IMPLEMENTED_YET;
	}

	E_RESULT_CODE CBasePostProcessingProfile::Serialize(IFileSystem* pFileSystem, const std::string& filename)
	{
		TDE2_UNIMPLEMENTED();
		return RC_NOT_IMPLEMENTED_YET;
	}

	E_RESULT_CODE CBasePostProcessingProfile::Deserialize(IFileSystem* pFileSystem, const std::string& filename)
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		TResult<TFileEntryId> fileReadingResult = pFileSystem->Open<IYAMLFileReader>(filename);

		if (fileReadingResult.HasError())
		{
			return fileReadingResult.GetError();
		}

		auto pYAMLFileReader = pFileSystem->Get<IYAMLFileReader>(fileReadingResult.Get());

		Yaml::Node profileDataRoot;

		E_RESULT_CODE result = RC_OK;

		if ((result = pYAMLFileReader->Deserialize(profileDataRoot)) != RC_OK)
		{
			return result;
		}

		pYAMLFileReader->Close();

		// \todo 

		return RC_OK;
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

	E_RESULT_CODE CBasePostProcessingProfileLoader::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	E_RESULT_CODE CBasePostProcessingProfileLoader::LoadResource(IResource* pResource) const
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		return dynamic_cast<IPostProcessingProfile*>(pResource)->Deserialize(mpFileSystem, pResource->GetName() + ".camera_profile.info");
	}

	TypeId CBasePostProcessingProfileLoader::GetResourceTypeId() const
	{
		return CBasePostProcessingProfile::GetTypeId();
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

	E_RESULT_CODE CBasePostProcessingProfileFactory::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

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
		return CBasePostProcessingProfile::GetTypeId();
	}


	TDE2_API IResourceFactory* CreateBasePostProcessingProfileFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IResourceFactory, CBasePostProcessingProfileFactory, result, pResourceManager, pGraphicsContext);
	}
}