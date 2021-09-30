#include "../../include/graphics/CStaticMesh.h"
#include "../../include/core/IFileSystem.h"
#include "../../include/core/IFile.h"
#include "../../include/core/IJobManager.h"
#include "../../include/core/IGraphicsContext.h"
#include "../../include/graphics/IGraphicsObjectManager.h"
#include "../../include/graphics/CGeometryBuilder.h"
#include "../../include/utils/CFileLogger.h"
#include "deferOperation.hpp"
#include <cstring>
#include <climits>


namespace TDEngine2
{
	CStaticMesh::CStaticMesh() :
		CBaseMesh()
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

		mLoadingPolicy = params.mLoadingPolicy;

		mpGraphicsObjectManager = pGraphicsContext->GetGraphicsObjectManager();

		mState = E_RESOURCE_STATE_TYPE::RST_LOADED;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CStaticMesh::Accept(IBinaryMeshFileReader* pReader)
	{
		if (!pReader)
		{
			return RC_INVALID_ARGS;
		}

		return pReader->LoadStaticMesh(this);
	}

	const IResourceLoader* CStaticMesh::_getResourceLoader()
	{
		return mpResourceManager->GetResourceLoader<IStaticMesh>();
	}


	IStaticMesh* CStaticMesh::CreateCube(IResourceManager* pResourceManager, IJobManager* pJobManager)
	{
		auto pCubeMeshResource = pResourceManager->GetResource<IStaticMesh>(pResourceManager->Create<IStaticMesh>("Cube", TMeshParameters{}));

		auto meshGenerationRoutine = std::function<void()>([pCubeMeshResource, pJobManager]
		{
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

			pJobManager->ExecuteInMainThread([pCubeMeshResource]
			{
				PANIC_ON_FAILURE(pCubeMeshResource->PostLoad());
			});
		});

		if (auto pResource = dynamic_cast<IResource*>(pCubeMeshResource))
		{
			if (E_RESOURCE_LOADING_POLICY::STREAMING != pResource->GetLoadingPolicy())
			{
				meshGenerationRoutine();
				return pCubeMeshResource;
			}
		}

		pJobManager->SubmitJob(meshGenerationRoutine);

		return pCubeMeshResource;
	}

	IStaticMesh* CStaticMesh::CreatePlane(IResourceManager* pResourceManager, IJobManager* pJobManager)
	{
		auto pPlaneMeshResource = pResourceManager->GetResource<IStaticMesh>(pResourceManager->Create<IStaticMesh>("Plane", TMeshParameters{}));

		auto meshGenerationRoutine = std::function<void()>([pPlaneMeshResource, pJobManager]
		{
			E_RESULT_CODE result = RC_OK;

			IGeometryBuilder* pGeometryBuilder = CreateGeometryBuilder(result);

			defer([&] { pGeometryBuilder->Free(); });

			if (result != RC_OK)
			{
				return;
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

			pJobManager->ExecuteInMainThread([pPlaneMeshResource]
			{
				PANIC_ON_FAILURE(pPlaneMeshResource->PostLoad());
			});
		});

		if (auto pResource = dynamic_cast<IResource*>(pPlaneMeshResource))
		{
			if (E_RESOURCE_LOADING_POLICY::STREAMING != pResource->GetLoadingPolicy())
			{
				meshGenerationRoutine();
				return pPlaneMeshResource;
			}
		}

		pJobManager->SubmitJob(meshGenerationRoutine);

		return pPlaneMeshResource;
	}

	IStaticMesh* CStaticMesh::CreateSphere(IResourceManager* pResourceManager, IJobManager* pJobManager)
	{
		constexpr F32 sphereRadius = 10.0f;
		
		auto pSphereMeshResource = pResourceManager->GetResource<IStaticMesh>(pResourceManager->Create<IStaticMesh>("Sphere", TMeshParameters{}));

		auto meshGenerationRoutine = std::function<void()>([pSphereMeshResource, pJobManager, sphereRadius]
		{
			E_RESULT_CODE result = RC_OK;

			IGeometryBuilder* pGeometryBuilder = CreateGeometryBuilder(result);

			defer([&] { pGeometryBuilder->Free(); });

			if (result != RC_OK)
			{
				return;
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

			pJobManager->ExecuteInMainThread([pSphereMeshResource]
			{
				PANIC_ON_FAILURE(pSphereMeshResource->PostLoad());
			});
		});

		if (auto pResource = dynamic_cast<IResource*>(pSphereMeshResource))
		{
			if (E_RESOURCE_LOADING_POLICY::STREAMING != pResource->GetLoadingPolicy())
			{
				meshGenerationRoutine();
				return pSphereMeshResource;
			}
		}

		pJobManager->SubmitJob(meshGenerationRoutine);

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

		result = pMeshInstance->Init(pResourceManager, pGraphicsContext, name, params);

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

	E_RESULT_CODE CStaticMeshLoader::LoadResource(IResource* pResource) const
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;
		
		IJobManager* pJobManager = mpFileSystem->GetJobManager();

		// \todo Refactor this later (may be the declaration should be placed somewhere else
		static const std::unordered_map<std::string, std::function<void()>> builtInMeshesFactories
		{
			{ "Cube", [this, pJobManager] { CStaticMesh::CreateCube(mpResourceManager, pJobManager); } },
			{ "Plane", [this, pJobManager] {  CStaticMesh::CreatePlane(mpResourceManager, pJobManager); } },
			{ "Sphere", [this, pJobManager] { CStaticMesh::CreateSphere(mpResourceManager, pJobManager); } }
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

		IBinaryMeshFileReader* pMeshFileReader = mpFileSystem->Get<IBinaryMeshFileReader>(meshFileId.Get());		

		auto loadMeshRoutine = [pJobManager, pMeshFileReader, pResource]
		{
			E_RESULT_CODE result = RC_OK;

			IMesh* pMesh = dynamic_cast<IMesh*>(pResource);

			if (RC_OK != (result = pMeshFileReader->LoadMesh(pMesh)))
			{
				TDE2_ASSERT(false);
				return;
			}

			pJobManager->ExecuteInMainThread([pMesh, pResource]
			{
				E_RESULT_CODE result = RC_OK;
				
				if (RC_OK != (result = pMesh->PostLoad()))
				{
					TDE2_ASSERT(false);
					return;
				}

				pResource->SetState(E_RESOURCE_STATE_TYPE::RST_LOADED);
			});
		};
		
		if (E_RESOURCE_LOADING_POLICY::STREAMING != pResource->GetLoadingPolicy())
		{
			loadMeshRoutine();
			return RC_OK;
		}

		pJobManager->SubmitJob(std::function<void()>(loadMeshRoutine));

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

	IResource* CStaticMeshFactory::Create(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		const TMeshParameters& meshParams = dynamic_cast<const TMeshParameters&>(params);

		return dynamic_cast<IResource*>(CreateStaticMesh(mpResourceManager, mpGraphicsContext, name, meshParams, result));
	}

	IResource* CStaticMeshFactory::CreateDefault(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		TMeshParameters meshParams;
		meshParams.mLoadingPolicy = params.mLoadingPolicy;

		return dynamic_cast<IResource*>(CreateStaticMesh(mpResourceManager, mpGraphicsContext, name, meshParams, result));
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