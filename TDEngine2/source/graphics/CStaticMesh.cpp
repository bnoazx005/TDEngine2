#include "../../include/graphics/CStaticMesh.h"
#include "../../include/core/IFileSystem.h"
#include "../../include/core/IFile.h"
#include "../../include/utils/CFileLogger.h"
#include "../../include/core/IGraphicsContext.h"
#include "../../include/graphics/IGraphicsObjectManager.h"
#include "../../include/graphics/CGeometryBuilder.h"
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

		mState = E_RESOURCE_STATE_TYPE::RST_LOADED;

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

		mState = E_RESOURCE_STATE_TYPE::RST_LOADED;

		mIsInitialized = true;

		return RC_OK;
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

		U32 elementsCount = 0;

		for (U32 i = 0, ptrPos = 0; i < mPositions.size(); ++i, ptrPos += strideSize)
		{
			// mandatory element
			memcpy(&bytes[ptrPos], &mPositions[i], sizeof(TVector4));
			memcpy(&bytes[ptrPos + sizeof(TVector4)], HasColors() ? &mVertexColors[i] : &TColorUtils::mWhite, sizeof(TColor32F));
		
			elementsCount = 2; // \note equals to 2 because of position and color are mandatory elements of a vertex declaration

			if (HasTexCoords0()) { memcpy(&bytes[ptrPos + elementsCount++ * sizeof(TVector4)], &mTexcoords0[i], sizeof(TVector2)); }
			if (HasNormals()) { memcpy(&bytes[ptrPos + elementsCount++ * sizeof(TVector4)], &mNormals[i], sizeof(TVector4)); }
			if (HasTangents()) { memcpy(&bytes[ptrPos + elementsCount++ * sizeof(TVector4)], &mTangents[i], sizeof(TVector4)); }
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

	const IResourceLoader* CStaticMesh::_getResourceLoader()
	{
		return mpResourceManager->GetResourceLoader<IStaticMesh>();
	}


	IStaticMesh* CStaticMesh::CreateCube(IResourceManager* pResourceManager)
	{
		auto pCubeMeshResource = pResourceManager->GetResource<IStaticMesh>(pResourceManager->Create<IStaticMesh>("Cube", TMeshParameters{}));

		auto addVertex = [](IStaticMesh* pMesh, const TVector4& pos, const TVector2& texcoords, const TVector4& normal, const TVector4& tangent)
		{
			pMesh->AddPosition(pos); 
			pMesh->AddTexCoord0(texcoords); 
			pMesh->AddNormal(normal);
			pMesh->AddTangent(tangent);
		};

		// clock-wise order is used, bottom face
		addVertex(pCubeMeshResource, { 0.5f, -0.5f, -0.5f, 1.0f }, { 1.0f, 1.0f }, TVector4(-UpVector3, 0.0f), TVector4(-RightVector3, 0.0f));
		addVertex(pCubeMeshResource, { -0.5f, -0.5f, -0.5f, 1.0f }, { 0.0f, 1.0f }, TVector4(-UpVector3, 0.0f), TVector4(-RightVector3, 0.0f));
		addVertex(pCubeMeshResource, { -0.5f, -0.5f, 0.5f, 1.0f }, { 1.0f, 0.0f }, TVector4(-UpVector3, 0.0f), TVector4(-RightVector3, 0.0f));
		addVertex(pCubeMeshResource, { 0.5f, -0.5f, 0.5f, 1.0f }, { 0.0f, 0.0f }, TVector4(-UpVector3, 0.0f), TVector4(-RightVector3, 0.0f));

		// top face
		addVertex(pCubeMeshResource, { 0.5f, 0.5f, -0.5f, 1.0f }, { 1.0f, 1.0f }, TVector4(UpVector3, 0.0f), TVector4(RightVector3, 0.0f));
		addVertex(pCubeMeshResource, { -0.5f, 0.5f, -0.5f, 1.0f }, { 0.0f, 1.0f }, TVector4(UpVector3, 0.0f), TVector4(RightVector3, 0.0f));
		addVertex(pCubeMeshResource, { -0.5f, 0.5f, 0.5f, 1.0f }, { 1.0f, 0.0f }, TVector4(UpVector3, 0.0f), TVector4(RightVector3, 0.0f));
		addVertex(pCubeMeshResource, { 0.5f, 0.5f, 0.5f, 1.0f }, { 0.0f, 0.0f }, TVector4(UpVector3, 0.0f), TVector4(RightVector3, 0.0f));

		// front face
		addVertex(pCubeMeshResource, { 0.5f, -0.5f, -0.5f, 1.0f }, { 1.0f, 1.0f }, TVector4(-ForwardVector3, 0.0f), TVector4(RightVector3, 0.0f));
		addVertex(pCubeMeshResource, { -0.5f, -0.5f, -0.5f, 1.0f }, { 0.0f, 1.0f }, TVector4(-ForwardVector3, 0.0f), TVector4(RightVector3, 0.0f));
		addVertex(pCubeMeshResource, { -0.5f, 0.5f, -0.5f, 1.0f }, { 0.0f, 0.0f }, TVector4(-ForwardVector3, 0.0f), TVector4(RightVector3, 0.0f));
		addVertex(pCubeMeshResource, { 0.5f, 0.5f, -0.5f, 1.0f }, { 1.0f, 0.0f }, TVector4(-ForwardVector3, 0.0f), TVector4(RightVector3, 0.0f));

		// back face
		addVertex(pCubeMeshResource, { 0.5f, -0.5f, 0.5f, 1.0f }, { 1.0f, 1.0f }, TVector4(ForwardVector3, 0.0f), TVector4(-RightVector3, 0.0f));
		addVertex(pCubeMeshResource, { -0.5f, -0.5f, 0.5f, 1.0f }, { 0.0f, 1.0f }, TVector4(ForwardVector3, 0.0f), TVector4(-RightVector3, 0.0f));
		addVertex(pCubeMeshResource, { -0.5f, 0.5f, 0.5f, 1.0f }, { 0.0f, 0.0f }, TVector4(ForwardVector3, 0.0f), TVector4(-RightVector3, 0.0f));
		addVertex(pCubeMeshResource, { 0.5f, 0.5f, 0.5f, 1.0f }, { 1.0f, 0.0f }, TVector4(ForwardVector3, 0.0f), TVector4(-RightVector3, 0.0f));

		// left face
		addVertex(pCubeMeshResource, { -0.5f, 0.5f, -0.5f, 1.0f }, { 0.0f, 0.0f }, TVector4(-RightVector3, 0.0f), TVector4(-ForwardVector3, 0.0f));
		addVertex(pCubeMeshResource, { -0.5f, -0.5f, -0.5f, 1.0f }, { 0.0f, 1.0f }, TVector4(-RightVector3, 0.0f), TVector4(-ForwardVector3, 0.0f));
		addVertex(pCubeMeshResource, { -0.5f, -0.5f, 0.5f, 1.0f }, { 1.0f, 0.0f }, TVector4(-RightVector3, 0.0f), TVector4(-ForwardVector3, 0.0f));
		addVertex(pCubeMeshResource, { -0.5f, 0.5f, 0.5f, 1.0f }, { 1.0f, 1.0f }, TVector4(-RightVector3, 0.0f), TVector4(-ForwardVector3, 0.0f));

		// right face
		addVertex(pCubeMeshResource, { 0.5f, 0.5f, -0.5f, 1.0f }, { 0.0f, 0.0f }, TVector4(RightVector3, 0.0f), TVector4(ForwardVector3, 0.0f));
		addVertex(pCubeMeshResource, { 0.5f, -0.5f, -0.5f, 1.0f }, { 0.0f, 1.0f }, TVector4(RightVector3, 0.0f), TVector4(ForwardVector3, 0.0f));
		addVertex(pCubeMeshResource, { 0.5f, -0.5f, 0.5f, 1.0f }, { 1.0f, 0.0f }, TVector4(RightVector3, 0.0f), TVector4(ForwardVector3, 0.0f));
		addVertex(pCubeMeshResource, { 0.5f, 0.5f, 0.5f, 1.0f }, { 1.0f, 1.0f }, TVector4(RightVector3, 0.0f), TVector4(ForwardVector3, 0.0f));

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

	IStaticMesh* CStaticMesh::CreatePlane(IResourceManager* pResourceManager)
	{
		auto pPlaneMeshResource = pResourceManager->GetResource<IStaticMesh>(pResourceManager->Create<IStaticMesh>("Plane", TMeshParameters{}));

		E_RESULT_CODE result = RC_OK;

		IGeometryBuilder* pGeometryBuilder = CreateGeometryBuilder(result);

		CDeferOperation releaseMemory([&] { pGeometryBuilder->Free(); });

		if (result != RC_OK)
		{
			return nullptr;
		}

		auto meshData = pGeometryBuilder->CreatePlaneGeometry(ZeroVector3, UpVector3, 10.0f, 10.0f, 10);

		for (auto&& currVertex : meshData.mVertices)
		{
			const TVector3& uv = currVertex.mUV;

			pPlaneMeshResource->AddPosition(currVertex.mPosition);
			pPlaneMeshResource->AddTexCoord0(TVector2(uv.x, uv.y));
			pPlaneMeshResource->AddNormal(TVector4(UpVector3, 0.0f));
			pPlaneMeshResource->AddTangent(TVector4(RightVector3, 0.0f));
		}

		auto&& indices = meshData.mIndices;

		U32 currFace[3];

		for (U16 i = 0; i < indices.size(); i += 3)
		{
			currFace[0] = indices[i];
			currFace[1] = indices[i + 1];
			currFace[2] = indices[i + 2];

			pPlaneMeshResource->AddFace(currFace);
		}

		PANIC_ON_FAILURE(pPlaneMeshResource->PostLoad());

		return pPlaneMeshResource;
	}

	IStaticMesh* CStaticMesh::CreateSphere(IResourceManager* pResourceManager)
	{
		constexpr F32 sphereRadius = 10.0f;

		auto pSphereMeshResource = pResourceManager->GetResource<IStaticMesh>(pResourceManager->Create<IStaticMesh>("Sphere", TMeshParameters{}));

		E_RESULT_CODE result = RC_OK;

		IGeometryBuilder* pGeometryBuilder = CreateGeometryBuilder(result);

		CDeferOperation releaseMemory([&] { pGeometryBuilder->Free(); });

		if (result != RC_OK)
		{
			return nullptr;
		}

		const std::tuple<TVector3, TVector3> sidesData[6]
		{
			{ TVector3(sphereRadius * 0.5f, 0.0f, 0.0f), RightVector3 },
			{ TVector3(-sphereRadius * 0.5f, 0.0f, 0.0f), -RightVector3 },
			{ TVector3(0.0f, sphereRadius * 0.5f, 0.0f), UpVector3 },
			{ TVector3(0.0f, -sphereRadius * 0.5f, 0.0f), -UpVector3 },
			{ TVector3(0.0f, 0.0f, sphereRadius * 0.5f), ForwardVector3 },
			{ TVector3(0.0f, 0.0f, -sphereRadius * 0.5f), -ForwardVector3 }
		};

		TVector3 currNormal, currOrigin;

		U16 currOffset = 0;

		for (auto&& currSide : sidesData)
		{
			std::tie(currOrigin, currNormal) = currSide;

			auto meshData = pGeometryBuilder->CreatePlaneGeometry(currOrigin, currNormal, sphereRadius, sphereRadius, 10);

			for (auto&& currVertex : meshData.mVertices)
			{
				const TVector3& uv = currVertex.mUV;

				TVector4 normal = Normalize(currVertex.mPosition) * 0.5f; // \note The whole sphere diameter equals to 1 in-game meter

				pSphereMeshResource->AddPosition(TVector4(normal, 1.0f));
				pSphereMeshResource->AddTexCoord0(TVector2(uv.x, uv.y));
				pSphereMeshResource->AddNormal(TVector4(Normalize(normal), 0.0f));
				pSphereMeshResource->AddTangent({}); // \todo Implement correct computation of a tangent
			}

			auto&& indices = meshData.mIndices;

			U32 currFace[3];

			for (U16 i = 0; i < indices.size(); i += 3)
			{
				currFace[0] = currOffset + indices[i];
				currFace[1] = currOffset + indices[i + 1];
				currFace[2] = currOffset + indices[i + 2];

				pSphereMeshResource->AddFace(currFace);
			}

			currOffset += static_cast<U16>(meshData.mVertices.size());
		}

		PANIC_ON_FAILURE(pSphereMeshResource->PostLoad());

		return pSphereMeshResource;
	}


	TDE2_API IStaticMesh* CreateStaticMesh(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IStaticMesh, CStaticMesh, result, pResourceManager, pGraphicsContext, name);
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
		CBaseObject()
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

		// \todo Refactor this later (may be the declaration should be placed somewhere else
		static const std::unordered_map<std::string, std::function<void()>> builtInMeshesFactories
		{
			{ "Cube", [this] { CStaticMesh::CreateCube(mpResourceManager); } },
			{ "Plane", [this] {  CStaticMesh::CreatePlane(mpResourceManager); } },
			{ "Sphere", [this] { CStaticMesh::CreateSphere(mpResourceManager); } }
		};

		auto it = builtInMeshesFactories.find(pResource->GetName());
		if (it != builtInMeshesFactories.cend())
		{
			(it->second)();
			return RC_OK;
		}

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

	TypeId CStaticMeshLoader::GetResourceTypeId() const
	{
		return IStaticMesh::GetTypeId();
	}


	TDE2_API IResourceLoader* CreateStaticMeshLoader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IResourceLoader, CStaticMeshLoader, result, pResourceManager, pGraphicsContext, pFileSystem);
	}


	CStaticMeshFactory::CStaticMeshFactory() :
		CBaseObject()
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

	TypeId CStaticMeshFactory::GetResourceTypeId() const
	{
		return IStaticMesh::GetTypeId();
	}


	TDE2_API IResourceFactory* CreateStaticMeshFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IResourceFactory, CStaticMeshFactory, result, pResourceManager, pGraphicsContext);
	}
}