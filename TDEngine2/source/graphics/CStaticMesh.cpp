#include "./../../include/graphics/CStaticMesh.h"


namespace TDEngine2
{
	CStaticMesh::CStaticMesh() :
		CBaseResource()
	{
	}

	E_RESULT_CODE CStaticMesh::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name)
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

		mIsInitialized = true;

		return RC_OK;
	}
	
	E_RESULT_CODE CStaticMesh::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
									const TMeshParameters& params)
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

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CStaticMesh::Load()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		const IResourceLoader* pResourceLoader = mpResourceManager->GetResourceLoader<CStaticMesh>();

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

	E_RESULT_CODE CStaticMesh::Unload()
	{
		return Reset();
	}

	E_RESULT_CODE CStaticMesh::Reset()
	{
		return RC_NOT_IMPLEMENTED_YET;
	}

	void CStaticMesh::AddPosition(const TVector4& pos)
	{
		mPositions.emplace_back(pos);
	}

	void CStaticMesh::AddNormal(const TVector4& normal)
	{
		mNormals.emplace_back(normal);
	}

	void CStaticMesh::AddTangent(const TVector4& tangent)
	{
		mTangents.emplace_back(tangent);
	}

	void CStaticMesh::AddTexCoord0(const TVector2& uv0)
	{
		mTexcoords0.emplace_back(uv0);
	}

	void CStaticMesh::AddFace(F32 face[3])
	{
		mIndices.push_back(face[0]);
		mIndices.push_back(face[1]);
		mIndices.push_back(face[2]);
	}

	const CStaticMesh::TPositionsArray& CStaticMesh::GetPositionsArray() const
	{
		return mPositions;
	}

	const CStaticMesh::TNormalsArray& CStaticMesh::GetNormalsArray() const
	{
		return mNormals;
	}

	const CStaticMesh::TTangentsArray& CStaticMesh::GetTangentsArray() const
	{
		return mTangents;
	}

	const CStaticMesh::TTexcoordsArray& CStaticMesh::GetTexCoords0Array() const
	{
		return mTexcoords0;
	}

	const CStaticMesh::TIndicesArray& CStaticMesh::GetIndices() const
	{
		return mIndices;
	}


	TDE2_API IStaticMesh* CreateStaticMesh(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, E_RESULT_CODE& result)
	{
		CStaticMesh* pMaterialInstance = new (std::nothrow) CStaticMesh();

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


	TDE2_API IStaticMesh* CreateStaticMesh(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
										   const TMeshParameters& params, E_RESULT_CODE& result)
	{
		CStaticMesh* pMaterialInstance = new (std::nothrow) CStaticMesh();

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
			// \todo
		}

		return pMaterialInstance;
	}


	CStaticMeshLoader::CStaticMeshLoader() :
		mIsInitialized(false)
	{
	}

	E_RESULT_CODE CStaticMeshLoader::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem)
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

	E_RESULT_CODE CStaticMeshLoader::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	E_RESULT_CODE CStaticMeshLoader::LoadResource(IResource* pResource) const
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		TDE2_UNIMPLEMENTED();

		return RC_OK;
	}

	U32 CStaticMeshLoader::GetResourceTypeId() const
	{
		return CStaticMesh::GetTypeId();
	}


	TDE2_API IResourceLoader* CreateStaticMeshLoader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem, E_RESULT_CODE& result)
	{
		CStaticMeshLoader* pMaterialLoaderInstance = new (std::nothrow) CStaticMeshLoader();

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


	CStaticMeshFactory::CStaticMeshFactory() :
		mIsInitialized(false)
	{
	}

	E_RESULT_CODE CStaticMeshFactory::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext)
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

	E_RESULT_CODE CStaticMeshFactory::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	IResource* CStaticMeshFactory::Create(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		const TMeshParameters& meshParams = dynamic_cast<const TMeshParameters&>(params);

		return dynamic_cast<IResource*>(CreateStaticMesh(mpResourceManager, mpGraphicsContext, name, meshParams, result));
	}

	IResource* CStaticMeshFactory::CreateDefault(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		return dynamic_cast<IResource*>(CreateStaticMesh(mpResourceManager, mpGraphicsContext, name, result));
	}

	U32 CStaticMeshFactory::GetResourceTypeId() const
	{
		return CStaticMesh::GetTypeId();
	}


	TDE2_API IResourceFactory* CreateStaticMeshFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		CStaticMeshFactory* pMaterialFactoryInstance = new (std::nothrow) CStaticMeshFactory();

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