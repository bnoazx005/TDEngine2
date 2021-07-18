#include "../../include/graphics/CBaseMesh.h"
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

		auto vertexBufferResult = mpGraphicsObjectManager->CreateVertexBuffer(BUT_STATIC, vertices.size(), &vertices.front());
		if (vertexBufferResult.HasError())
		{
			return vertexBufferResult.GetError();
		}

		mpSharedVertexBuffer = vertexBufferResult.Get();

		// \note create a position-only vertex buffer
		// \todo In future may be it's better to split shared VB into separate channels

		auto positionOnlyVertexBufferResult = mpGraphicsObjectManager->CreateVertexBuffer(BUT_STATIC, mPositions.size() * sizeof(TVector4), &mPositions.front());
		if (positionOnlyVertexBufferResult.HasError())
		{
			return positionOnlyVertexBufferResult.GetError();
		}

		mpPositionOnlyVertexBuffer = positionOnlyVertexBufferResult.Get();

		E_INDEX_FORMAT_TYPE indexFormatType = (mIndices.size() < (std::numeric_limits<U16>::max)()) ? IFT_INDEX16 : IFT_INDEX32;

		std::vector<U8> indices = _getIndicesArray(indexFormatType);

		auto indexBufferResult = mpGraphicsObjectManager->CreateIndexBuffer(BUT_STATIC, indexFormatType, static_cast<U32>(indices.size()), &indices[0]);
		if (indexBufferResult.HasError())
		{
			return indexBufferResult.GetError();
		}

		mpSharedIndexBuffer = indexBufferResult.Get();

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
		return mIndices.size() / 3;
	}

	IVertexBuffer* CBaseMesh::GetSharedVertexBuffer() const
	{
		return mpSharedVertexBuffer;
	}

	IVertexBuffer* CBaseMesh::GetPositionOnlyVertexBuffer() const
	{
		return mpPositionOnlyVertexBuffer;
	}

	IIndexBuffer* CBaseMesh::GetSharedIndexBuffer() const
	{
		return mpSharedIndexBuffer;
	}

	std::vector<U8> CBaseMesh::_getIndicesArray(const E_INDEX_FORMAT_TYPE& indexFormat) const
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