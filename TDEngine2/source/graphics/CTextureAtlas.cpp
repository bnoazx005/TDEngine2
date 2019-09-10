#include "./../../include/graphics/CTextureAtlas.h"
#include "./../../include/core/IGraphicsContext.h"
#include "./../../include/core/IResourceManager.h"
#include "./../../include/core/IFileSystem.h"


namespace TDEngine2
{
	CTextureAtlas::CTextureAtlas() :
		CBaseResource()
	{
	}

	E_RESULT_CODE CTextureAtlas::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name)
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

	E_RESULT_CODE CTextureAtlas::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
									  const TTexture2DParameters& params)
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

	E_RESULT_CODE CTextureAtlas::Load()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		const IResourceLoader* pResourceLoader = mpResourceManager->GetResourceLoader<CTextureAtlas>();

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

	E_RESULT_CODE CTextureAtlas::Unload()
	{
		return Reset();
	}

	E_RESULT_CODE CTextureAtlas::Reset()
	{
		return RC_NOT_IMPLEMENTED_YET;
	}
	
	void CTextureAtlas::Bind(U32 slot)
	{

	}

	U32 CTextureAtlas::GetWidth() const
	{
		return 0x0;
	}

	U32 CTextureAtlas::GetHeight() const
	{
		return 0x0;
	}


	/*!
		\brief CTextureAtlasLoader's definition
	*/

	CTextureAtlasLoader::CTextureAtlasLoader() :
		mIsInitialized(false)
	{
	}

	E_RESULT_CODE CTextureAtlasLoader::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem)
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

	E_RESULT_CODE CTextureAtlasLoader::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	E_RESULT_CODE CTextureAtlasLoader::LoadResource(IResource* pResource) const
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;
/*
		TResult<TFileEntryId> materialFileId = mpFileSystem->Open<CBinaryFileReader>(pResource->GetName());

		if (materialFileId.HasError())
		{
			return materialFileId.GetError();
		}

		IBinaryFileReader* pMaterialFile = dynamic_cast<IBinaryFileReader*>(mpFileSystem->Get<CBinaryFileReader>(materialFileId.Get()));

		/// try to read the file's header
		TTextureAtlasFileHeader header = _readMaterialFileHeader(pMaterialFile).Get();

		if ((result = pMaterialFile->Close()) != RC_OK)
		{
			return result;
		}*/

		return RC_OK;
	}

	U32 CTextureAtlasLoader::GetResourceTypeId() const
	{
		return CTextureAtlas::GetTypeId();
	}


	TDE2_API IResourceLoader* CreateTextureAtlasLoader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem, E_RESULT_CODE& result)
	{
		CTextureAtlasLoader* pMaterialLoaderInstance = new (std::nothrow) CTextureAtlasLoader();

		if (!pMaterialLoaderInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pMaterialLoaderInstance->Init(pResourceManager, pGraphicsContext, pFileSystem);

		if (result != RC_OK)
		{
			delete pMaterialLoaderInstance;

			pMaterialLoaderInstance = nullptr;
		}

		return pMaterialLoaderInstance;
	}


	/*!
		\brief CTextureAtlasFactory's definition
	*/

	CTextureAtlasFactory::CTextureAtlasFactory() :
		mIsInitialized(false)
	{
	}

	E_RESULT_CODE CTextureAtlasFactory::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext)
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

	E_RESULT_CODE CTextureAtlasFactory::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	IResource* CTextureAtlasFactory::Create(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		return nullptr;
	}

	IResource* CTextureAtlasFactory::CreateDefault(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		return nullptr;
	}

	U32 CTextureAtlasFactory::GetResourceTypeId() const
	{
		return CTextureAtlas::GetTypeId();
	}


	TDE2_API IResourceFactory* CreateTextureAtlasFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		CTextureAtlasFactory* pMaterialFactoryInstance = new (std::nothrow) CTextureAtlasFactory();

		if (!pMaterialFactoryInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pMaterialFactoryInstance->Init(pResourceManager, pGraphicsContext);

		if (result != RC_OK)
		{
			delete pMaterialFactoryInstance;

			pMaterialFactoryInstance = nullptr;
		}

		return pMaterialFactoryInstance;
	}
}