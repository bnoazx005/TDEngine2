#include "../../include/graphics/CSkinnedMesh.h"
#include "../../include/core/IFileSystem.h"
#include "../../include/core/IFile.h"
#include "../../include/core/IJobManager.h"
#include "../../include/core/IGraphicsContext.h"
#include "../../include/graphics/IGraphicsObjectManager.h"
#include "../../include/graphics/CGeometryBuilder.h"
#include "../../include/utils/CFileLogger.h"
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

	bool CSkinnedMesh::HasJointWeights() const
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _hasJointWeightsInternal();
	}

	bool CSkinnedMesh::HasJointIndices() const
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _hasJointIndicesInternal();
	}

	E_RESULT_CODE CSkinnedMesh::_initPositionOnlyVertexBuffer()
	{
		auto&& positions = _toPositionOnlyArray();

		auto positionOnlyVertexBufferResult = mpGraphicsObjectManager->CreateVertexBuffer(BUT_STATIC, positions.size(), &positions.front());
		if (positionOnlyVertexBufferResult.HasError())
		{
			return positionOnlyVertexBufferResult.GetError();
		}

		mpPositionOnlyVertexBuffer = positionOnlyVertexBufferResult.Get();

		return RC_OK;
	}

	// \todo Maybe some refactoring is needed
	std::vector<U8> CSkinnedMesh::_toPositionOnlyArray() const
	{
		U32 strideSize = sizeof(TVector4);
		strideSize += (_hasJointWeightsInternal() ? sizeof(TVector4) : 0);
		strideSize += (_hasJointIndicesInternal() ? sizeof(U32) * 4 : 0);

		std::vector<U8> bytes(mPositions.size() * strideSize);

		U32 elementsCount = 0;

		for (U32 i = 0, ptrPos = 0; i < mPositions.size(); ++i, ptrPos += strideSize)
		{
			// mandatory element
			memcpy(&bytes[ptrPos], &mPositions[i], sizeof(TVector4));

			elementsCount = 1;

			if (_hasJointWeightsInternal()) { memcpy(&bytes[ptrPos + elementsCount++ * sizeof(TVector4)], &mJointsWeights[i], sizeof(TVector4)); }
			if (_hasJointIndicesInternal()) { memcpy(&bytes[ptrPos + elementsCount++ * 4 * sizeof(U32)], &mJointsIndices[i], 4 * sizeof(U32)); }
		}

		return bytes;
	}

	std::vector<U8> CSkinnedMesh::_toArrayOfStructsDataLayoutInternal() const
	{
		U32 strideSize = sizeof(TVector4) + sizeof(TColor32F);
		strideSize += (_hasTexCoords0Internal() ? sizeof(TVector4) : 0); // \note texcoords use float2, but we align them manually to float4
		strideSize += (_hasNormalsInternal() ? sizeof(TVector4) : 0);
		strideSize += (_hasTangentsInternal() ? sizeof(TVector4) : 0);
		strideSize += (_hasJointWeightsInternal() ? sizeof(TVector4) : 0);
		strideSize += (_hasJointIndicesInternal() ? sizeof(U32) * 4 : 0);

		std::vector<U8> bytes(mPositions.size() * strideSize);

		U32 elementsCount = 0;

		for (U32 i = 0, ptrPos = 0; i < mPositions.size(); ++i, ptrPos += strideSize)
		{
			// mandatory element
			memcpy(&bytes[ptrPos], &mPositions[i], sizeof(TVector4));
			memcpy(&bytes[ptrPos + sizeof(TVector4)], _hasColorsInternal() ? &mVertexColors[i] : &TColorUtils::mWhite, sizeof(TColor32F));

			elementsCount = 2; // \note equals to 2 because of position and color are mandatory elements of a vertex declaration

			if (_hasTexCoords0Internal()) { memcpy(&bytes[ptrPos + elementsCount++ * sizeof(TVector4)], &mTexcoords0[i], sizeof(TVector2)); }
			if (_hasNormalsInternal()) { memcpy(&bytes[ptrPos + elementsCount++ * sizeof(TVector4)], &mNormals[i], sizeof(TVector4)); }
			if (_hasTangentsInternal()) { memcpy(&bytes[ptrPos + elementsCount++ * sizeof(TVector4)], &mTangents[i], sizeof(TVector4)); }
			if (_hasJointWeightsInternal()) { memcpy(&bytes[ptrPos + elementsCount++ * sizeof(TVector4)], &mJointsWeights[i], sizeof(TVector4)); }
			if (_hasJointIndicesInternal()) { memcpy(&bytes[ptrPos + elementsCount++ * 4 * sizeof(U32)], &mJointsIndices[i], 4 * sizeof(U32)); }
		}

		return bytes;
	}


	const TPtr<IResourceLoader> CSkinnedMesh::_getResourceLoader()
	{
		return mpResourceManager->GetResourceLoader<ISkinnedMesh>();
	}

	bool CSkinnedMesh::_hasJointWeightsInternal() const
	{
		return !mJointsWeights.empty();
	}

	bool CSkinnedMesh::_hasJointIndicesInternal() const
	{
		return !mJointsIndices.empty();
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

		if (E_RESOURCE_LOADING_POLICY::SYNCED == pResource->GetLoadingPolicy())
		{
			loadMeshRoutine();
			return RC_OK;
		}

		pJobManager->SubmitJob(std::function<void()>(loadMeshRoutine));

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