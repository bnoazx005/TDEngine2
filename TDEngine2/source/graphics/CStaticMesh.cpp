#include "./../../include/graphics/CStaticMesh.h"
#include "./../../include/core/IFileSystem.h"
#include "./../../include/core/IFile.h"
#include "./../../include/utils/CFileLogger.h"
#include "./../../include/core/IGraphicsContext.h"
#include "./../../include/graphics/IGraphicsObjectManager.h"
#include <cstring>
#include <climits>


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

		mpGraphicsObjectManager = pGraphicsContext->GetGraphicsObjectManager();

		mState = RST_LOADED;

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

		mpGraphicsObjectManager = pGraphicsContext->GetGraphicsObjectManager();

		mState = RST_LOADED;

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

	E_RESULT_CODE CStaticMesh::PostLoad()
	{
		// create shared buffers for the mesh
		auto&& vertices = ToArrayOfStructsDataLayout();

		auto vertexBufferResult = mpGraphicsObjectManager->CreateVertexBuffer(BUT_STATIC, vertices.size(), &vertices[0]);
		if (vertexBufferResult.HasError())
		{
			return vertexBufferResult.GetError();
		}

		mpSharedVertexBuffer = vertexBufferResult.Get();

		// \note create a position-only vertex buffer
		// \todo In future may be it's better to split shared VB into separate channels

		auto positionOnlyVertexBufferResult = mpGraphicsObjectManager->CreateVertexBuffer(BUT_STATIC, mPositions.size() * sizeof(TVector4), &mPositions[0]);
		if (positionOnlyVertexBufferResult.HasError())
		{
			return positionOnlyVertexBufferResult.GetError();
		}

		mpPositionOnlyVertexBuffer = positionOnlyVertexBufferResult.Get();

		U32 indicesCount = mIndices.size();
		E_INDEX_FORMAT_TYPE indexFormatType = indicesCount < (std::numeric_limits<U16>::max)() ? IFT_INDEX16 : IFT_INDEX32;

		std::vector<U8> indices = _getIndicesArray(indexFormatType);

		auto indexBufferResult = mpGraphicsObjectManager->CreateIndexBuffer(BUT_STATIC, indexFormatType, indicesCount * static_cast<U32>(indexFormatType), &indices[0]);
		if (indexBufferResult.HasError())
		{
			return indexBufferResult.GetError();
		}

		mpSharedIndexBuffer = indexBufferResult.Get();

		return RC_OK;
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

	void CStaticMesh::AddColor(const TColor32F& color)
	{
		mVertexColors.emplace_back(color);
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

	void CStaticMesh::AddFace(const U32 face[3])
	{
		mIndices.push_back(face[0]);
		mIndices.push_back(face[1]);
		mIndices.push_back(face[2]);
	}

	const CStaticMesh::TPositionsArray& CStaticMesh::GetPositionsArray() const
	{
		return mPositions;
	}

	const CStaticMesh::TVertexColorArray& CStaticMesh::GetColorsArray() const
	{
		return mVertexColors;
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

	bool CStaticMesh::HasColors() const
	{
		return mVertexColors.size();
	}

	bool CStaticMesh::HasNormals() const
	{
		return mNormals.size();
	}

	bool CStaticMesh::HasTangents() const
	{
		return mTangents.size();
	}

	bool CStaticMesh::HasTexCoords0() const
	{
		return mTexcoords0.size();
	}

	std::vector<U8> CStaticMesh::ToArrayOfStructsDataLayout() const
	{
		U32 strideSize = sizeof(TVector4) + sizeof(TColor32F);
		strideSize += (HasTexCoords0() ? sizeof(TVector4) : 0); // \note texcoords use float2, but we align them manually to float4
		strideSize += (HasNormals() ? sizeof(TVector4) : 0);
		strideSize += (HasTangents() ? sizeof(TVector4) : 0);

		std::vector<U8> bytes(mPositions.size() * strideSize);

		static TColor32F white(1.0f); // \todo replace this static variable with global one in TColor.h

		for (U32 i = 0, ptrPos = 0; i < mPositions.size(); ++i, ptrPos += strideSize)
		{
			// mandatory element
			memcpy(&bytes[ptrPos], &mPositions[i], sizeof(TVector4));
			memcpy(&bytes[ptrPos + sizeof(TVector4)], HasColors() ? &mVertexColors[i] : &white, sizeof(TColor32F));
			
			if (HasTexCoords0()) { memcpy(&bytes[ptrPos + 2 * sizeof(TVector4)], &mTexcoords0[i], sizeof(TVector2)); }
			if (HasNormals()) { memcpy(&bytes[ptrPos + 3 * sizeof(TVector4)], &mNormals[i], sizeof(TVector4)); }
			if (HasTangents()) { memcpy(&bytes[ptrPos + 4 * sizeof(TVector4)], &mTangents[i], sizeof(TVector4)); }
		}

		return bytes;
	}

	U32 CStaticMesh::GetFacesCount() const
	{
		return mIndices.size() / 3;
	}

	IVertexBuffer* CStaticMesh::GetSharedVertexBuffer() const
	{
		return mpSharedVertexBuffer;
	}

	IVertexBuffer* CStaticMesh::GetPositionOnlyVertexBuffer() const
	{
		return mpPositionOnlyVertexBuffer;
	}

	IIndexBuffer* CStaticMesh::GetSharedIndexBuffer() const
	{
		return mpSharedIndexBuffer;
	}

	std::vector<U8> CStaticMesh::_getIndicesArray(const E_INDEX_FORMAT_TYPE& indexFormat) const
	{
		std::vector<U8> indicesBytesArray(static_cast<U32>(indexFormat) * mIndices.size());

		U8* pPtr = &indicesBytesArray[0];

		for (auto&& currIndex : mIndices)
		{
			switch (indexFormat)
			{
				case IFT_INDEX16:
					*reinterpret_cast<U16*>(pPtr) = currIndex;
					break;
				case IFT_INDEX32:
					*reinterpret_cast<U32*>(pPtr) = currIndex;
					break;
			}

			pPtr += static_cast<U32>(indexFormat);
		}

		return indicesBytesArray;
	}


	IStaticMesh* CStaticMesh::CreateCube(IResourceManager* pResourceManager)
	{
		auto pCubeMeshResource = pResourceManager->Create<CStaticMesh>("Cube", TMeshParameters{})->Get<CStaticMesh>(RAT_BLOCKING);

		// clock-wise order is used, bottom face
		pCubeMeshResource->AddPosition({ 0.5f, -0.5f, -0.5f, 1.0f }); pCubeMeshResource->AddTexCoord0({ 1.0f, 1.0f });
		pCubeMeshResource->AddPosition({ -0.5f, -0.5f, -0.5f, 1.0f }); pCubeMeshResource->AddTexCoord0({ 0.0f, 1.0f });
		pCubeMeshResource->AddPosition({ -0.5f, -0.5f, 0.5f, 1.0f }); pCubeMeshResource->AddTexCoord0({ 1.0f, 0.0f });
		pCubeMeshResource->AddPosition({ 0.5f, -0.5f, 0.5f, 1.0f }); pCubeMeshResource->AddTexCoord0({ 0.0f, 0.0f });

		// top face
		pCubeMeshResource->AddPosition({ 0.5f, 0.5f, -0.5f, 1.0f }); pCubeMeshResource->AddTexCoord0({ 1.0f, 1.0f });
		pCubeMeshResource->AddPosition({ -0.5f, 0.5f, -0.5f, 1.0f }); pCubeMeshResource->AddTexCoord0({ 0.0f, 1.0f });
		pCubeMeshResource->AddPosition({ -0.5f, 0.5f, 0.5f, 1.0f }); pCubeMeshResource->AddTexCoord0({ 1.0f, 0.0f });
		pCubeMeshResource->AddPosition({ 0.5f, 0.5f, 0.5f, 1.0f }); pCubeMeshResource->AddTexCoord0({ 0.0f, 0.0f });

		// front face
		pCubeMeshResource->AddPosition({ 0.5f, -0.5f, -0.5f, 1.0f }); pCubeMeshResource->AddTexCoord0({ 1.0f, 1.0f });
		pCubeMeshResource->AddPosition({ -0.5f, -0.5f, -0.5f, 1.0f }); pCubeMeshResource->AddTexCoord0({ 0.0f, 1.0f });
		pCubeMeshResource->AddPosition({ -0.5f, 0.5f, -0.5f, 1.0f }); pCubeMeshResource->AddTexCoord0({ 0.0f, 0.0f });
		pCubeMeshResource->AddPosition({ 0.5f, 0.5f, -0.5f, 1.0f }); pCubeMeshResource->AddTexCoord0({ 1.0f, 0.0f });

		// back face
		pCubeMeshResource->AddPosition({ 0.5f, -0.5f, 0.5f, 1.0f }); pCubeMeshResource->AddTexCoord0({ 1.0f, 1.0f });
		pCubeMeshResource->AddPosition({ -0.5f, -0.5f, 0.5f, 1.0f }); pCubeMeshResource->AddTexCoord0({ 0.0f, 1.0f });
		pCubeMeshResource->AddPosition({ -0.5f, 0.5f, 0.5f, 1.0f }); pCubeMeshResource->AddTexCoord0({ 0.0f, 0.0f });
		pCubeMeshResource->AddPosition({ 0.5f, 0.5f, 0.5f, 1.0f }); pCubeMeshResource->AddTexCoord0({ 1.0f, 0.0f });

		// left face
		pCubeMeshResource->AddPosition({ -0.5f, 0.5f, -0.5f, 1.0f }); pCubeMeshResource->AddTexCoord0({ 0.0f, 0.0f });
		pCubeMeshResource->AddPosition({ -0.5f, -0.5f, -0.5f, 1.0f }); pCubeMeshResource->AddTexCoord0({ 0.0f, 1.0f });
		pCubeMeshResource->AddPosition({ -0.5f, -0.5f, 0.5f, 1.0f }); pCubeMeshResource->AddTexCoord0({ 1.0f, 0.0f });
		pCubeMeshResource->AddPosition({ -0.5f, 0.5f, 0.5f, 1.0f }); pCubeMeshResource->AddTexCoord0({ 1.0f, 1.0f });

		// right face
		pCubeMeshResource->AddPosition({ 0.5f, 0.5f, -0.5f, 1.0f }); pCubeMeshResource->AddTexCoord0({ 0.0f, 0.0f });
		pCubeMeshResource->AddPosition({ 0.5f, -0.5f, -0.5f, 1.0f }); pCubeMeshResource->AddTexCoord0({ 0.0f, 1.0f });
		pCubeMeshResource->AddPosition({ 0.5f, -0.5f, 0.5f, 1.0f }); pCubeMeshResource->AddTexCoord0({ 1.0f, 0.0f });
		pCubeMeshResource->AddPosition({ 0.5f, 0.5f, 0.5f, 1.0f }); pCubeMeshResource->AddTexCoord0({ 1.0f, 1.0f });

		static const U32 indices[] = 
		{
			0, 1, 2,
			0, 2, 3,
			4, 5, 6,
			4, 6, 7,
			8, 9, 10,
			8, 10, 11,
			12, 13, 14,
			12, 14, 15,
			16, 17, 18,
			16, 18, 19,
			20, 21, 22,
			20, 22, 23,
		};

		for (U8 i = 0, index = 0; i < 6; ++i, index += 6)
		{
			pCubeMeshResource->AddFace(&indices[index]);
			pCubeMeshResource->AddFace(&indices[index + 3]);
		}

		PANIC_ON_FAILURE(pCubeMeshResource->PostLoad());

		return pCubeMeshResource;
	}


	TDE2_API IStaticMesh* CreateStaticMesh(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, E_RESULT_CODE& result)
	{
		CStaticMesh* pMeshInstance = new (std::nothrow) CStaticMesh();

		if (!pMeshInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pMeshInstance->Init(pResourceManager, pGraphicsContext, name);

		if (result != RC_OK)
		{
			delete pMeshInstance;

			pMeshInstance = nullptr;
		}

		return pMeshInstance;
	}


	TDE2_API IStaticMesh* CreateStaticMesh(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
										   const TMeshParameters& params, E_RESULT_CODE& result)
	{
		CStaticMesh* pMeshInstance = new (std::nothrow) CStaticMesh();

		if (!pMeshInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pMeshInstance->Init(pResourceManager, pGraphicsContext, name);

		if (result != RC_OK)
		{
			delete pMeshInstance;

			pMeshInstance = nullptr;
		}
		else
		{
			// \todo
		}

		return pMeshInstance;
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

		TResult<TFileEntryId> meshFileId = mpFileSystem->Open<IBinaryMeshFileReader>(pResource->GetName());
		if (meshFileId.HasError())
		{
			LOG_WARNING(std::string("[Mesh Loader] Could not load the specified mesh file (").append(pResource->GetName()).append("), load default one instead..."));

			/// \note can't load file with the shader, so load default one
			return RC_FAIL;
		}

		IBinaryMeshFileReader* pMeshFileReader = dynamic_cast<IBinaryMeshFileReader*>(mpFileSystem->Get<IBinaryMeshFileReader>(meshFileId.Get()));		
		IMesh* pMesh = dynamic_cast<IMesh*>(pResource);

		if ((result = pMeshFileReader->LoadMesh(pMesh)) != RC_OK ||
			(result = pMesh->PostLoad()) != RC_OK)
		{
			return result;
		}
		

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