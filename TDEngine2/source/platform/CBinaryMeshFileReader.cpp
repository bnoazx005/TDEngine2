#include "../../include/platform/CBinaryMeshFileReader.h"
#include "../../include/graphics/IMesh.h"
#include "../../include/graphics/ISkeleton.h"
#include "../../include/platform/IOStreams.h"
#include "../../include/utils/CFileLogger.h"
#include <cstring>


namespace TDEngine2
{
	const U32 CBinaryMeshFileReader::mMeshVersion = 0x00030000; // 00.03.0000 


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

		return _readSubmeshes(pDestMesh, headerResult.Get());
	}

	E_RESULT_CODE CBinaryMeshFileReader::LoadStaticMesh(IStaticMesh* const& pMesh)
	{
		E_RESULT_CODE result = RC_OK;

		U16 tag = 0x0;

		I32 parentId = 0;
		U32 vertexCount = 0;
		U32 facesCount = 0;

		std::string subMeshId;

		auto readHeaderResult = _readSubMeshEntryHeader();
		if (readHeaderResult.HasError())
		{
			return readHeaderResult.GetError();
		}

		std::tie(parentId, vertexCount, facesCount, subMeshId) = readHeaderResult.Get();

		pMesh->AddSubMeshInfo(subMeshId, {pMesh->GetFacesCount() * 3, facesCount * 3});

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

		I32 parentId = 0;
		U32 vertexCount = 0;
		U32 facesCount = 0;

		std::string subMeshId;

		auto readHeaderResult = _readSubMeshEntryHeader();
		if (readHeaderResult.HasError())
		{
			return readHeaderResult.GetError();
		}

		std::tie(parentId, vertexCount, facesCount, subMeshId) = readHeaderResult.Get();

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
			std::array<U32, MaxJointsCountPerVertex> tmpJointsIndices;
			U16 jointsCount = 0;

			for (U32 i = 0; i < vertexCount; ++i)
			{
				result = result | Read(&jointsCount, sizeof(U16));

				if (!jointsCount || jointsCount > MaxJointsCountPerVertex)
				{
					TDE2_ASSERT(false);
					continue;
				}

				for (U16 k = 0; k < MaxJointsCountPerVertex; ++k)
				{
					if (k >= jointsCount)
					{
						tmpJointsIndices[k] = static_cast<U32>(ISkeleton::mMaxNumOfJoints - 1);
						continue;
					}

					result = result | Read(&tmpJointsIndices[k], sizeof(U32));
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
		result = result | ((strcmp(header.mTag, "MESH") != 0) ? RC_INVALID_FILE : RC_OK);

		if (header.mVersion < mMeshVersion)
		{
			LOG_WARNING(Wrench::StringUtils::Format("[CBinaryMeshFileReader] The version of the file {0} is older than current {1}. The migration is needed", header.mVersion, mMeshVersion));
			return Wrench::TErrValue<E_RESULT_CODE>(RC_INVALID_FILE);
		}

		if (result != RC_OK)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		return Wrench::TOkValue<TMeshFileHeader>(header);
	}

	E_RESULT_CODE CBinaryMeshFileReader::_readSubmeshes(IMesh*& pMesh, const TMeshFileHeader& header)
	{
		E_RESULT_CODE result = SetPosition(sizeof(TMeshFileHeader));

		for (U16 i = 0; i < header.mMeshesCount; ++i)
		{
			result = result | pMesh->Accept(this);
		}

		return result;
	}

	TResult<CBinaryMeshFileReader::TMeshEntityHeader> CBinaryMeshFileReader::_readSubMeshEntryHeader()
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

		C8 meshId[64];

		if ((result = Read(&meshId, sizeof(meshId))) != RC_OK)
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

		I32 parentId = 0x0;
		if ((result = Read(&parentId, sizeof(parentId))) != RC_OK)
		{
			TDE2_ASSERT(false);
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		return Wrench::TOkValue<TMeshEntityHeader>({ parentId, vertexCount, facesCount, { meshId } });
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

		for (U32 i = 0; i < facesCount; ++i)
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


	IFile* CreateBinaryMeshFileReader(IMountableStorage* pStorage, TPtr<IStream> pStream, E_RESULT_CODE& result)
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