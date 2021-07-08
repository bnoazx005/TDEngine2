#include "./../../include/platform/CBinaryMeshFileReader.h"
#include "./../../include/graphics/IMesh.h"
#include "../../include/platform/IOStreams.h"
#include <cstring>


namespace TDEngine2
{
	const U32 CBinaryMeshFileReader::mMeshVersion = 0x00010000; // 00.01.0000 


	CBinaryMeshFileReader::CBinaryMeshFileReader() :
		CBinaryFileReader()
	{
	}

	E_RESULT_CODE CBinaryMeshFileReader::LoadMesh(IMesh*& pDestMesh)
	{
		auto headerResult = _readMeshFileHeader();
		if (headerResult.HasError()) // \note read and verify header
		{
			return headerResult.GetError();
		}

		E_RESULT_CODE result = RC_OK;
		
		if ((result = _readGeometryBlock(pDestMesh)) != RC_OK)
		{
			return result;
		}

#if 0
		if ((result = _readSceneDescBlock(pDestMesh, headerResult.Get().mSceneDescOffset)) != RC_OK)
		{
			return result;
		}
#endif

		return RC_OK;
	}

	E_RESULT_CODE CBinaryMeshFileReader::LoadStaticMesh(IStaticMesh* const& pMesh)
	{
		E_RESULT_CODE result = RC_OK;

		U16 tag = 0x0;

		U16 objectId = 0;
		U32 vertexCount = 0;
		U32 facesCount = 0;

		auto readHeaderResult = _readMeshEntryHeader();
		if (readHeaderResult.HasError())
		{
			return readHeaderResult.GetError();
		}

		std::tie(objectId, vertexCount, facesCount) = readHeaderResult.Get();

		IMesh* pBaseMesh = dynamic_cast<IMesh*>(pMesh);

		if (RC_OK != (result = _readCommonMeshVertexData(pBaseMesh, vertexCount, facesCount)))
		{
			return result;
		}

		return _readMeshFacesData(pBaseMesh, facesCount);
	}

	E_RESULT_CODE CBinaryMeshFileReader::LoadSkinnedMesh(ISkinnedMesh* const& pMesh)
	{
		E_RESULT_CODE result = RC_OK;

		U16 tag = 0x0;

		U16 objectId = 0;
		U32 vertexCount = 0;
		U32 facesCount = 0;

		auto readHeaderResult = _readMeshEntryHeader();
		if (readHeaderResult.HasError())
		{
			return readHeaderResult.GetError();
		}

		std::tie(objectId, vertexCount, facesCount) = readHeaderResult.Get();

		IMesh* pBaseMesh = dynamic_cast<IMesh*>(pMesh);

		if (RC_OK != (result = _readCommonMeshVertexData(pBaseMesh, vertexCount, facesCount)))
		{
			return result;
		}

		/// \note Read faces or joint indices
		if ((result = Read(&tag, sizeof(U16))) != RC_OK)
		{
			TDE2_ASSERT(false);
			return result;
		}

		if (0xA401 == tag) /// \note Read joint weights (this is an optional step)
		{
			std::array<F32, MaxJointsCountPerVertex> tmpJointsWeights;
			U16 jointsCount = 0;

			for (U32 i = 0; i < vertexCount; ++i)
			{
				result = result | Read(&jointsCount, sizeof(U16));

				memset(&tmpJointsWeights.front(), 0, sizeof(tmpJointsWeights));

				if (!jointsCount || jointsCount > MaxJointsCountPerVertex)
				{
					TDE2_ASSERT(false);
					continue;
				}

				for (U16 k = 0; k < jointsCount; ++k)
				{
					result = result | Read(&tmpJointsWeights[k], sizeof(F32));
				}

				pMesh->AddVertexJointWeights(tmpJointsWeights);
			}

			/// \note Read faces or joint indices
			if ((result = Read(&tag, sizeof(U16))) != RC_OK)
			{
				TDE2_ASSERT(false);
				return result;
			}
		}

		if (0xA502 == tag) /// \note Read joint indices (this is an optional step)
		{
			std::array<U16, MaxJointsCountPerVertex> tmpJointsIndices;
			U16 jointsCount = 0;

			for (U32 i = 0; i < vertexCount; ++i)
			{
				result = result | Read(&jointsCount, sizeof(U16));

				if (!jointsCount || jointsCount > MaxJointsCountPerVertex)
				{
					TDE2_ASSERT(false);
					continue;
				}

				memset(&tmpJointsIndices.front(), 0, sizeof(tmpJointsIndices));

				for (U16 k = 0; k < jointsCount; ++k)
				{
					result = result | Read(&tmpJointsIndices[k], sizeof(U16));
				}

				pMesh->AddVertexJointIndices(tmpJointsIndices);
			}
		}

		return _readMeshFacesData(pBaseMesh, facesCount);
	}

	E_RESULT_CODE CBinaryMeshFileReader::_onInit()
	{
		E_RESULT_CODE result = CBinaryFileReader::_onInit();
		result = result | _getInputStream()->Reset(true);

		return result;
	}

	E_RESULT_CODE CBinaryMeshFileReader::_onFree()
	{
		return RC_OK;
	}

	TResult<CBinaryMeshFileReader::TMeshFileHeader> CBinaryMeshFileReader::_readMeshFileHeader()
	{
		TMeshFileHeader header;

		E_RESULT_CODE result = Read(&header, sizeof(header));
		result = result | ((strcmp(header.mTag, "MESH") != 0 || header.mVersion != mMeshVersion) ? RC_INVALID_FILE : RC_OK);

		if (result != RC_OK)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		return Wrench::TOkValue<TMeshFileHeader>(header);
	}

	E_RESULT_CODE CBinaryMeshFileReader::_readGeometryBlock(IMesh*& pMesh)
	{
		E_RESULT_CODE result = SetPosition(16);

		U16 tag = 0, meshesCount = 0;

		if (RC_OK != (result = Read(&tag, sizeof(U16))))
		{
			return result;
		}

		TDE2_ASSERT(tag == 0x2F);

		if (RC_OK != (result = Read(&meshesCount, sizeof(meshesCount))))
		{
			return result;
		}

		for (U16 i = 0; i < meshesCount; ++i)
		{
			result = result | pMesh->Accept(this);
		}

		return result;
	}

	bool CBinaryMeshFileReader::_readMeshEntryBlock(IMesh*& pMesh)
	{
		return false;
#if 0
		E_RESULT_CODE result = RC_OK;

		U16 tag = 0x0;
		
		U16 objectId    = 0;
		U32 vertexCount = 0;
		U32 facesCount  = 0;

		auto readHeaderResult = _readMeshEntryHeader();
		if (readHeaderResult.HasError())
		{
			return false;
		}

		std::tie(objectId, vertexCount, facesCount) = readHeaderResult.Get();
		
		if (RC_OK != (result = _readCommonMeshVertexData(pMesh, vertexCount, facesCount)))
		{
			return result;
		}

#if 0
		if (0xA401 == tag) /// \note Read joint weights (this is an optional step)
		{
			std::array<F32, mMaxJointsCountPerVertex> tmpJointsWeights;
			U16 jointsCount = 0;

			for (U32 i = 0; i < vertexCount; ++i)
			{
				result = result | Read(&jointsCount, sizeof(U16));

				if (!jointsCount || jointsCount > mMaxJointsCountPerVertex)
				{
					TDE2_ASSERT(false);
					continue;
				}

				for (U16 k = 0; k < jointsCount; ++k)
				{
					result = result | Read(&tmpJointsWeights[k], sizeof(F32));
				}

				//pStaticMesh->AddNormal(TVector4(vecData.x, vecData.y, vecData.z, 0.0f));
			}

			/// \note Read faces or joint indices
			if ((result = Read(&tag, sizeof(U16))) != RC_OK)
			{
				TDE2_ASSERT(false);
				return false;
			}
		}

		if (0xA502 == tag) /// \note Read joint indices (this is an optional step)
		{
			std::array<U16, mMaxJointsCountPerVertex> tmpJointsIndices;
			U16 jointsCount = 0;

			for (U32 i = 0; i < vertexCount; ++i)
			{
				result = result | Read(&jointsCount, sizeof(U16));

				if (!jointsCount || jointsCount > mMaxJointsCountPerVertex)
				{
					TDE2_ASSERT(false);
					continue;
				}

				for (U16 k = 0; k < jointsCount; ++k)
				{
					result = result | Read(&tmpJointsIndices[k], sizeof(U16));
				}
			}

			/// \note Read faces tag
			if ((result = Read(&tag, sizeof(U16))) != RC_OK)
			{
				TDE2_ASSERT(false);
				return false;
			}
		}
#endif

		return _readMeshFacesData(pMesh, facesCount);
#endif
	}

	TResult<CBinaryMeshFileReader::TMeshEntityHeader> CBinaryMeshFileReader::_readMeshEntryHeader()
	{
		E_RESULT_CODE result = RC_OK;

		U16 tag = 0x0;
		if ((result = Read(&tag, 2)) != RC_OK)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		if (tag != 0x4D48)
		{
			SetPosition(GetPosition() - 2); // roll back to previous position
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		U16 objectId = 0x0;
		if ((result = Read(&objectId, 2)) != RC_OK)
		{
			TDE2_ASSERT(false);
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		U32 vertexCount = 0;
		U32 facesCount = 0;

		if ((result = Read(&vertexCount, sizeof(U32))) != RC_OK)
		{
			TDE2_ASSERT(false);
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		if ((result = Read(&facesCount, sizeof(U32))) != RC_OK)
		{
			TDE2_ASSERT(false);
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		U32 padding = 0x0;
		if ((result = Read(&padding, sizeof(U32))) != RC_OK)
		{
			TDE2_ASSERT(false);
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		return Wrench::TOkValue<TMeshEntityHeader>({ objectId, vertexCount, facesCount });
	}
	
	E_RESULT_CODE CBinaryMeshFileReader::_readCommonMeshVertexData(IMesh*& pMesh, U32 vertexCount, U32 facesCount)
	{
		E_RESULT_CODE result = RC_OK;

		U16 tag = 0x0;

		/// \note read vertices
		if ((result = Read(&tag, sizeof(U16))) != RC_OK)
		{
			TDE2_ASSERT(false);
			return result;
		}
		TDE2_ASSERT(tag == 0x01CD);

		TVector4 vecData;

		for (U32 i = 0; i < vertexCount; ++i)
		{
			result = result | Read(&vecData.x, sizeof(F32));
			result = result | Read(&vecData.y, sizeof(F32));
			result = result | Read(&vecData.z, sizeof(F32));
			result = result | Read(&vecData.w, sizeof(F32));

			pMesh->AddPosition(vecData);
		}

		/// \note Read normals (this is an optional step)
		if ((result = Read(&tag, sizeof(U16))) != RC_OK)
		{
			TDE2_ASSERT(false);
			return result;
		}

		if (0xA10E == tag) /// \note Read normals (this is an optional step)
		{
			for (U32 i = 0; i < vertexCount; ++i)
			{
				result = result | Read(&vecData.x, sizeof(F32));
				result = result | Read(&vecData.y, sizeof(F32));
				result = result | Read(&vecData.z, sizeof(F32));
				result = result | Read(&vecData.w, sizeof(F32));

				pMesh->AddNormal(TVector4(vecData.x, vecData.y, vecData.z, 0.0f));
			}

			/// \note Read first uv channel
			if ((result = Read(&tag, sizeof(U16))) != RC_OK)
			{
				TDE2_ASSERT(false);
				return result;
			}
		}

		if (0xA2DF == tag) /// \note Read tangents (this is an optional step)
		{
			for (U32 i = 0; i < vertexCount; ++i)
			{
				result = result | Read(&vecData.x, sizeof(F32));
				result = result | Read(&vecData.y, sizeof(F32));
				result = result | Read(&vecData.z, sizeof(F32));
				result = result | Read(&vecData.w, sizeof(F32));

				pMesh->AddTangent(TVector4(vecData.x, vecData.y, vecData.z, 0.0f));
			}

			/// \note Read first uv channel
			if ((result = Read(&tag, sizeof(U16))) != RC_OK)
			{
				TDE2_ASSERT(false);
				return result;
			}
		}

		TDE2_ASSERT(tag == 0x02F0);

		for (U32 i = 0; i < vertexCount; ++i)
		{
			result = result | Read(&vecData.x, sizeof(F32));
			result = result | Read(&vecData.y, sizeof(F32));
			result = result | Read(&vecData.z, sizeof(F32));
			result = result | Read(&vecData.w, sizeof(F32));

			pMesh->AddTexCoord0(TVector2(vecData.x, vecData.y));
		}

		return result;
	}

	E_RESULT_CODE CBinaryMeshFileReader::_readMeshFacesData(IMesh*& pMesh, U32 facesCount)
	{
		E_RESULT_CODE result = RC_OK;

		U16 tag = 0x0;

		/// \note read faces
		if ((result = Read(&tag, sizeof(U16))) != RC_OK)
		{
			TDE2_ASSERT(false);
			return result;
		}

		TDE2_ASSERT(tag == 0x03FF);

		U16 format = 0x0;
		if ((result = Read(&format, sizeof(U16))) != RC_OK)
		{
			TDE2_ASSERT(false);
			return result;
		}

		U32 faceIndices[3];

		for (U32 i = 0; i < facesCount / 3; ++i)
		{
			memset(faceIndices, 0, sizeof(faceIndices));

			result = result | Read(&faceIndices[0], format);
			result = result | Read(&faceIndices[1], format);
			result = result | Read(&faceIndices[2], format);

			pMesh->AddFace(faceIndices);
		}

		return result;
	}

	E_RESULT_CODE CBinaryMeshFileReader::_readSceneDescBlock(IMesh*& pMesh, U32 offset)
	{
		return RC_NOT_IMPLEMENTED_YET;
	}


	IFile* CreateBinaryMeshFileReader(IMountableStorage* pStorage, IStream* pStream, E_RESULT_CODE& result)
	{
		CBinaryMeshFileReader* pFileInstance = new (std::nothrow) CBinaryMeshFileReader();

		if (!pFileInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pFileInstance->Open(pStorage, pStream);

		if (result != RC_OK)
		{
			delete pFileInstance;

			pFileInstance = nullptr;
		}

		return dynamic_cast<IFile*>(pFileInstance);
	}
}