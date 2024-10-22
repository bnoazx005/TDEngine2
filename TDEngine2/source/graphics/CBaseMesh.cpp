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
#include <tuple>


namespace TDEngine2
{
	CBaseMesh::CBaseMesh() :
		CBaseResource()
	{
	}

	E_RESULT_CODE CBaseMesh::PostLoad()
	{
		std::lock_guard<std::mutex> lock(mMutex);

		const std::array<std::tuple<const U8*, USIZE, USIZE>, static_cast<USIZE>(E_VERTEX_STREAM_TYPE::SKINNING)> buffersCreateInfo
		{
			std::make_tuple(reinterpret_cast<const U8*>(mPositions.data()), sizeof(TVector4) * mPositions.size(), sizeof(TVector4)),
			std::make_tuple(reinterpret_cast<const U8*>(mVertexColors.data()), sizeof(TColor32F) * mVertexColors.size(), sizeof(TColor32F)),
			std::make_tuple(reinterpret_cast<const U8*>(mTexcoords0.data()), sizeof(TVector4) * mTexcoords0.size(), sizeof(TVector2)),
			std::make_tuple(reinterpret_cast<const U8*>(mNormals.data()), sizeof(TVector4) * mNormals.size(), sizeof(TVector4)),
			std::make_tuple(reinterpret_cast<const U8*>(mTangents.data()), sizeof(TVector4) * mTangents.size(), sizeof(TVector4))
		};

		for (I32 currVertexStreamIndex = 0; currVertexStreamIndex < static_cast<U32>(E_VERTEX_STREAM_TYPE::SKINNING); ++currVertexStreamIndex)
		{
			USIZE totalBufferSize = 0;
			USIZE bufferStrideSize = 0;
			const U8* pDataPtr = nullptr;

			std::tie(pDataPtr, totalBufferSize, bufferStrideSize) = buffersCreateInfo[currVertexStreamIndex];

			if (!totalBufferSize) /// \note Skip empty streams
			{
				mVertexStreams[currVertexStreamIndex] = TBufferHandleId::Invalid;
				continue;
			}

			auto createBufferResult = mpGraphicsObjectManager->CreateBuffer(
				{
					E_BUFFER_USAGE_TYPE::STATIC,
					E_BUFFER_TYPE::STRUCTURED,
					totalBufferSize,
					pDataPtr,
					totalBufferSize,
					false,
					bufferStrideSize,
					E_STRUCTURED_BUFFER_TYPE::DEFAULT
				});

			if (createBufferResult.HasError())
			{
				return createBufferResult.GetError();
			}

			mVertexStreams[currVertexStreamIndex] = createBufferResult.Get();
		}

		// \todo In future may be it's better to split shared VB into separate channels

		std::vector<U8> indices = _getIndicesArray(E_INDEX_FORMAT_TYPE::INDEX32);

		TInitBufferParams indexBufferCreateParams
		{ 
			E_BUFFER_USAGE_TYPE::STATIC, 
			E_BUFFER_TYPE::STRUCTURED, 
			static_cast<U32>(indices.size()), 
			&indices[0],
			static_cast<U32>(indices.size()),
			false,
			sizeof(U32),
			E_STRUCTURED_BUFFER_TYPE::DEFAULT
		};

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
		mTexcoords0.emplace_back(TVector4(uv0.x, uv0.y, 0.0f, 0.0f));
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

	bool CBaseMesh::HasVertexStream(E_VERTEX_STREAM_TYPE streamType) const
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _hasVertexStreamInternal(streamType);
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

	TBufferHandleId CBaseMesh::GetVertexBufferForStream(E_VERTEX_STREAM_TYPE streamType) const
	{
		const USIZE index = static_cast<USIZE>(streamType);
		return (index >= mVertexStreams.size()) ? TBufferHandleId::Invalid : mVertexStreams[index];
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

	bool CBaseMesh::_hasVertexStreamInternal(E_VERTEX_STREAM_TYPE streamType) const
	{
		switch (streamType)
		{
			case E_VERTEX_STREAM_TYPE::POSITIONS:
				return mPositions.size() > 0;
			case E_VERTEX_STREAM_TYPE::COLORS:
				return mVertexColors.size() > 0;
			case E_VERTEX_STREAM_TYPE::TEXCOORDS:
				return mTexcoords0.size() > 0;
			case E_VERTEX_STREAM_TYPE::NORMALS:
				return mNormals.size() > 0;
			case E_VERTEX_STREAM_TYPE::TANGENTS:
				return mTangents.size() > 0;
			case E_VERTEX_STREAM_TYPE::SKINNING:
				return false;
		}

		return false;
	}
}