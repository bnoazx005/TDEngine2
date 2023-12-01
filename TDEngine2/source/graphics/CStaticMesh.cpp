#include "../../include/graphics/CStaticMesh.h"
#include "../../include/core/IFileSystem.h"
#include "../../include/core/IFile.h"
#include "../../include/core/IJobManager.h"
#include "../../include/core/IGraphicsContext.h"
#include "../../include/graphics/IGraphicsObjectManager.h"
#include "../../include/graphics/CGeometryBuilder.h"
#include "../../include/utils/CFileLogger.h"
#include "../../include/editor/CPerfProfiler.h"
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

	const TPtr<IResourceLoader> CStaticMesh::_getResourceLoader()
	{
		return mpResourceManager->GetResourceLoader<IStaticMesh>();
	}


	IStaticMesh* CStaticMesh::CreateCube(IResourceManager* pResourceManager, IJobManager* pJobManager)
	{
		auto pCubeMeshResource = pResourceManager->GetResource<IStaticMesh>(pResourceManager->Create<IStaticMesh>("Cube", TMeshParameters{}));

		auto meshGenerationRoutine = [pCubeMeshResource, pJobManager](auto&&)
		{
			auto addVertex = [](auto&& pMesh, const TVector4& pos, const TVector2& texcoords, const TVector4& normal, const TVector4& tangent)
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
		};

		if (auto pResource = dynamic_cast<IResource*>(pCubeMeshResource.Get()))
		{
			if (E_RESOURCE_LOADING_POLICY::STREAMING != pResource->GetLoadingPolicy())
			{
				meshGenerationRoutine(TJobArgs{});
				return pCubeMeshResource.Get();
			}
		}

		pJobManager->SubmitJob(nullptr, meshGenerationRoutine);

		return pCubeMeshResource.Get();
	}

	IStaticMesh* CStaticMesh::CreatePlane(IResourceManager* pResourceManager, IJobManager* pJobManager)
	{
		auto pPlaneMeshResource = pResourceManager->GetResource<IStaticMesh>(pResourceManager->Create<IStaticMesh>("Plane", TMeshParameters{}));

		auto meshGenerationRoutine = [pPlaneMeshResource, pJobManager](auto&&)
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
		};

		if (auto pResource = dynamic_cast<IResource*>(pPlaneMeshResource.Get()))
		{
			if (E_RESOURCE_LOADING_POLICY::STREAMING != pResource->GetLoadingPolicy())
			{
				meshGenerationRoutine(TJobArgs{});
				return pPlaneMeshResource.Get();
			}
		}

		pJobManager->SubmitJob(nullptr, meshGenerationRoutine);

		return pPlaneMeshResource.Get();
	}

	IStaticMesh* CStaticMesh::CreateSphere(IResourceManager* pResourceManager, IJobManager* pJobManager)
	{
		constexpr F32 sphereRadius = 10.0f;
		
		auto pSphereMeshResource = pResourceManager->GetResource<IStaticMesh>(pResourceManager->Create<IStaticMesh>("Sphere", TMeshParameters{}));

		auto meshGenerationRoutine = [pSphereMeshResource, pJobManager, sphereRadius](auto&&)
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
		};

		if (auto pResource = dynamic_cast<IResource*>(pSphereMeshResource.Get()))
		{
			if (E_RESOURCE_LOADING_POLICY::STREAMING != pResource->GetLoadingPolicy())
			{
				meshGenerationRoutine(TJobArgs{});
				return pSphereMeshResource.Get();
			}
		}

		pJobManager->SubmitJob(nullptr, meshGenerationRoutine);

		return pSphereMeshResource.Get();
	}


	static IStaticMesh* CreateSkydome(IResourceManager* pResourceManager, IJobManager* pJobManager)
	{
		constexpr F32 sphereRadius = 2.0f;

		auto pSphereMeshResource = pResourceManager->GetResource<IStaticMesh>(pResourceManager->Create<IStaticMesh>("Skydome", TMeshParameters{}));

		auto meshGenerationRoutine = [pSphereMeshResource, pJobManager, sphereRadius](auto&&)
		{
			E_RESULT_CODE result = RC_OK;

			IGeometryBuilder* pGeometryBuilder = CreateGeometryBuilder(result);

			defer([&] { pGeometryBuilder->Free(); });

			if (result != RC_OK)
			{
				return;
			}

			const TVector3 basisVertices[]
			{
				TVector3(0.0f, sphereRadius, 0.0f),
				-TVector3(0.0f, sphereRadius, 0.0f),
				TVector3(sphereRadius, 0.0f, 0.0f),
				-TVector3(sphereRadius, 0.0f, 0.0f),
				TVector3(0.0f, 0.0f, sphereRadius),
				-TVector3(0.0f, 0.0f, sphereRadius),
			};

			auto tryFindIndex = [](auto&& vertices, const TVector3& v)
			{
				auto it = std::find(vertices.cbegin(), vertices.cend(), v);
				if (it == vertices.cend())
				{
					return std::numeric_limits<USIZE>::max();
				}

				return static_cast<USIZE>(std::distance(vertices.cbegin(), it));
			};

			std::function<void(std::vector<TVector3>&, std::vector<U32>&, const TVector3&, const TVector3&, const TVector3&, U16)> triangulateSphereSegment =
				[&triangulateSphereSegment, &tryFindIndex](auto&& vertices, auto&& indices, const TVector3& v0, const TVector3& v1, const TVector3& v2, U16 depth)
			{
				if (!depth)
				{
					USIZE i0 = tryFindIndex(vertices, v0);
					if (i0 >= vertices.size())
					{
						vertices.push_back(v0);
						i0 = vertices.size() - 1;
					}

					USIZE i1 = tryFindIndex(vertices, v1);
					if (i1 >= vertices.size())
					{
						vertices.push_back(v1);
						i1 = vertices.size() - 1;
					}

					USIZE i2 = tryFindIndex(vertices, v2);
					if (i2 >= vertices.size())
					{
						vertices.push_back(v2);
						i2 = vertices.size() - 1;
					}

					indices.push_back(static_cast<U32>(i0));
					indices.push_back(static_cast<U32>(i1));
					indices.push_back(static_cast<U32>(i2));

					std::tuple<TVector3, U32, U32> edges[3]
					{
						{ v1 - v0, static_cast<U32>(i0), static_cast<U32>(i1) }, 
						{ v2 - v1, static_cast<U32>(i2), static_cast<U32>(i1) }, 
						{ v0 - v2, static_cast<U32>(i0), static_cast<U32>(i2) }, 
					};

					// provide bottom fan generation
					for (U32 i = 0; i < 3; i++)
					{
						if (CMathUtils::Abs(std::get<0>(edges[i]).y) > FloatEpsilon || CMathUtils::Abs(vertices[std::get<1>(edges[i])].y) > FloatEpsilon)
						{
							continue;
						}

						indices.push_back(0);
						indices.push_back(std::get<1>(edges[i]));
						indices.push_back(std::get<2>(edges[i]));
					}

					return;
				}

				triangulateSphereSegment(vertices, indices, v0, Lerp(v0, v1, 0.5f), Lerp(v0, v2, 0.5f), depth - 1);
				triangulateSphereSegment(vertices, indices, v1, Lerp(v1, v0, 0.5f), Lerp(v1, v2, 0.5f), depth - 1);
				triangulateSphereSegment(vertices, indices, v2, Lerp(v2, v0, 0.5f), Lerp(v2, v1, 0.5f), depth - 1);
				triangulateSphereSegment(vertices, indices, Lerp(v0, v1, 0.5f), Lerp(v0, v2, 0.5f), Lerp(v1, v2, 0.5f), depth - 1);
			};

			U8 indices[][3]
			{
				{ 0, 2, 4 },
				{ 0, 2, 5 },
				{ 0, 3, 4 },
				{ 0, 3, 5 },
			};

			std::vector<TVector3> vertices
			{
				TVector3(0.0f, -sphereRadius, 0.0f)
			};

			std::vector<U32> finalIndices;

			for (const auto& currIndices : indices)
			{
				triangulateSphereSegment(vertices, finalIndices, basisVertices[currIndices[0]], basisVertices[currIndices[1]], basisVertices[currIndices[2]], 3);
			}

			for (U16 i = 0; i < vertices.size(); i++)
			{
				auto&& v0 = Normalize(vertices[i]);

				pSphereMeshResource->AddPosition(TVector4(v0 * sphereRadius, 1.0f));
				pSphereMeshResource->AddTexCoord0(TVector2(v0.x, v0.z) * 0.5f + TVector2(0.5f));
				pSphereMeshResource->AddNormal(TVector4(-v0, 0.0f));
				pSphereMeshResource->AddTangent({}); // \todo Implement correct computation of a tangent
			}

			U32 currFace[3];

			for (U16 i = 0; i < finalIndices.size(); i += 3)
			{
				currFace[0] = finalIndices[i];
				currFace[1] = finalIndices[i + 1];
				currFace[2] = finalIndices[i + 2];

				pSphereMeshResource->AddFace(currFace);
			}

			pJobManager->ExecuteInMainThread([pSphereMeshResource]
			{
				PANIC_ON_FAILURE(pSphereMeshResource->PostLoad());
			});
		};

		if (auto pResource = dynamic_cast<IResource*>(pSphereMeshResource.Get()))
		{
			if (E_RESOURCE_LOADING_POLICY::STREAMING != pResource->GetLoadingPolicy())
			{
				meshGenerationRoutine(TJobArgs{});
				return pSphereMeshResource.Get();
			}
		}

		pJobManager->SubmitJob(nullptr, meshGenerationRoutine);

		return pSphereMeshResource.Get();
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
		TDE2_PROFILER_SCOPE("CStaticMeshLoader::LoadResource");

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
			{ "Sphere", [this, pJobManager] { CStaticMesh::CreateSphere(mpResourceManager, pJobManager); } },
			{ "Skydome", [this, pJobManager] { CreateSkydome(mpResourceManager, pJobManager); } }
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

		auto loadMeshRoutine = [pJobManager, pMeshFileReader, pResource](auto&&)
		{
			TDE2_PROFILER_SCOPE("LoadStaticMeshJob");

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
			loadMeshRoutine(TJobArgs{});
			return RC_OK;
		}

		pJobManager->SubmitJob(nullptr, loadMeshRoutine);

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