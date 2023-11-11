#include "../../include/graphics/CBaseMesh.h"
#include "../../include/core/IFileSystem.h"
#include "../../include/core/IFile.h"
#include "../../include/core/IJobManager.h"
#include "../../include/core/IGraphicsContext.h"
#include "../../include/graphics/IGraphicsObjectManager.h"
#include "../../include/graphics/CGeometryBuilder.h"
#include "../../include/graphics/IBuffer.h"
#include "../../include/utils/CFileLogger.h"
#include <cstring>
#include <climits>


namespace TDEngine2
{
	CBaseMesh::CBaseMesh() :
		CBaseResource()
	{
	}

	E_RESULT_CODE CBaseMesh::PostLoad()
	{
		std::lock_guard<std::mutex> lock(mMutex);

		// create shared buffers for the mesh
		auto&& vertices = _toArrayOfStructsDataLayoutInternal();
		if (vertices.empty())
		{
			TDE2_ASSERT(false);
			return RC_FAIL;
		}

		auto pGraphicsContext = mpGraphicsObjectManager->GetGraphicsContext();

		auto vertexBufferResult = mpGraphicsObjectManager->CreateBuffer({ E_BUFFER_USAGE_TYPE::STATIC, E_BUFFER_TYPE::BT_VERTEX_BUFFER, vertices.size(), &vertices.front() });
		if (vertexBufferResult.HasError())
		{
			return vertexBufferResult.GetError();
		}

		mSharedVertexBufferHandle = vertexBufferResult.Get();

		// \note create a position-only vertex buffer
		// \todo In future may be it's better to split shared VB into separate channels

		E_RESULT_CODE result = _initPositionOnlyVertexBuffer();
		if (RC_OK != result)
		{
			return result;
		}

		E_INDEX_FORMAT_TYPE indexFormatType = (mIndices.size() < (std::numeric_limits<U16>::max)()) ? E_INDEX_FORMAT_TYPE::INDEX16 : E_INDEX_FORMAT_TYPE::INDEX32;

		std::vector<U8> indices = _getIndicesArray(indexFormatType);

		TInitBufferParams indexBufferCreateParams{ E_BUFFER_USAGE_TYPE::STATIC, E_BUFFER_TYPE::BT_INDEX_BUFFER, static_cast<U32>(indices.size()), &indices[0] };
		indexBufferCreateParams.mIndexFormat = indexFormatType;

		auto indexBufferResult = mpGraphicsObjectManager->CreateBuffer(indexBufferCreateParams);
		if (indexBufferResult.HasError())
		{
			return indexBufferResult.GetError();
		}

		mSharedIndexBufferHandle = indexBufferResult.Get();

		{
			mSubMeshesIdentifiers.emplace_back(Wrench::StringUtils::GetEmptyStr());
			mSubMeshesInfo.push_back({ 0, static_cast<U32>(mIndices.size()) });
		}

		SetState(E_RESOURCE_STATE_TYPE::RST_LOADED);

		return RC_OK;
	}

	E_RESULT_CODE CBaseMesh::Reset()
	{
		return RC_NOT_IMPLEMENTED_YET;
	}

	void CBaseMesh::AddPosition(const TVector4& pos)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		mPositions.emplace_back(pos);
	}

	void CBaseMesh::AddColor(const TColor32F& color)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		mVertexColors.emplace_back(color);
	}

	void CBaseMesh::AddNormal(const TVector4& normal)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		mNormals.emplace_back(normal);
	}

	void CBaseMesh::AddTangent(const TVector4& tangent)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		mTangents.emplace_back(tangent);
	}

	void CBaseMesh::AddTexCoord0(const TVector2& uv0)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		mTexcoords0.emplace_back(uv0);
	}

	void CBaseMesh::AddFace(const U32 face[3])
	{
		std::lock_guard<std::mutex> lock(mMutex);

		mIndices.push_back(face[0]);
		mIndices.push_back(face[1]);
		mIndices.push_back(face[2]);
	}

	void CBaseMesh::AddSubMeshInfo(const std::string& subMeshId, const TSubMeshRenderInfo& info)
	{
		TDE2_ASSERT(!subMeshId.empty());

		std::lock_guard<std::mutex> lock(mMutex);

		mSubMeshesIdentifiers.emplace_back(subMeshId);
		mSubMeshesInfo.emplace_back(info);
	}

	const CBaseMesh::TPositionsArray& CBaseMesh::GetPositionsArray() const
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return mPositions;
	}

	const CBaseMesh::TVertexColorArray& CBaseMesh::GetColorsArray() const
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return mVertexColors;
	}

	const CBaseMesh::TNormalsArray& CBaseMesh::GetNormalsArray() const
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return mNormals;
	}

	const CBaseMesh::TTangentsArray& CBaseMesh::GetTangentsArray() const
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return mTangents;
	}

	const CBaseMesh::TTexcoordsArray& CBaseMesh::GetTexCoords0Array() const
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return mTexcoords0;
	}

	const CBaseMesh::TIndicesArray& CBaseMesh::GetIndices() const
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return mIndices;
	}

	bool CBaseMesh::HasColors() const
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _hasColorsInternal();
	}

	bool CBaseMesh::HasNormals() const
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _hasNormalsInternal();
	}

	bool CBaseMesh::HasTangents() const
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _hasTangentsInternal();
	}

	bool CBaseMesh::HasTexCoords0() const
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _hasTexCoords0Internal();
	}

	std::vector<U8> CBaseMesh::ToArrayOfStructsDataLayout() const
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return std::move(_toArrayOfStructsDataLayoutInternal());
	}

	U32 CBaseMesh::GetFacesCount() const
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return static_cast<U32>(mIndices.size()) / 3;
	}

	const TSubMeshRenderInfo& CBaseMesh::GetSubmeshInfo(const std::string& subMeshId) const
	{
		std::lock_guard<std::mutex> lock(mMutex);

		static TSubMeshRenderInfo invalid;

		auto it = std::find(mSubMeshesIdentifiers.cbegin(), mSubMeshesIdentifiers.cend(), subMeshId);
		return it == mSubMeshesIdentifiers.cend() ? invalid : mSubMeshesInfo[std::distance(mSubMeshesIdentifiers.cbegin(), it)];
	}

	TBufferHandleId CBaseMesh::GetSharedVertexBuffer() const
	{
		return mSharedVertexBufferHandle;
	}

	TBufferHandleId CBaseMesh::GetPositionOnlyVertexBuffer() const
	{
		return mPositionOnlyVertexBufferHandle;
	}

	TBufferHandleId CBaseMesh::GetSharedIndexBuffer() const
	{
		return mSharedIndexBufferHandle;
	}

	const std::vector<std::string> CBaseMesh::GetSubmeshesIdentifiers() const
	{
		return mSubMeshesIdentifiers;
	}

	std::vector<U8> CBaseMesh::_getIndicesArray(const E_INDEX_FORMAT_TYPE& indexFormat) const
	{
		std::vector<U8> indicesBytesArray(static_cast<U32>(indexFormat) * mIndices.size());

		U8* pPtr = &indicesBytesArray[0];

		for (auto&& currIndex : mIndices)
		{
			switch (indexFormat)
			{
				case E_INDEX_FORMAT_TYPE::INDEX16:
					*reinterpret_cast<U16*>(pPtr) = currIndex;
					break;
				case E_INDEX_FORMAT_TYPE::INDEX32:
					*reinterpret_cast<U32*>(pPtr) = currIndex;
					break;
			}

			pPtr += static_cast<U32>(indexFormat);
		}

		return indicesBytesArray;
	}

	E_RESULT_CODE CBaseMesh::_initPositionOnlyVertexBuffer()
	{
		auto positionOnlyVertexBufferResult = mpGraphicsObjectManager->CreateBuffer({ E_BUFFER_USAGE_TYPE::STATIC, E_BUFFER_TYPE::BT_VERTEX_BUFFER, mPositions.size() * sizeof(TVector4), &mPositions.front() });
		if (positionOnlyVertexBufferResult.HasError())
		{
			return positionOnlyVertexBufferResult.GetError();
		}

		mPositionOnlyVertexBufferHandle = positionOnlyVertexBufferResult.Get();

		return RC_OK;
	}

	std::vector<U8> CBaseMesh::_toArrayOfStructsDataLayoutInternal() const
	{
		U32 strideSize = sizeof(TVector4) + sizeof(TColor32F);
		strideSize += (_hasTexCoords0Internal() ? sizeof(TVector4) : 0); // \note texcoords use float2, but we align them manually to float4
		strideSize += (_hasNormalsInternal() ? sizeof(TVector4) : 0);
		strideSize += (_hasTangentsInternal() ? sizeof(TVector4) : 0);

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
		}

		return bytes;
	}

	bool CBaseMesh::_hasColorsInternal() const
	{
		return mVertexColors.size();
	}

	bool CBaseMesh::_hasNormalsInternal() const
	{
		return mNormals.size();
	}

	bool CBaseMesh::_hasTangentsInternal() const
	{
		return mTangents.size();
	}

	bool CBaseMesh::_hasTexCoords0Internal() const
	{
		return mTexcoords0.size();
	}
}