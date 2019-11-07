#include "./../../include/graphics/CBaseMaterial.h"
#include "./../../include/core/IGraphicsContext.h"
#include "./../../include/core/IResourceManager.h"
#include "./../../include/core/IFileSystem.h"
#include "./../../include/platform/CBinaryFileReader.h"
#include "./../../include/graphics/CBaseShader.h"
#include "./../../include/core/IResourceHandler.h"
#include "./../../include/graphics/ITexture.h"
#include <cstring>


namespace TDEngine2
{
	/*!
		\note The declaration of TMaterialParameters is placed at IMaterial.h
	*/

	TMaterialParameters::TMaterialParameters(const std::string& shaderName, bool isTransparent):
		mShaderName(shaderName), mIsUseAlphaBlending(isTransparent)
	{
	}


	CBaseMaterial::CBaseMaterial() :
		CBaseResource()
	{
	}

	E_RESULT_CODE CBaseMaterial::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name)
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

	E_RESULT_CODE CBaseMaterial::Load()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		const IResourceLoader* pResourceLoader = mpResourceManager->GetResourceLoader<CBaseMaterial>();

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

	E_RESULT_CODE CBaseMaterial::Unload()
	{
		return Reset();
	}

	E_RESULT_CODE CBaseMaterial::Reset()
	{
		return RC_NOT_IMPLEMENTED_YET;
	}

	void CBaseMaterial::SetShader(const std::string& shaderName)
	{
		//mpShader = mpResourceManager->Create<CBaseShader>(&shaderParams);
		mpShader = mpResourceManager->Load<CBaseShader>(shaderName); /// \todo replace it with Create and load only on demand within Load method
	}

	void CBaseMaterial::SetTransparentState(bool isTransparent)
	{
		mIsTransparent = isTransparent;
	}

	void CBaseMaterial::Bind()
	{
		IShader* pShaderInstance = mpShader->Get<IShader>(TDEngine2::RAT_BLOCKING);

		if (!pShaderInstance)
		{
			return;
		}

		pShaderInstance->Bind();

		for (auto iter = mpAssignedTextures.cbegin(); iter != mpAssignedTextures.cend(); ++iter)
		{
			pShaderInstance->SetTextureResource(iter->first, iter->second);
		}
	}

	E_RESULT_CODE CBaseMaterial::SetTextureResource(const std::string& resourceName, ITexture* pTexture)
	{
		if (resourceName.empty() || !pTexture)
		{
			return RC_INVALID_ARGS;
		}

		mpAssignedTextures[resourceName] = pTexture;

		return RC_OK;
	}

	IResourceHandler* CBaseMaterial::GetShaderHandler() const
	{
		return mpShader;
	}

	bool CBaseMaterial::IsTransparent() const
	{
		return mIsTransparent;
	}


	TDE2_API IMaterial* CreateBaseMaterial(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, E_RESULT_CODE& result)
	{
		CBaseMaterial* pMaterialInstance = new (std::nothrow) CBaseMaterial();

		if (!pMaterialInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pMaterialInstance->Init(pResourceManager, pGraphicsContext, name);

		if (result != RC_OK)
		{
			delete pMaterialInstance;

			pMaterialInstance = nullptr;
		}

		return pMaterialInstance;
	}


	TDE2_API IMaterial* CreateBaseMaterial(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
										   const TMaterialParameters& params, E_RESULT_CODE& result)
	{
		CBaseMaterial* pMaterialInstance = new (std::nothrow) CBaseMaterial();

		if (!pMaterialInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pMaterialInstance->Init(pResourceManager, pGraphicsContext, name);
		
		if (result != RC_OK)
		{
			delete pMaterialInstance;

			pMaterialInstance = nullptr;
		}
		else
		{
			pMaterialInstance->SetShader(params.mShaderName);
			pMaterialInstance->SetTransparentState(params.mIsUseAlphaBlending);
		}

		return pMaterialInstance;
	}
	

	CBaseMaterialLoader::CBaseMaterialLoader() :
		mIsInitialized(false)
	{
	}

	E_RESULT_CODE CBaseMaterialLoader::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem)
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

	E_RESULT_CODE CBaseMaterialLoader::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	E_RESULT_CODE CBaseMaterialLoader::LoadResource(IResource* pResource) const
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		TResult<TFileEntryId> materialFileId = mpFileSystem->Open<CBinaryFileReader>(pResource->GetName());

		if (materialFileId.HasError())
		{
			return materialFileId.GetError();
		}

		IBinaryFileReader* pMaterialFile = dynamic_cast<IBinaryFileReader*>(mpFileSystem->Get<CBinaryFileReader>(materialFileId.Get()));
		
		/// try to read the file's header
		TBaseMaterialFileHeader header = _readMaterialFileHeader(pMaterialFile).Get();

		if ((result = pMaterialFile->Close()) != RC_OK)
		{
			return result;
		}

		return RC_OK;
	}

	U32 CBaseMaterialLoader::GetResourceTypeId() const
	{
		return CBaseMaterial::GetTypeId();
	}

	TResult<TBaseMaterialFileHeader> CBaseMaterialLoader::_readMaterialFileHeader(IBinaryFileReader* pFileReader) const
	{
		TBaseMaterialFileHeader header;

		memset(&header, 0, sizeof(TBaseMaterialFileHeader));

		E_RESULT_CODE result = RC_OK;

		/// read the file's header
		if ((result = pFileReader->Read(static_cast<void*>(header.mTag), sizeof(C8) * BaseMaterialFileTagLength)) != RC_OK)
		{
			return TErrorValue<E_RESULT_CODE>(result);
		}

		if (strncmp(header.mTag, "MAT", BaseMaterialFileTagLength))
		{
			return TErrorValue<E_RESULT_CODE>(RC_FAIL);
		}

		/// read endianness of the file
		if ((result = pFileReader->Read(static_cast<void*>(&header.mEndianType), sizeof(U8))) != RC_OK)
		{
			return TErrorValue<E_RESULT_CODE>(result);
		}

		/// read offsets' values of data blocks
		if ((result = pFileReader->Read(static_cast<void*>(&header.mShaderEntriesBlockOffset), sizeof(U8))) != RC_OK)
		{
			return TErrorValue<E_RESULT_CODE>(result);
		}

		if ((result = pFileReader->Read(static_cast<void*>(&header.mMaterialPropertiesBlockOffset), sizeof(U8))) != RC_OK)
		{
			return TErrorValue<E_RESULT_CODE>(result);
		}

		return TOkValue<TBaseMaterialFileHeader>(header);
	}


	TDE2_API IResourceLoader* CreateBaseMaterialLoader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem, E_RESULT_CODE& result)
	{
		CBaseMaterialLoader* pMaterialLoaderInstance = new (std::nothrow) CBaseMaterialLoader();

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


	CBaseMaterialFactory::CBaseMaterialFactory():
		mIsInitialized(false)
	{
	}

	E_RESULT_CODE CBaseMaterialFactory::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext)
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

	E_RESULT_CODE CBaseMaterialFactory::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	IResource* CBaseMaterialFactory::Create(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		const TMaterialParameters& matParams = dynamic_cast<const TMaterialParameters&>(params);

		return dynamic_cast<IResource*>(CreateBaseMaterial(mpResourceManager, mpGraphicsContext, name, matParams, result));
	}

	IResource* CBaseMaterialFactory::CreateDefault(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		return dynamic_cast<IResource*>(CreateBaseMaterial(mpResourceManager, mpGraphicsContext, name, result));
	}

	U32 CBaseMaterialFactory::GetResourceTypeId() const
	{
		return CBaseMaterial::GetTypeId();
	}


	TDE2_API IResourceFactory* CreateBaseMaterialFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		CBaseMaterialFactory* pMaterialFactoryInstance = new (std::nothrow) CBaseMaterialFactory();

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