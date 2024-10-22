#include "../../include/graphics/CSkinnedMesh.h"
#include "../../include/core/IFileSystem.h"
#include "../../include/core/IFile.h"
#include "../../include/core/IJobManager.h"
#include "../../include/core/IGraphicsContext.h"
#include "../../include/graphics/IGraphicsObjectManager.h"
#include "../../include/graphics/CGeometryBuilder.h"
#include "../../include/utils/CFileLogger.h"
#include "../../include/editor/CPerfProfiler.h"
#include <cstring>
#include <climits>


namespace TDEngine2
{
	CSkinnedMesh::CSkinnedMesh() :
		CBaseMesh()
	{
	}

	E_RESULT_CODE CSkinnedMesh::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name)
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

	E_RESULT_CODE CSkinnedMesh::PostLoad()
	{
		E_RESULT_CODE result = CBaseMesh::PostLoad();
		if (RC_OK != result)
		{
			return result;
		}

		std::lock_guard<std::mutex> lock(mMutex);

		if (!_hasVertexStreamInternal(E_VERTEX_STREAM_TYPE::SKINNING))
		{
			return RC_OK;
		}

		const USIZE totalBufferSize  = sizeof(TJointsWeightsArray) * mJointsWeights.size() + sizeof(TJointsIndicesArray) * mJointsIndices.size();
		const USIZE bufferStrideSize = sizeof(TJointsWeightsArray) + sizeof(TJointsIndicesArray);

		struct TSkinningVertexData
		{
			TJointsWeightsArray mWeights;
			TJointsIndicesArray mIndices;
		};

		std::vector<TSkinningVertexData> skinningDataBuffer(mJointsWeights.size());

		USIZE index = 0;

		for (USIZE i = 0; i < mJointsWeights.size(); ++i)
		{
			std::copy(mJointsWeights[i].begin(), mJointsWeights[i].end(), skinningDataBuffer[index].mWeights.begin());
			std::copy(mJointsIndices[i].begin(), mJointsIndices[i].end(), skinningDataBuffer[index].mIndices.begin());

			++index;
		}

		auto createBufferResult = mpGraphicsObjectManager->CreateBuffer(
			{
				E_BUFFER_USAGE_TYPE::STATIC,
				E_BUFFER_TYPE::STRUCTURED,
				totalBufferSize,
				skinningDataBuffer.data(),
				totalBufferSize,
				false,
				bufferStrideSize,
				E_STRUCTURED_BUFFER_TYPE::DEFAULT
			});

		if (createBufferResult.HasError())
		{
			return createBufferResult.GetError();
		}

		mVertexStreams[static_cast<U32>(E_VERTEX_STREAM_TYPE::SKINNING)] = createBufferResult.Get();

		return RC_OK;
	}

	E_RESULT_CODE CSkinnedMesh::Accept(IBinaryMeshFileReader* pReader)
	{
		if (!pReader)
		{
			return RC_INVALID_ARGS;
		}

		return pReader->LoadSkinnedMesh(this);
	}

	void CSkinnedMesh::AddVertexJointWeights(const TJointsWeightsArray& weights)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		mJointsWeights.push_back(weights);
	}

	void CSkinnedMesh::AddVertexJointIndices(const TJointsIndicesArray& indices)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		mJointsIndices.push_back(indices);
	}

	const std::vector<CSkinnedMesh::TJointsWeightsArray>& CSkinnedMesh::GetJointWeightsArray() const
	{		
		std::lock_guard<std::mutex> lock(mMutex);
		return mJointsWeights;
	}

	const std::vector<CSkinnedMesh::TJointsIndicesArray>& CSkinnedMesh::GetJointIndicesArray() const
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return mJointsIndices;
	}

	const TPtr<IResourceLoader> CSkinnedMesh::_getResourceLoader()
	{
		return mpResourceManager->GetResourceLoader<ISkinnedMesh>();
	}

	bool CSkinnedMesh::_hasVertexStreamInternal(E_VERTEX_STREAM_TYPE streamType) const
	{
		switch (streamType)
		{
			case E_VERTEX_STREAM_TYPE::SKINNING:
				return mJointsIndices.size() > 0 && mJointsWeights.size() > 0;
		}

		return CBaseMesh::_hasVertexStreamInternal(streamType);
	}


	TDE2_API ISkinnedMesh* CreateSkinnedMesh(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISkinnedMesh, CSkinnedMesh, result, pResourceManager, pGraphicsContext, name);
	}


	CSkinnedMeshLoader::CSkinnedMeshLoader() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CSkinnedMeshLoader::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem)
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

	E_RESULT_CODE CSkinnedMeshLoader::LoadResource(IResource* pResource) const
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;
		
		IJobManager* pJobManager = mpFileSystem->GetJobManager();
		
		TResult<TFileEntryId> meshFileId = mpFileSystem->Open<IBinaryMeshFileReader>(pResource->GetName());
		if (meshFileId.HasError())
		{
			LOG_WARNING(std::string("[Mesh Loader] Could not load the specified mesh file (").append(pResource->GetName()).append("), load default one instead..."));
			return RC_FAIL;
		}

		IBinaryMeshFileReader* pMeshFileReader = mpFileSystem->Get<IBinaryMeshFileReader>(meshFileId.Get());		

		auto loadMeshRoutine = [pJobManager, pMeshFileReader, pResource](auto&&)
		{
			TDE2_PROFILER_SCOPE("LoadSkinnedMeshJob");

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

		if (E_RESOURCE_LOADING_POLICY::SYNCED == pResource->GetLoadingPolicy())
		{
			loadMeshRoutine(TJobArgs{});
			return RC_OK;
		}

		pJobManager->SubmitJob(nullptr, loadMeshRoutine);

		return RC_OK;
	}

	TypeId CSkinnedMeshLoader::GetResourceTypeId() const
	{
		return ISkinnedMesh::GetTypeId();
	}


	TDE2_API IResourceLoader* CreateSkinnedMeshLoader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IResourceLoader, CSkinnedMeshLoader, result, pResourceManager, pGraphicsContext, pFileSystem);
	}


	CSkinnedMeshFactory::CSkinnedMeshFactory() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CSkinnedMeshFactory::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext)
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

	IResource* CSkinnedMeshFactory::Create(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		const TMeshParameters& meshParams = dynamic_cast<const TMeshParameters&>(params);

		return dynamic_cast<IResource*>(CreateSkinnedMesh(mpResourceManager, mpGraphicsContext, name, result));
	}

	IResource* CSkinnedMeshFactory::CreateDefault(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		if (auto pResource = dynamic_cast<IResource*>(CreateSkinnedMesh(mpResourceManager, mpGraphicsContext, name, result)))
		{
			pResource->SetLoadingPolicy(params.mLoadingPolicy);
			return pResource;
		}

		return nullptr;
	}

	TypeId CSkinnedMeshFactory::GetResourceTypeId() const
	{
		return ISkinnedMesh::GetTypeId();
	}


	TDE2_API IResourceFactory* CreateSkinnedMeshFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IResourceFactory, CSkinnedMeshFactory, result, pResourceManager, pGraphicsContext);
	}
}