#include "./../../include/graphics/CStaticMesh.h"


namespace TDEngine2
{
	CStaticMesh::CStaticMesh() :
		CBaseComponent()
	{
	}

	E_RESULT_CODE CStaticMesh::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
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


	IComponent* CreateStaticMesh(E_RESULT_CODE& result)
	{
		CStaticMesh* pStaticMeshInstance = new (std::nothrow) CStaticMesh();

		if (!pStaticMeshInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pStaticMeshInstance->Init();

		if (result != RC_OK)
		{
			delete pStaticMeshInstance;

			pStaticMeshInstance = nullptr;
		}

		return pStaticMeshInstance;
	}


	CStaticMeshFactory::CStaticMeshFactory() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CStaticMeshFactory::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

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

	IComponent* CStaticMeshFactory::Create(const TBaseComponentParameters* pParams) const
	{
		if (!pParams)
		{
			return nullptr;
		}

		const TStaticMeshParameters* transformParams = static_cast<const TStaticMeshParameters*>(pParams);

		E_RESULT_CODE result = RC_OK;

		return CreateStaticMesh(result);
	}

	IComponent* CStaticMeshFactory::CreateDefault(const TBaseComponentParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		return CreateStaticMesh(result);
	}

	TypeId CStaticMeshFactory::GetComponentTypeId() const
	{
		return CStaticMesh::GetTypeId();
	}


	IComponentFactory* CreateStaticMeshFactory(E_RESULT_CODE& result)
	{
		CStaticMeshFactory* pStaticMeshFactoryInstance = new (std::nothrow) CStaticMeshFactory();

		if (!pStaticMeshFactoryInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pStaticMeshFactoryInstance->Init();

		if (result != RC_OK)
		{
			delete pStaticMeshFactoryInstance;

			pStaticMeshFactoryInstance = nullptr;
		}

		return pStaticMeshFactoryInstance;
	}
}