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

		;

		return RC_OK;
	}

	E_RESULT_CODE CBasePostProcessingProfile::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_INVALID_ARGS;
		}

		;

		return RC_OK;
	}

	const IResourceLoader* CBasePostProcessingProfile::_getResourceLoader()
	{
		return mpResourceManager->GetResourceLoader<CBasePostProcessingProfile>();
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

		if (TResult<TFileEntryId> materialFileId = mpFileSystem->Open<IYAMLFileReader>(pResource->GetName()))
		{
			return dynamic_cast<IPostProcessingProfile*>(pResource)->Load(mpFileSystem->Get<IYAMLFileReader>(materialFileId.Get()));
		}

		return RC_FILE_NOT_FOUND;
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